#include "Channel.h"
#include <poll.h>

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

void Channel::handleEvent() {
	if (revents_&POLLNVAL) {
		//warn
	}
	if (revents_&(POLLERR | POLLNVAL)) {
		if (errorCallback_) errorCallback_(fd_);
	}
	if (revents_&(POLLIN | POLLPRI)) {
		if (readCallback_) readCallback_(fd_);
	}
	if (revents_&POLLOUT) {
		if (writeCallback_) writeCallback_(fd_);
	}
}

void Channel::update() {
	loop_->updateChannel(this);
}
