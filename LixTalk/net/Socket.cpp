#include "Socket.h"
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <cstdio>

Socket::Socket() {
	sock_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
}

Socket::Socket(const int sockfd) :sock_fd_(sockfd) {}

Socket::~Socket() {
	::close(sock_fd_);
}

void Socket::bind(NetAddress& addr) const {
	int n = ::bind(sock_fd_, sockaddr_cast(addr.addr()), sizeof(*addr.addr()));
	if (n == 0) {
		printf("bind success!\n");
	}
	else {
		printf("bind error!\n");
	}
}

void Socket::bind(const char* ip, in_port_t port) const {
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
	if (::listen(sock_fd_, backlog) != 0) {
		printf("listen error!\n");
	}
}

int Socket::accept(NetAddress* addr) const {
	struct sockaddr_in add;
	bzero(&add, sizeof(add));
	socklen_t len = sizeof(add);
	const int connfd = ::accept4(sock_fd_, sockaddr_cast(add), &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
	if (connfd >= 0)
	{
		addr->set(add);
	}
	return connfd;
}

int Socket::connect(NetAddress* addr) const {
	sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
	socklen_t len = sizeof(servaddr);
	int connfd = ::connect(sock_fd_, sockaddr_cast(servaddr), len);
	if (connfd >= 0) {
		addr->set(servaddr);
	}
	return connfd;
}


void Socket::close() {
	::close(sock_fd_);
}

void Socket::close(int fd) {
	::close(fd);
}