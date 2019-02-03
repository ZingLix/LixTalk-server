#include "DbConnector.h"
#include "Setting.h"
#include <psyche/psyche.h>

const char * DbConnector::DbName = "LixTalk_test";

DbConnector::DbConnector() : con(nullptr) {
	driver_ = get_driver_instance();
	redisCon = redisConnect("127.0.0.1", 6379);
}

void DbConnector::connect(std::string username, std::string password) {
	con = driver_->connect("tcp://127.0.0.1:3306", username, password);
	try {
		con->setSchema(DbName);
	} catch (sql::SQLException& e) {
		if (e.getErrorCode() == 1049) {
			LOG_ERROR << "Database not existed.";
			initDb();
			LOG_INFO << "Database create success.";
		} else {
			throw;
		}
	}
}

int DbConnector::getMaxUserID() {
	std::unique_ptr<sql::PreparedStatement> stmt(
		con->prepareStatement("SELECT count(*) c from user"));
	std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
	res->next();
	return res->getInt("c");
}

int DbConnector::getChatMsgIdx() {
	std::unique_ptr<sql::PreparedStatement> stmt(
		con->prepareStatement("SELECT curMsgIdx from status"));
	std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
	res->next();
	return res->getInt("curMsgIdx");
}

int DbConnector::getChatMsgCount(int idx) {
	std::unique_ptr<sql::PreparedStatement> stmt(
		con->prepareStatement("SELECT count(*) c from chatHistory_" + std::to_string(idx)));
	std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
	res->next();
	return res->getInt("c");
}

std::shared_ptr<sql::ResultSet> DbConnector::getUser(std::string username) {
	std::shared_ptr<sql::Statement> stmt(con->createStatement());
	stmt->execute("SELECT * from user where username = '" + username + "'");
	return std::shared_ptr<sql::ResultSet>(stmt->getResultSet());
}

void DbConnector::addUser(std::string username, std::string password) {
	sql::PreparedStatement* stmt = con->prepareStatement(
		"INSERT INTO user(username,password) VALUES (?,?)");
	stmt->setString(1, username);
	stmt->setString(2, password);
	stmt->executeUpdate();
	delete stmt;
}

void DbConnector::addFriend(int userID_1, int userID_2) {
	sql::PreparedStatement* stmt = con->prepareStatement(
		"INSERT INTO friend(user_id_1,user_id_2) VALUES ("
		+ std::to_string(userID_1) + " , " + std::to_string(userID_2) + ")");
	stmt->executeUpdate();
	delete stmt;
}

void DbConnector::addOfflineMsg(int id, std::string msg) {
	redisCommand(redisCon, "RPUSH OFFLINE_MSG_%d %s", id, msg.c_str());
}

std::shared_ptr<std::vector<std::string>> DbConnector::getOfflineMsg(int id) {
	redisReply* reply = static_cast<redisReply*>(redisCommand(redisCon, 
		"LLEN OFFLINE_MSG_%d", id));
	auto length = reply->integer;
	reply = static_cast<redisReply*>(redisCommand(redisCon, 
		"LRANGE OFFLINE_MSG_%d 0 %d", id, length));
	std::shared_ptr<std::vector<std::string>> ptr(new std::vector<std::string>());
	for (int i = 0; i < length; i++) {
		ptr->push_back(reply->element[i]->str);
	}
	redisCommand(redisCon, "DEL OFFLINE_MSG_%d", id);
	return ptr;
}

void DbConnector::createUserSeqTable(int idx) {
	std::string tableName("userSeq_" + std::to_string(idx));
	std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement(
		"Create Table " + tableName +
		" ( \
			seq_id Bigint Unsigned auto_increment primary key, \
			user_id int unsigned not null,\
			record_table_idx int unsigned not null,\
			record_id int unsigned not null)"));
	stmt->executeUpdate();
	stmt.reset(con->prepareStatement("Create index idx on " + tableName + "(user_id)"));
	stmt->executeUpdate();
}

void DbConnector::createChatMsgTable(int idx) {
	std::string tableName("chatHistory_" + std::to_string(idx));
	std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement(
		"Create Table " + tableName +
		" ( \
			msg_id int Unsigned auto_increment primary key, \
			user_id_from int unsigned not null,\
			user_id_to int unsigned not null,\
			type int not null,\
			content varchar(255) not null)"));
	stmt->executeUpdate();
	stmt.reset(con->prepareStatement("Create index idx on " + tableName + "(msg_id)"));
	stmt->executeUpdate();
}

void DbConnector::createUserTable() {
	std::string tableName("user");
	std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement(
		"Create Table " + tableName +
		" ( \
			user_id bigint Unsigned auto_increment primary key, \
			username varchar(64) not null,\
			password varchar(64) not null)"));
	stmt->executeUpdate();
	stmt.reset(con->prepareStatement("Create index idx on " + tableName + "(user_id)"));
	stmt->executeUpdate();
}

void DbConnector::createFriendTable() {
	std::string tableName("friend");
	std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement(
		"Create Table " + tableName +
		" ( \
			id bigint Unsigned auto_increment primary key, \
			user_id_1 bigint unsigned not null,\
			user_id_2 bigint unsigned not null,\
			group_id_in_1 int unsigned not null default 0,\
			group_id_in_2 int unsigned not null default 0,\
			add_time timestamp default current_timestamp)"));
	stmt->executeUpdate();
	stmt.reset(con->prepareStatement("Create index idx on " + tableName + "(user_id_1,user_id_2)"));
	stmt->executeUpdate();
}

void DbConnector::createStatusTable() {
	std::string tableName("status");
	std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement(
		"Create Table " + tableName +
		" ( \
			curUserCount bigint Unsigned , \
			curMsgIdx bigint unsigned not null)"));
	stmt->executeUpdate();
	stmt.reset(con->prepareStatement("insert into " + tableName + " values (0,0)"));
	stmt->executeUpdate();
}

void DbConnector::initDb() {
	std::string DatabaseName(DbName);
	std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement("Create Database " + DatabaseName));
	stmt->executeUpdate();
	con->setSchema(DbName);
	createStatusTable();
	createUserTable();
	createFriendTable();
	createUserSeqTable(0);
	createChatMsgTable(0);
}

void DbConnector::saveMsg(int sender_id, int recver_id, std::string& msg, int idx, int type) {
	std::string tableName("chatHistory_" + std::to_string(idx));
	std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement("insert into " + tableName +
		"(user_id_from,user_id_to,type,content) values(" +
		std::to_string(sender_id) + "," + std::to_string(recver_id) + "," +
		std::to_string(type) + ",'" + msg + "');"));
	stmt->execute();
	stmt.reset(con->prepareStatement("SELECT LAST_INSERT_ID() id;"));
	std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
	res->next();
	int seqid = res->getInt("id");
	stmt.reset(con->prepareStatement("insert into userSeq_" + std::to_string(sender_id / userCountEachTable)+
		"(user_id,record_table_idx,record_id) values("+std::to_string(sender_id)+","+
		std::to_string(idx)+","+ std::to_string(seqid)+ ")"));
	stmt->execute();
	stmt.reset(con->prepareStatement("insert into userSeq_" + std::to_string(recver_id / userCountEachTable) +
		"(user_id,record_table_idx,record_id) values(" + std::to_string(recver_id) + "," +
		std::to_string(idx) + "," + std::to_string(seqid) + ")"));
	stmt->execute();
}

DbConnector::~DbConnector() {
	if (con != nullptr)
		delete con;
}

std::unique_ptr<sql::ResultSet> DbConnector::queryFriend(int id) {
	sql::PreparedStatement* stmt = con->prepareStatement(
		"SELECT user_id_1,user_id_2, group_id_in_1, group_id_in_1 \
			from friend where user_id_1 = " + std::to_string(id) +
		" or user_id_2 = " + std::to_string(id));
	std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
	delete stmt;
	return res;
}

std::unique_ptr<sql::ResultSet> DbConnector::pullMsg(int id, int chatHisIdx, int num) {
	std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement(
		"select seq_id,user_id_from,user_id_to,type,content from userSeq_"+std::to_string(id/userCountEachTable)+
		" ,chatHistory_"+std::to_string(chatHisIdx)+" where record_id=msg_id and record_table_idx="+
		std::to_string(chatHisIdx)+ " and user_id="+std::to_string(id)+" order by seq_id desc limit 0,"+std::to_string(num)));
	return std::unique_ptr<sql::ResultSet>(stmt->executeQuery());
}

void DbConnector::updateStatusMsgIdx(int idx) {
	std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement(
		"update status set curMsgIdx = "+std::to_string(idx)));
	stmt->executeUpdate();
}
