#include "Acceptor.h"

Acceptor::Acceptor(EventLoop* loop,const sockaddr_in& listenAddr)
	:loop_(loop),
	sock(),
	acceptChannel(loop,sock.fd()),
	listening(false)
{
	sock.bind(listenAddr);
	acceptChannel.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

void Acceptor::listen() {
	listening = true;
	sock.listen();
	acceptChannel.enableReading();
}

void Acceptor::handleRead() {
	NetAddress peerAddr;
	int connfd = sock.accept(&peerAddr);
	if(connfd>=0) {
		if(newConnCallback_) {
			newConnCallback_(connfd, peerAddr);
		}else {
			Socket::close(connfd);
		}
	}
}
