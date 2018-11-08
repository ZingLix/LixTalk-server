#ifndef LIXTALK_DBCONNECTOR
#define LIXTALK_DBCONNECTOR

#include <cppconn/resultset.h>
#include <cppconn/driver.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/exception.h>
#include <memory>
#include <hiredis/hiredis.h>
#include "LogInfo.h"

class DbConnector {
public:
    DbConnector(): con(nullptr) {
        driver_ = get_driver_instance();
        redisCon = redisConnect("127.0.0.1", 6379);
    }

    void connect(std::string username, std::string password) {
        con = driver_->connect("tcp://127.0.0.1:3306", username, password);
		try {
			con->setSchema("LixTalk");
		}catch (sql::SQLException& e) {
			if(e.getErrorCode()==1049) {
				LOG_ERROR << "Database not existed.";
				initDb();
				LOG_INFO << "Database create success.";
			}else {
				throw;
			}
		}
    }

    int getMaxUserID() {
        sql::PreparedStatement *stmt = con->prepareStatement("SELECT Max(id) from user");
        std::shared_ptr<sql::ResultSet> res (stmt->executeQuery());
        delete stmt;
        return res->getInt("id");
    }

    std::shared_ptr < sql::ResultSet > getUser(std::string username) {
        std::shared_ptr< sql::Statement > stmt (con->createStatement());
        stmt->execute("SELECT * from user where username = '" + username + "'");
        return std::shared_ptr < sql::ResultSet >(stmt->getResultSet());
    }

    void addUser(std::string username, std::string password) {
        sql::PreparedStatement *stmt = con->prepareStatement("INSERT INTO user(id,username,password) VALUES (null,?,?)");
        stmt->setString(1, username);
        stmt->setString(2, password);
        stmt->executeUpdate();
        delete stmt;
    }

    void addFriend(int userID_1, int userID_2) {
        sql::PreparedStatement *stmt = con->prepareStatement("INSERT INTO friend(userID_1,userID_2) VALUES ("
                                       + std::to_string(userID_1) + " , " + std::to_string(userID_2) + ")");
        stmt->executeUpdate();
        delete stmt;
    }

    auto queryFriend(int id) {
        sql::PreparedStatement *stmt = con->prepareStatement("SELECT userID_1,userID_2,groupID_in_1,groupID_in_2 \
			from friend where userID_1 = " + std::to_string(id) + " or userID_2 = " + std::to_string(id));
        std::shared_ptr<sql::ResultSet> res (stmt->executeQuery()) ;
        delete stmt;
        return res;
    }

    void addOfflineMsg(int id, std::string msg) {
        redisCommand(redisCon, "RPUSH OFFLINE_MSG_%d %s", id, msg.c_str());
    }

    std::shared_ptr<std::vector<std::string>> getOfflineMsg(int id) {
        redisReply* reply = static_cast<redisReply*>(redisCommand(redisCon, "LLEN OFFLINE_MSG_%d", id));
        auto length = reply->integer;
        reply = static_cast<redisReply*>(redisCommand(redisCon, "LRANGE OFFLINE_MSG_%d 0 %d", id, length));
        std::shared_ptr<std::vector<std::string>> ptr(new std::vector<std::string>());
        for(int i = 0; i < length; i++) {
            ptr->push_back(reply->element[i]->str);
        }
        redisCommand(redisCon, "DEL OFFLINE_MSG_%d", id);
        return ptr;
    }

	void createUserSeqTable(int idx) {
		std::string tableName("userSeq_" + std::to_string(idx));
		std::unique_ptr<sql::PreparedStatement> stmt ( con->prepareStatement("Create Table "+ tableName + " ( \
			seq_id Bigint Unsigned auto_increment primary key, \
			user_id int unsigned not null,\
			record_table_idx int unsigned not null,\
			record_id int unsigned not null)" ));
		stmt->executeUpdate();
		stmt.reset(con->prepareStatement("Create index idx on "+tableName+"(user_id)"));
		stmt->executeUpdate();
    }

	void createChatMsgTable(int idx) {
		std::string tableName("chatHistory_" + std::to_string(idx));
		std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement("Create Table " + tableName + " ( \
			msg_id int Unsigned auto_increment primary key, \
			user_id_from int unsigned not null,\
			user_id_to int unsigned not null,\
			type int not null,\
			content varchar(255) not null)"));
		stmt->executeUpdate();
		stmt.reset(con->prepareStatement("Create index idx on " + tableName + "(msg_id)"));
		stmt->executeUpdate();
	}

	void createUserTable() {
		std::string tableName("user");
		std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement("Create Table " + tableName + " ( \
			user_id bigint Unsigned auto_increment primary key, \
			username varchar(64) not null,\
			password varchar(64) not null)"));
		stmt->executeUpdate();
		stmt.reset(con->prepareStatement("Create index idx on " + tableName + "(user_id)"));
		stmt->executeUpdate();
    }

	void createFriendTable() {
		std::string tableName("friend");
		std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement("Create Table " + tableName + " ( \
			id bigint Unsigned auto_increment primary key, \
			user_id_1 bigint unsigned not null,\
			user_id_2 bigint unsigned not null,\
			group_id_in_1 int unsigned not null,\
			group_id_in_2 int unsigned not null,\
			add_time timestamp default current_timestamp)"));
		stmt->executeUpdate();
		stmt.reset(con->prepareStatement("Create index idx on " + tableName + "(user_id_1,user_id_2)"));
		stmt->executeUpdate();
	}

	void createStatusTable() {
		std::string tableName("status");
		std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement("Create Table " + tableName + " ( \
			curUserCount bigint Unsigned , \
			curMsgCount bigint unsigned not null)"));
		stmt->executeUpdate();
		stmt.reset(con->prepareStatement("insert into " + tableName + " values (0,0)"));
		stmt->executeUpdate();
	}

	void initDb() {
		std::string DbName("LixTalk");
		std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement("Create Database " + DbName ));
		stmt->executeUpdate();
		con->setSchema("LixTalk");
		createStatusTable();
		createUserTable();
		createFriendTable();
		createUserSeqTable(0);
		createChatMsgTable(0);
    }

    ~DbConnector() {
        if(con != nullptr)
            delete con;
    }

private:
    sql::Driver* driver_;
    sql::Connection* con;
    redisContext *redisCon;
};

#endif
