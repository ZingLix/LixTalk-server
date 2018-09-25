#include "Poller.h"
#include "Channel.h"

void Poller::poll(int timeoutMs, ChannelList* activeChannels) {
	int numEvents = ::poll(&*pollfds_.begin(), pollfds_.size(), timeoutMs);
	int savedErrno = errno;
	if (numEvents > 0)
	{
		fillActiveChannels(numEvents, activeChannels);
	}
	else if (numEvents == 0)
	{
		//LOG_TRACE << " nothing happened";
	}
	else
	{
		if (savedErrno != EINTR)
		{
			errno = savedErrno;
			//LOG_SYSERR << "PollPoller::poll()";
		}
	}
}

void Poller::removeChannel(Channel* channel) {
	int idx = channel->index();
	//assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
	const struct pollfd& pfd = pollfds_[idx];
	(void)pfd;
	//assert(pfd.fd == -channel->fd() - 1 && pfd.events == channel->events());
	/*size_t n = */channel_map_.erase(channel->fd());
	//assert(n == 1); (void)n;
	if (static_cast<size_t>(idx) == pollfds_.size() - 1)
	{
		pollfds_.pop_back();
	}
	else
	{
		int channelAtEnd = pollfds_.back().fd;
		iter_swap(pollfds_.begin() + idx, pollfds_.end() - 1);
		if (channelAtEnd < 0)
		{
			channelAtEnd = -channelAtEnd - 1;
		}
		channel_map_[channelAtEnd]->set_index(idx);
		pollfds_.pop_back();
	}
}

void Poller::fillActiveChannels(int num, ChannelList* chs) const {
	for (PollFdList::const_iterator pfd = pollfds_.begin();
		pfd != pollfds_.end() && num > 0; ++pfd)
	{
		if (pfd->revents > 0)
		{
			--num;
			ChannelMap::const_iterator ch = channel_map_.find(pfd->fd);
			//assert(ch != channels_.end());
			Channel* channel = ch->second;
			//assert(channel->fd() == pfd->fd);
			channel->set_revents(pfd->revents);
			// pfd->revents = 0;
			chs->push_back(channel);
		}
	}
}

void Poller::updateChannel(Channel* channel) {
	//Poller::assertInLoopThread();
	//LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events();
	if (channel->index() < 0)
	{
		// a new one, add to pollfds_
		//assert(channels_.find(channel->fd()) == channels_.end());
		struct pollfd pfd;
		pfd.fd = channel->fd();
		pfd.events = static_cast<short>(channel->events());
		pfd.revents = 0;
		pollfds_.push_back(pfd);
		int idx = static_cast<int>(pollfds_.size()) - 1;
		channel->set_index(idx);
		channel_map_[pfd.fd] = channel;
	}
	else
	{
		// update existing one
		//assert(channels_.find(channel->fd()) != channels_.end());
		//assert(channels_[channel->fd()] == channel);
		int idx = channel->index();
		//assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
		struct pollfd& pfd = pollfds_[idx];
		//assert(pfd.fd == channel->fd() || pfd.fd == -channel->fd() - 1);
		pfd.fd = channel->fd();
		pfd.events = static_cast<short>(channel->events());
		pfd.revents = 0;
		if (channel->isNoneEvent())
		{
			// ignore this pollfd
			pfd.fd = -channel->fd() - 1;
		}
	}
}
