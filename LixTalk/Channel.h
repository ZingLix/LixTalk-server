#ifndef LIXTALK_NET_CHANNEL
#define LIXTALK_NET_CHANNEL
#include <functional>
#include "EventLoop.h"
#include "Socket.h"

class Channel
{
public:
	using EventCallback = std::function<void(const int)>;

	Channel(EventLoop* event_loop, int fd)
		:loop_(event_loop), fd_(fd), events_(0), revents_(0), idx_(-1)
	{}

	Channel(EventLoop* event_loop, const Socket& addr)
		:loop_(event_loop), fd_(addr.fd()), events_(0), revents_(0), idx_(-1)
	{}

	void setReadCallback(const EventCallback& cb){
		readCallback_ = cb;
	}
	void setWriteCallback(const EventCallback& cb){
		writeCallback_ = cb;
	}
	void setCloseCallback(const EventCallback& cb){
		closeCallback_ = cb;
	}
	void setErrorCallback(const EventCallback& cb){
		errorCallback_ = cb;
	}

	void enableReading() { events_ |= kReadEvent; update(); }
	void disableReading() { events_ &= ~kReadEvent; update(); }
	void enableWriting() { events_ |= kWriteEvent; update(); }
	void disableWriting() { events_ &= ~kWriteEvent; update(); }

	void handleEvent();

	void set_revents(int revent) { revents_ = revent; }
	int events() const { return events_; }
	int fd() const { return fd_; }
	bool isNoneEvent()const { return events() == kNoneEvent; }
	int index() const { return idx_; }
	void set_index(int index) { idx_ = index; }

private:
	void update();

	EventLoop* loop_;
	const int fd_;
	int events_;
	int revents_;
	int idx_;

	EventCallback readCallback_;
	EventCallback writeCallback_;
	EventCallback closeCallback_;
	EventCallback errorCallback_;

	static const int kNoneEvent;
	static const int kReadEvent;
	static const int kWriteEvent;
};


#endif
