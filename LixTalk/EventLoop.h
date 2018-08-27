#ifndef LIXTALK_NET_EVENTLOOP
#define LIXTALK_NET_EVENTLOOP

#include <memory>
#include <vector>
#include "Poller.h"
#include "Channel.h"

class EventLoop
{
public:
	EventLoop();
	~EventLoop();

	void updateChannel(Channel*);
	void removeChannel(Channel*);
	void loop();
	static EventLoop* thisThreadEventLoop();


	static const int kPollTimeMs;

private:
	using ChannelList = std::vector<Channel*>;

	bool looping_;
	bool quit_;
	std::unique_ptr<Poller> poller_;
	ChannelList activeList_;
	const pthread_t thread_id_;


};

#endif
