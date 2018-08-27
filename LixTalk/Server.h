#ifndef LIXTALK_NET_SERVER
#define LIXTALK_NET_SERVER
#include <arpa/inet.h>
#include <memory>
#include "EventLoop.h"
#include "NetAddress.h"
#include "Socket.h"

class Server
{
public:
	using NewConnCallback = std::function<bool(int, const NetAddress&,Server*)>;
	using EventCallback = std::function<void(const int, Server*)>;
	using MessageCallback = std::function<void(int, std::string, Server*)>;
	using CloseCallback = std::function<void(int, Server*)>;

	Server(in_port_t port);
	void start();
	void setNewConnCallback(const NewConnCallback& cb);
	void setDefaultEventCallback(const EventCallback& cb);
	void setDefaultMessageCallback(const MessageCallback& cb);
	void setMessageCallback(const int fd,const MessageCallback& cb);
	void setCloseCallback(const CloseCallback& cb);
	void send(const int fd, std::string msg);

	void shutdown(int fd);

private:
	void handleNewConn();
	void createNewChannel(int fd);
	void handleRead(const int fd);

	enum state{WAITING,RUNNING,CLOSED};

	EventCallback defaultEventCb_;
	NewConnCallback ConnCallback_;
	MessageCallback MessageCallback_;
	CloseCallback CloseCallback_;

	std::shared_ptr<EventLoop> event_loop_;
	state state_;
	NetAddress addr_;
	std::shared_ptr<Socket> socket_;
	std::shared_ptr<Channel> channel_;
	std::map<int,std::shared_ptr<Channel>> channelMap_;
	std::map<int,MessageCallback> messageMap_;
	std::map<int, std::vector<char>> BufferMap_;
};


#endif
