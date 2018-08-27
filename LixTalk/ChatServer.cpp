#include "ChatServer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include <iostream>

void ChatServer::msgExec_login(int fd, message& msg) {
	//if(checkLoginInfo()){
	//userMap_.insert(std::make_pair(fd, msg.getString("sender_name")));
	//userMap_[fd] = user(msg.getString("sender_name"));
	//}else{
	//msgExec_err("login info error");
	//}

}

void ChatServer::msgExec_register(int fd, message& msg) {
	int user_id = generateNewID();
	userMap_[user_id] = user(user_id);
	socketMap_[user_id] = fd;
	rapidjson::Document doc;
	rapidjson::MemoryPoolAllocator<>& allocator = doc.GetAllocator();
	doc.SetObject();
	doc.AddMember("sender_id", 0, allocator);
	doc.AddMember("type", 3, allocator);
	doc.AddMember("recver_id", user_id, allocator);

	rapidjson::StringBuffer buffer;
	buffer.Clear();
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	doc.Accept(writer);
	std::string m = buffer.GetString();
	server_.send(fd, m);
	printf("new user:%d\n", user_id);
	server_.setMessageCallback(fd, std::bind(&ChatServer::recvMsg, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void ChatServer::msgExec_err_fatal(int fd, std::string errMsg) {
	msgExec_err(fd, errMsg);
	server_.shutdown(fd);
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
	server_.send(fd, errMsg);
	server_.shutdown(fd);
}

bool ChatServer::forwardMsg(int sender_id, int recver_id, std::string msg) {
	std::cout << "sender id:" << sender_id << std::endl;
	std::cout << "recver id:" << recver_id << std::endl;
	std::cout << "message:" << msg << std::endl;
	auto it = socketMap_.find(recver_id);
	if (it!=socketMap_.end()) {
		sendMsg(it->second, msg);
		std::cout << "forwarded\n";
		return true;
	}
	return false;
}
