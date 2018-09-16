#include "ChatServer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include <iostream>

ChatServer::ChatServer(in_port_t port): server_(port) {
	server_.setNewConnCallback(std::bind(&ChatServer::onNewConn, this,
	                                     std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	server_.setCloseCallback([this](int id, Server* s) { this->logout(id); });
}

void ChatServer::msgExec_login(int fd, message& msg) {
	int id = checkLoginInfo(msg);
	if(id==-1) {
		msgExec_err_fatal(fd, "Incorrect username or password");
	}else {
		userMap_.insert(std::make_pair(id, user(fd)));
		socketMap_.insert(fd, id);

		rapidjson::Document doc;
		rapidjson::MemoryPoolAllocator<>& allocator = doc.GetAllocator();
		doc.SetObject();
		doc.AddMember("sender_id", 0, allocator);
		doc.AddMember("type", 0, allocator);
		doc.AddMember("result", 1, allocator);
		doc.AddMember("recver_id", id, allocator);
		rapidjson::StringBuffer buffer;
		buffer.Clear();
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		doc.Accept(writer);
		std::string m = buffer.GetString();
		std::cout << id << " login!" << std::endl;
		server_.send(fd, m);
		server_.setMessageCallback(fd, std::bind(&ChatServer::recvMsg, this,
			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	}

}

int ChatServer::checkLoginInfo(message& msg) {
	std::string username = msg.getString("username");
	auto userinfo = db_.getUser(username);
	try {
		userinfo->next();
		if (msg.getString("password") == userinfo->getString("password")) {
			return userinfo->getInt("id");
		}
		else {
			return -1;
		}
	}catch (sql::InvalidArgumentException) {
		return -1;
	}
}

void ChatServer::logout(int fd) {
	std::cout << socketMap_.getId(fd) << " offline!" << std::endl;
	socketMap_.removeByFd(fd);

}


void ChatServer::msgExec_register(int fd, message& msg) {
	std::string username = msg.getString("username");
	std::string password = msg.getString("password");
	db_.addUser(username, password);

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
	message m(msg);
	if (m.getInt("recver_id") != 0) {
		serv->send(fd, "Bad request!");
		serv->shutdown(fd);
	} else {
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

bool ChatServer::forwardMsg(int sender_id, int recver_id, std::string msg) {
	std::cout << "sender id:" << sender_id << std::endl;
	std::cout << "recver id:" << recver_id << std::endl;
	std::cout << "message:" << msg << std::endl;
	auto fd = socketMap_.getFd(recver_id);
	if (fd!=-1) {
		sendMsg(fd, msg);
		std::cout << "forwarded\n";
		return true;
	}
	return false;
}

void ChatServer::recvMsg(int fd, std::string msg, Server* serv) {
	message m(msg);
	switch (m.getInt("type")) {
		case 9:
			if (forwardMsg(m.getInt("sender_id"), m.getInt("recver_id"), msg) == false) {
				msgExec_err(fd, "offline");
			}
			break;
		case 3:
			msgExec_friend(fd, m);
			break;
		default:
			msgExec_err(fd, "unknown kype!");
	}
}

//Friend Table
//create table friend
//	-> (
//	->seqID int NOT NULL AUTO_INCREMENT,
//	->userID_1 int NOT NULL,
//	->userID_2 int NOT NULL,
//	->groupID_in_1 int NOT NULL,
//	->groupID_in_2 int NOT NULL,
//	->addTime datetime not NULL,
//	->PRIMARY KEY(seqID)
//	->);

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
