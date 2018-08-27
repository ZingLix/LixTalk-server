#ifndef LIXTALK_NET_POLLER
#define LIXTALK_NET_POLLER
//#include "Channel.h"
#include <map>
#include <poll.h>
#include <vector>

class EventLoop;
class Channel;

class Poller
{
public:
	using ChannelList = std::vector<Channel*>;

	Poller(EventLoop* loop):ownerLoop_(loop){}

	void updateChannel(Channel*);
	void poll(int, ChannelList* activeChannels);
	void removeChannel(Channel* channel);

private:
	void fillActiveChannels(int num, ChannelList* chs) const;

	using ChannelMap = std::map<int, Channel*>;
	using PollFdList = std::vector<pollfd>;

	ChannelMap channel_map_;
	EventLoop* ownerLoop_;
	PollFdList pollfds_;
};




#endif
