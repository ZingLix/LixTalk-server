#ifndef LIXTALK_NET_NETADDRESS
#define LIXTALK_NET_NETADDRESS

#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

class NetAddress
{
public:
	NetAddress() {
		bzero(&sockaddr_, sizeof(sockaddr_));
	}

	NetAddress(const char* ip,in_port_t port) {
		bzero(&sockaddr_, sizeof(sockaddr_));
		in_addr_t ip_bin;
		inet_pton(AF_INET, ip, &ip_bin);
		sockaddr_.sin_addr.s_addr = htonl(ip_bin);
		sockaddr_.sin_family = AF_INET;
		sockaddr_.sin_port = htons(port);
	}

	NetAddress(in_port_t port) {
		bzero(&sockaddr_, sizeof(sockaddr_));
		sockaddr_.sin_addr.s_addr = htonl(INADDR_ANY);
		sockaddr_.sin_family = AF_INET;
		sockaddr_.sin_port = htons(port);
	}

	NetAddress(sockaddr_in sockaddr):sockaddr_(sockaddr) {
		
	}

	sockaddr_in* addr() { return &sockaddr_; }

	void set(const sockaddr_in& sockaddr) {
		sockaddr_ = sockaddr;
	}

	operator sockaddr_in() const {
		return sockaddr_;
	}

	in_port_t port() const{
		return ntohs(sockaddr_.sin_port);
	}

private:
	sockaddr_in sockaddr_;
};

#endif

