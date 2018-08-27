#ifndef LIXTALK_CHATSERVER
#define LIXTALK_CHATSERVER
#include "Server.h"
#include "user.h"

class ChatServer
{
public:
	ChatServer(in_port_t port):server_(port) {
		server_.setNewConnCallback(std::bind(&ChatServer::onNewConn, this,
			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	}

	void start() {
		server_.start();
	}

	void msgExec_login(int fd, message& msg);
	void msgExec_register(int fd, message& msg);
	void msgExec_err(int fd, std::string errMsg);
	void msgExec_err_fatal(int fd, std::string errMsg);

	bool onNewConn(int fd,const NetAddress& addr,Server* serv) {
		server_.setMessageCallback(fd, std::bind(&ChatServer::waitingFirstMsg, this,
			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 ));
		return true;
	}

	void waitingFirstMsg(int fd,std::string msg,Server* serv){
		message m(msg);
		if(m.getInt("recver_id")!=0) {
			serv->send(fd,"Bad request!");
			serv->shutdown(fd);
		}else {
			switch (m.getInt("type")) {
			case 0:  //login request
				msgExec_login(fd,m); break;
			case 1:  //register request
				msgExec_register(fd,m); break;
			default:
				msgExec_err(fd, "unknown type");
			}
		}
	}

	bool forwardMsg(int sender_id, int recver_id, std::string msg);

	void recvMsg(int fd, std::string msg, Server* serv) {
		message m(msg);
		switch (m.getInt("type")) {
		case 9:
			if(forwardMsg(m.getInt("sender_id"), m.getInt("recver_id"),msg) ==false) {
				server_.send(fd, "recver offline");
			}
			break;
		default:
			msgExec_err(fd, "unknown kype!");
		}
	}

	void sendMsg(const int fd,std::string msg) {
		server_.send(fd, msg);
	}

	int generateNewID() {
		return rand();
	}

private:
	std::map<int, user> userMap_;
	std::map<int, int> socketMap_;
	Server server_;
};


#endif
