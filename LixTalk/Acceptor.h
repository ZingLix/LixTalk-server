#pragma once
#include <functional>
#include <sys/socket.h>
#include <netinet/in.h>
#include "Channel.h"
#include "Socket.h"

class Acceptor
{
public:
	using ConnCallback = std::function<void(int, const NetAddress&)>;
	Acceptor(EventLoop* loop, const sockaddr_in&);
	void listen();
	void setNewConnectionCallback(const ConnCallback& cb) { newConnCallback_ = cb; }

private:
	void handleRead();

	EventLoop* loop_;
	Socket sock;
	Channel acceptChannel;
	ConnCallback newConnCallback_;
	bool listening;
};
