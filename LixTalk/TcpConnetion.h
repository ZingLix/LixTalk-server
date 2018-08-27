#ifndef LIXTALK_NET_TCPCONNECTION
#define LIXTALK_NET_TCPCONNECTION
#include "Socket.h"
#include "EventLoop.h"

class TcpConnection
{
public:
	TcpConnection();

private:
	EventLoop* event_loop_;
	Socket socket_;
};

#endif
