#include "Server.h"
#include "unistd.h"
#include <iostream>

Server::Server(in_port_t port):state_(WAITING),addr_(port){
	if(EventLoop::thisThreadEventLoop()==nullptr) {
		event_loop_ = std::make_shared<EventLoop>();
	}else {
		event_loop_.reset(EventLoop::thisThreadEventLoop());
	}
	socket_ = std::make_shared<Socket>();
	channel_ = std::make_shared<Channel>(&*event_loop_,*socket_);
	channel_->setReadCallback(std::bind(&Server::handleNewConn, this));
}

void Server::start() {
	socket_->bind(addr_);
	socket_->listen();
	channel_->enableReading();
	state_ = RUNNING;
	event_loop_->loop();
}

void Server::setNewConnCallback(const NewConnCallback& cb) {
	ConnCallback_ = cb;
}

void Server::setDefaultEventCallback(const EventCallback& cb) {
	defaultEventCb_ = cb;
}

void Server::setDefaultMessageCallback(const MessageCallback& cb) {
	MessageCallback_ = cb;
}

void Server::setCloseCallback(const CloseCallback& cb) {
	CloseCallback_ = cb;
}

void Server::setMessageCallback(const int fd, const MessageCallback& cb) {
	messageMap_[fd] = cb;
}


void Server::handleNewConn() {
	NetAddress peerAddr;
	int connfd = socket_->accept(&peerAddr);
	if (connfd >= 0) {
		bool n = true;
		if (ConnCallback_) {
			n = ConnCallback_(connfd, peerAddr,this);
		}
		if(n) {
			createNewChannel(connfd);
		}
		
	}
}

void Server::createNewChannel(int fd) {
	std::shared_ptr<Channel> ptr = std::make_shared<Channel>(&*event_loop_, fd);
	channelMap_.insert(std::make_pair(fd, ptr));
	ptr->setReadCallback(std::bind(&Server::handleRead,this, std::placeholders::_1));
	ptr->enableReading();
	BufferMap_.insert(std::make_pair(fd, std::vector<char>()));
	messageMap_.insert(std::make_pair(fd, MessageCallback_));
}

void Server::handleRead(const int fd) {
	std::vector<char>& Buffer = BufferMap_[fd];
	//auto it = Buffer.begin();
	Buffer.clear();
	ssize_t n;
	char buf[32];
	while( (n = read(fd,buf,32) ) != 0 ) {
		if (n == -1) {
			if (errno == EAGAIN) {
				break;
			}else {
				std::cout << "error! " << errno << std::endl;
			}
		}
		Buffer.insert(Buffer.end(), buf, buf + n);
		
	}
	if (Buffer.size() == 0) {
		if (CloseCallback_) {
			CloseCallback_(fd, this);
			shutdown(fd);
		}
		event_loop_->removeChannel(&*channelMap_[fd]);
	}else {
		std::string msg(Buffer.begin(), Buffer.end());
		if (messageMap_[fd] != nullptr)
			messageMap_[fd](fd, msg, this);
	}
}

void Server::send(const int fd, std::string msg) {
	//Socket::send(fd, msg);
	int n = ::write(fd, &*msg.begin(), msg.length());
	//std::cout << errno << std::endl;
}

void Server::shutdown(int fd) {
	::shutdown(fd, SHUT_WR);
}
