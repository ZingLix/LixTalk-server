#include "ChatServer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "LogInfo.h"
#include "Setting.h"
#include <iostream>

ChatServer::ChatServer(in_port_t port): userMap_(),server_(port) {
	server_.setNewConnCallback(std::bind(&ChatServer::onNewConn, this,
	                                     std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	server_.setCloseCallback([this](int id, Server* s) { this->logout(id); });
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

void ChatServer::msgExec_login(int fd, message& msg) {
	int id = checkLoginInfo(msg);
	if(id==-1) {
		msgExec_err_fatal(fd, "Incorrect username or password");
	}else {
		userMap_.insert(std::make_pair(id, user(fd)));
		socketMap_.insert(fd, id);

		message m;

		m.add("sender_id", 0);
		m.add("type", 0);
		m.add("result", 1);
		m.add("recver_id", id);

		LOG_INFO << id << " login." ;
		sendMsg(fd, m.getString());
		server_.setMessageCallback(fd, std::bind(&ChatServer::recvMsg, this,
			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	}

}

void ChatServer::execUnsentMsg(int id) {
	auto ptr = db_.getOfflineMsg(id);
	for(auto it=ptr->begin();it!=ptr->end();++it) {
		sendMsg(socketMap_.getFd(id), *it);
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

void ChatServer::logout(int fd) {
	auto id = socketMap_.getId(fd);
	if(id!=-1) LOG_INFO << id << " offline.";
	socketMap_.removeByFd(fd);
}

void ChatServer::msgExec_register(int fd, message& msg) {
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
	server_.send(fd, m);
	Socket::shutdown(fd);
}

void ChatServer::msgExec_err_fatal(int fd, std::string errMsg) {
	msgExec_err(fd, errMsg);
	server_.shutdown(fd);
}

bool ChatServer::onNewConn(int fd, const NetAddress& addr, Server* serv) {
	server_.setMessageCallback(fd, std::bind(&ChatServer::waitingFirstMsg, this,
	                                         std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	return true;
}

void ChatServer::msgExec_err(int fd, std::string errMsg) {
	LOG_INFO << fd << ": " << errMsg;
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
	server_.send(fd, msg);
}

void ChatServer::waitingFirstMsg(int fd, std::string msg, Server* serv) {
	try {
		message m(msg);

		if (m.getInt("recver_id") != 0) {
			serv->send(fd, "Bad request!");
			serv->shutdown(fd);
		}
		else {
			switch (m.getInt("type")) {
			case 0: //login request
				msgExec_login(fd, m);
				break;
			case 1: //register request
				msgExec_register(fd, m);
				break;

			default:
				msgExec_err(fd, "unknown type");
			}
		}
	}
	catch (std::invalid_argument) {
		msgExec_err_fatal(fd, "Bad Request");
	}
}

void ChatServer::forwardMsg(int sender_id, int recver_id, std::string msg) {
	std::string loginfo = "receive new message from " + std::to_string(sender_id) + " to " 
		+ std::to_string(recver_id) +  " message:" + msg ;
	auto fd = socketMap_.getFd(recver_id);
	if (fd!=-1) {
		sendMsg(fd, msg);
		LOG_INFO<<loginfo << " forwarded";
	}else {
		db_.addOfflineMsg(recver_id, msg);
		LOG_INFO << loginfo << " receiver offline";
	}
	saveMsg(sender_id, recver_id,msg);
}

void ChatServer::saveMsg(int sender_id, int recver_id,std::string& msg) {
	if(cur_chatmsg_idx >=chatMsgCountEachTable) {
		if (mutex_.try_lock()) {
			cur_chatmsg_count = 0;
			++cur_chatmsg_idx;
			db_.createChatMsgTable(cur_chatmsg_idx);
			mutex_.unlock();
		}else {
			using namespace std::chrono_literals;
			while (cur_chatmsg_idx >= chatMsgCountEachTable) {
				std::this_thread::sleep_for(100ms);
			}
		}
	}
	int idx = cur_chatmsg_idx;
	++cur_chatmsg_count;
	db_.saveMsg(sender_id, recver_id, msg, idx,1);
}

void ChatServer::recvMsg(int fd, std::string msg, Server* serv) {
	auto ptr= split(msg);

	for(auto it=ptr->begin();it!=ptr->end();++it) {
		try {
			message m(*it);
			switch (m.getInt("type")) {
			case 9:
				forwardMsg(m.getInt("sender_id"), m.getInt("recver_id"), *it);
				break;
			case 3:
				msgExec_friend(fd, m);
				break;
			case 8:
				execUnsentMsg(socketMap_.getId(fd));
				break;
			default:
				msgExec_err(fd, "unknown kype!");
			}
		}
		catch (std::invalid_argument) {
			msgExec_err(fd, "Bad Request");
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

void ChatServer::msgExec_friend(int fd, message& msg) {
	std::string m = msg.getString();
	switch (msg.getInt("code")) {
		case 1:
			friend_request(socketMap_.getId(fd), msg.getInt("recver_id"),msg.getString("content"));
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
	sendMsg(socketMap_.getFd(recver_id), m.getString());
}

void ChatServer::friend_accepted(int user1_id, int user2_id) {
	db_.addFriend(user1_id, user2_id);
	message m;
	m.add("type", 3);
	m.add("code", 2);
	m.add("recver_id", user2_id);
	sendMsg(socketMap_.getFd(user1_id), m.getString());
}

void ChatServer::friend_refused(int user1_id, int user2_id) {
	message m;
	m.add("type", 3);
	m.add("code", 3);
	m.add("recver_id", user2_id);
	sendMsg(socketMap_.getFd(user1_id), m.getString());
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
	sendMsg(socketMap_.getFd(id), m.getString());
}
