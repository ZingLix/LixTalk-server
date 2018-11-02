#ifndef LIXTALK_NET_SOCKET
#define LIXTALK_NET_SOCKET
#include "NetAddress.h"
#include <bits/stringfwd.h>
#include <unistd.h>
#include <string>

class Socket
{
public:
	Socket();
	explicit Socket(const int sockfd);
	~Socket();
	void bind(NetAddress& addr) const ;
	void bind(const char* ip, in_port_t port) const;
	void bind(in_port_t port)const;
	void bind(const sockaddr_in&) const;
	void listen(int backlog = 1000) const;
	int accept(NetAddress* addr) const;
	int connect(NetAddress* addr) const;
	void close();
	static void close(int fd);
	static void shutdown(int fd, int flag = SHUT_RDWR);

	int fd() const { return sock_fd_; }
	static void send(const int fd,std::string msg) {
		::write(fd, &*msg.begin(), msg.length());
	}
private:
	int sock_fd_;
};

inline sockaddr* sockaddr_cast(sockaddr_in& sock_in) {
	return reinterpret_cast<sockaddr*>(&sock_in);
}

inline sockaddr* sockaddr_cast(sockaddr_in* sock_in) {
	return reinterpret_cast<sockaddr*>(sock_in);
}
#endif


