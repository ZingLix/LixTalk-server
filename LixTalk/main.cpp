#include "CurThread.h"
#include "EventLoop.h"
#include "NetAddress.h"
#include <iostream>
#include <unistd.h>
#include "Acceptor.h"
#include "Server.h"
#include "ChatServer.h"

bool newConnection(int sockfd,const NetAddress& peeraddr, Server* server) {
	std::cout << "new connection from " << peeraddr.port()<<std::endl;
	server->send(sockfd, "how are you?");
	fflush(stdout);
	return true;
//	Socket::close(sockfd);
}

void readCB(int fd,std::string str, Server* server) {
	std::cout << str << std::endl;
	fflush(stdout);
	//sleep(3);
}

void closeCB(int fd, Server* server) {
	std::cout << fd<<"closed." << std::endl;
	fflush(stdout);
	//sleep(3);
}

int main() {
	std::cout << "pid:"<<getpid()<<", tid:"<<CurThread::tid()<<std::endl;
	//EventLoop loop;  
	//NetAddress listenAddr(9981);

	//Acceptor acceptor(&loop, listenAddr);
	//acceptor.setNewConnectionCallback(newConnection);
	//acceptor.listen();
	//loop.loop();
	ChatServer server(9981);
	server.start();
}
