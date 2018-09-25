#include "CurThread.h"
#include <iostream>
#include <unistd.h>
#include "ChatServer.h"


int main() {
	std::cout << "pid:"<<getpid()<<", tid:"<<CurThread::tid()<<std::endl;

	ChatServer server(9981);
	server.start();
}

