#include "CurThread.h"
#include <iostream>
#include <unistd.h>
#include "ChatServer.h"
#include "logging.h"
#include <fcntl.h>
#include "LogInfo.h"

int main() {
	std::cout << "pid:"<<getpid()<<", tid:"<<CurThread::tid()<<std::endl;

	//ChatServer server(9981);
	//server.start();

	int cnt = 5000000;
	while (cnt--) {
		LOG_INFO << "asdf" << "??" << std::to_string(cnt) << "ieie";
	}



	std::cin.get();
}

