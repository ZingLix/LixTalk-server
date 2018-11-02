#include "Socket.h"
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <cstdio>
#include "LogInfo.h"

Socket::Socket() {
	sock_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
	int opt = 1;
	setsockopt(sock_fd_, SOL_SOCKET, SO_REUSEADDR,
		(const void *)&opt, sizeof(opt));
}

Socket::Socket(const int sockfd):sock_fd_(sockfd) {}

Socket::~Socket() {
	::close(sock_fd_);
}

void Socket::bind(NetAddress& addr) const {
	int n = ::bind(sock_fd_, sockaddr_cast(addr.addr()), sizeof(*addr.addr()));
	if(n==0) {
		LOG_INFO << "bind success on " << inet_ntoa(addr.addr()->sin_addr) <<":" << htons(addr.addr()->sin_port);
	} else {
		LOG_INFO << "bind error. errno:" << errno;
	}
}

void Socket::bind(const char* ip, in_port_t port) const{
	NetAddress addr(ip, port);
	bind(addr);
}

void Socket::bind(const sockaddr_in& sockaddr) const {
	NetAddress addr(sockaddr);
	bind(addr);
}

void Socket::bind(in_port_t port) const {
	NetAddress addr(port);
	bind(addr);
}

void Socket::listen(int backlog) const {
	if(::listen(sock_fd_, backlog)!=0) {
		LOG_ERROR << "listen error. errno: " << errno;;
	}
}

int Socket::accept(NetAddress* addr) const {
	struct sockaddr_in add;
	bzero(&add, sizeof(add));
	socklen_t len = sizeof(add);
	const int connfd = ::accept4(sock_fd_, sockaddr_cast(add), &len,SOCK_NONBLOCK|SOCK_CLOEXEC);
	if (connfd >= 0)
	{
		addr->set(add);
		LOG_INFO<<"fd "<<connfd<<": new connect from "<< inet_ntoa(addr->addr()->sin_addr) << ":" << htons(addr->addr()->sin_port);
	}
	return connfd;
}

int Socket::connect(NetAddress* addr) const {
	sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
	socklen_t len = sizeof(servaddr);
	int connfd = ::connect(sock_fd_, sockaddr_cast(servaddr), len);
	if(connfd>=0) {
		addr->set(servaddr);
	}
	return connfd;
}


void Socket::close() {
	if(::close(sock_fd_)==0)
		LOG_INFO << "fd " << sock_fd_ << ": closed.";
}

void Socket::close(int fd) {
	if (::close(fd) == 0)
		LOG_INFO << "fd " << fd << ": closed.";
}

void Socket::shutdown(int fd,int flag) {
	if(::shutdown(fd,flag)==0)
		LOG_INFO << "fd " << fd << ": shutdown.";
}
