#include "ChatServer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include <iostream>

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
		server_.send(fd, m);
	}

}

int ChatServer::checkLoginInfo(message& msg) {
	std::string username = msg.getString("username");
	auto userinfo = db_.getUser(username);
	userinfo->next();
	if(msg.getString("password")==userinfo->getString("password")) {
		return userinfo->getInt("id");
	}else {
		return -1;
	}
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
	server_.shutdown(fd);
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
				server_.send(fd, "recver offline");
			}
			break;
		default:
			msgExec_err(fd, "unknown kype!");
	}
}
