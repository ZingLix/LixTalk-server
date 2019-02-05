#include "ChatServer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "psyche/psyche.h"
#include "Setting.h"
#include <iostream>
#include <functional>

using namespace std::placeholders;

ChatServer::ChatServer(in_port_t port): server_(port) {
	server_.setNewConnCallback(std::bind(&ChatServer::onNewConn, this, _1));
	server_.setReadCallback(std::bind(&ChatServer::waitingFirstMsg, this, _1,_2));
	server_.setCloseCallback(std::bind(&ChatServer::logout,this,_1));
}

void ChatServer::start() {
	try {
		db_.connect("zinglix", "password");
	} catch (sql::SQLException& e) {
		using std::cout;
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << "EXAMPLE_FUNCTION" << ") on line " << __LINE__ << std::endl;
		/* Use what() (derived from std::runtime_error) to fetch the error message */
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
	}
	cur_user_count = db_.getMaxUserID();
	cur_chatmsg_idx = db_.getChatMsgIdx();
	cur_chatmsg_count= db_.getChatMsgCount(cur_chatmsg_idx);
	server_.start();
}

void ChatServer::msgExec_login(psyche::Connection conn, message& msg) {
	int id = checkLoginInfo(msg);
	if(id==-1) {
		msgExec_err_fatal(conn, "Incorrect username or password");
	}else {
		user_.insert(std::make_pair(id, conn));
		con_to_id_.insert(std::make_pair(conn, id));
		vistor_.erase(conn);
		//userMap_.insert(std::make_pair(id, user(conn)));
		//socketMap_.insert(conn, id);

		message m;

		m.add("sender_id", 0);
		m.add("type", 0);
		m.add("result", 1);
		m.add("recver_id", id);

		LOG_INFO << id << " login." ;
		sendMsg(conn, m.getString());
		conn.setReadCallback(std::bind(&ChatServer::recvMsg, this, _1, _2));
		//server_.setMessageCallback(conn, std::bind(&ChatServer::recvMsg, this,
		//	std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		//server_.setMessageCallback(conn, [this](psyche::Connection conn, std::string msg, Server* serv)
		//{
		//	this->recvMsg(conn, msg, serv);
		//});
	}

}

void ChatServer::execUnsentMsg(int id) {
	auto ptr = db_.getOfflineMsg(id);
	for(auto it=ptr->begin();it!=ptr->end();++it) {
		sendMsg(user_.find(id)->second, *it);
	}
}

int ChatServer::checkLoginInfo(message& msg) {
	std::string username = msg.getString("username");
	auto userinfo = db_.getUser(username);
	try {
		userinfo->next();
		if (msg.getString("password") == userinfo->getString("password")) {
			return userinfo->getInt("user_id");
		}
		else {
			return -1;
		}
	}catch (sql::InvalidArgumentException&) {
		return -1;
	}
}

void ChatServer::logout(psyche::Connection conn) {
	auto it = con_to_id_.find(conn);
	if (it != con_to_id_.end()) {
		auto id = it->second;
		if (id != -1) LOG_INFO << id << " offline.";
		con_to_id_.erase(it);
		user_.erase(id);
	}
}

void ChatServer::msgExec_register(psyche::Connection conn, message& msg) {
	std::string username = msg.getString("username");
	std::string password = msg.getString("password");
	db_.addUser(username, password);
	auto info = db_.getUser(username);
	info->next();
	auto id = info->getUInt64("user_id");
	if(id % userCountEachTable ==0) {
		db_.createUserSeqTable(id/userCountEachTable);
	}
	rapidjson::Document doc;
	rapidjson::MemoryPoolAllocator<>& allocator = doc.GetAllocator();
	doc.SetObject();
	doc.AddMember("sender_id", 0, allocator);
	doc.AddMember("type", 3, allocator);
	doc.AddMember("result", 1, allocator);

	rapidjson::StringBuffer buffer;
	buffer.Clear();
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	doc.Accept(writer);
	std::string m = buffer.GetString();
	conn.send(m);
	//TODO close the connection
	conn.close();
}

void ChatServer::msgExec_err_fatal(psyche::Connection conn, std::string errMsg) {
	msgExec_err(conn, errMsg);
	//TODO close the connection
	conn.close();
}

bool ChatServer::onNewConn(psyche::Connection conn) {
	//server_.setMessageCallback(conn, std::bind(&ChatServer::waitingFirstMsg, this,
	//                                         std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	vistor_.insert(conn);
	return true;
}

void ChatServer::msgExec_err(psyche::Connection conn, std::string errMsg) {
	LOG_INFO << conn.peer_endpoint().to_string() << ": " << errMsg;
	rapidjson::Document doc;
	rapidjson::MemoryPoolAllocator<>& allocator = doc.GetAllocator();
	doc.SetObject();
	doc.AddMember("sender_id", 0, allocator);
	doc.AddMember("type", 999, allocator);
	//const char* tmp = errMsg.c_str();
	doc.AddMember("content", errMsg, allocator);

	rapidjson::StringBuffer buffer;
	buffer.Clear();
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	doc.Accept(writer);
	std::string msg = buffer.GetString();
	conn.send(msg);
}

void ChatServer::waitingFirstMsg(psyche::Connection conn, psyche::Buffer buff) {
	try {
		message m(buff.retrieveAll());

		if (m.getInt("recver_id") != 0) {
			conn.send("Bad request!");
			//TODO close the connection
			conn.close();
			//serv->send(conn, "Bad request!");
			//serv->shutdown(conn);
		}
		else {
			switch (m.getInt("type")) {
			case 0: //login request
				msgExec_login(conn, m);
				break;
			case 1: //register request
				msgExec_register(conn, m);
				break;

			default:
				msgExec_err(conn, "unknown type");
			}
		}
	}
	catch (std::invalid_argument) {
		msgExec_err_fatal(conn, "Bad Request");
	}
}

void ChatServer::forwardMsg(int sender_id, int recver_id, std::string msg) {
	std::string loginfo = "receive new message from " + std::to_string(sender_id) + " to " 
		+ std::to_string(recver_id) +  " message:" + msg ;
	auto it = user_.find(recver_id);
	//auto conn = user_[recver_id];
	if (it!=user_.end()) {
		sendMsg(it->second, msg);
		LOG_INFO<<loginfo << " forwarded";
	}else {
		db_.addOfflineMsg(recver_id, msg);
		LOG_INFO << loginfo << " receiver offline";
	}
	saveMsg(sender_id, recver_id,msg);
}

void ChatServer::saveMsg(int sender_id, int recver_id,std::string& msg) {
	message m(msg);
	std::string str = m.getString("content");
	if(cur_chatmsg_count >=chatMsgCountEachTable) {
		if (mutex_.try_lock()) {
			cur_chatmsg_count = 0;
			++cur_chatmsg_idx;
			db_.createChatMsgTable(cur_chatmsg_idx);
			db_.updateStatusMsgIdx(cur_chatmsg_idx);
			mutex_.unlock();
		}else {
			using namespace std::chrono_literals;
			while (cur_chatmsg_idx >= chatMsgCountEachTable) {
				std::this_thread::sleep_for(10ms);
			}
		}
	}
	int idx = cur_chatmsg_idx;
	++cur_chatmsg_count;
	db_.saveMsg(sender_id, recver_id, str, idx,1);
}

void ChatServer::recvMsg(psyche::Connection conn, psyche::Buffer buffer) {
	auto ptr= split(buffer.retrieveAll());

	for(auto it=ptr->begin();it!=ptr->end();++it) {
		try {
			message m(*it);
			switch (m.getInt("type")) {
			case 3:
				msgExec_friend(conn, m);
				break;
			case 7:
				pullMsg(conn, m);
				break;
			case 8:
				execUnsentMsg(con_to_id_[conn]);
				break;
			case 9:
				forwardMsg(m.getInt("sender_id"), m.getInt("recver_id"), *it);
				break;
			default:
				msgExec_err(conn, "unknown kype!");
			}
		}
		catch (std::invalid_argument) {
			msgExec_err(conn, "Bad Request");
		}
	}

}

std::shared_ptr<std::vector<std::string>> ChatServer::split(const std::string& msg) {
	std::shared_ptr<std::vector<std::string>> ptr(new std::vector<std::string>());
	size_t pos = 0;
	size_t p;
	while ((p = msg.find("\r\n\r\n", pos)) != std::string::npos) {
		ptr->push_back(msg.substr(pos, p-pos));
		pos = p + 4;
	}
	return ptr;
}

void ChatServer::msgExec_friend(psyche::Connection conn, message& msg) {
	std::string m = msg.getString();
	switch (msg.getInt("code")) {
		case 1:
			friend_request(con_to_id_[conn], msg.getInt("recver_id"),msg.getString("content"));
			break;
		case 2:
			friend_accepted(msg.getInt("sender_id"), msg.getInt("recver_id"));
			break;
		case 3:
			friend_refused(msg.getInt("sender_id"), msg.getInt("recver_id"));
			break;
		case 4:
			friend_list(msg.getInt("sender_id"));
			break;
	}
}

void ChatServer::friend_request(int sender_id, int recver_id,std::string content) {
	message m;
	m.add("type", 3);
	m.add("code", 1);
	m.add("sender_id", sender_id);
	m.add("recver_id", recver_id);
	m.add("content", content);
	sendMsg(user_.find(recver_id)->second, m.getString());
}

void ChatServer::friend_accepted(int user1_id, int user2_id) {
	db_.addFriend(user1_id, user2_id);
	message m;
	m.add("type", 3);
	m.add("code", 2);
	m.add("recver_id", user2_id);
	sendMsg(user_.find(user1_id)->second, m.getString());
}

void ChatServer::friend_refused(int user1_id, int user2_id) {
	message m;
	m.add("type", 3);
	m.add("code", 3);
	m.add("recver_id", user2_id);
	sendMsg(user_.find(user1_id)->second, m.getString());
}

void ChatServer::friend_list(int id) {
	auto res = db_.queryFriend(id);
	message m;
	m.add("type", 3);
	m.add("code", 4);
	rapidjson::Value friendId(rapidjson::kArrayType);
	rapidjson::Value friendGroup(rapidjson::kArrayType);
	auto& alloc = m.getAllocator();
	while(res->next()) {
		if(res->getInt("user_id_1")==id) {
			friendId.PushBack(res->getInt("user_id_2"),alloc);
			friendGroup.PushBack(res->getInt("group_id_in_1"), alloc);
		}else {
			friendId.PushBack(res->getInt("user_id_1"), alloc);
			friendGroup.PushBack(res->getInt("group_id_in_1"), alloc);
		}
	}
	m.add("friendID", std::move(friendId));
	m.add("friendGroup", std::move(friendGroup));
	auto str = m.getString();
	sendMsg(user_.find(id)->second, m.getString());
}

void ChatServer::pullMsg(psyche::Connection conn, message& m) {
	int id = con_to_id_[conn];
	auto result = db_.pullMsg(id, cur_chatmsg_idx);

	while(result->next()) {
		message msg;
		msg.add("type", 7);
		msg.add("seq_id", result->getInt64("seq_id"));
		msg.add("sender_id", result->getInt64("user_id_from"));
		msg.add("recver_id", result->getInt64("user_id_to"));
		msg.add("content", result->getString("content"));
		sendMsg(conn, msg.getString());
	}
}
