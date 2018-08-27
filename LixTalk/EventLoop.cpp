#include "EventLoop.h"
#include "CurThread.h"

thread_local EventLoop* thisThreadLoop = nullptr;
const int EventLoop::kPollTimeMs = 2000;

EventLoop* EventLoop::thisThreadEventLoop() {
	return thisThreadLoop;
}


EventLoop::EventLoop()
	:looping_(false), quit_(false), poller_(new Poller(this)), thread_id_(CurThread::tid()) {
	if (thisThreadLoop == nullptr) {
		thisThreadLoop = this;
	}
	else {
		//error
		exit(12);
	}
}

EventLoop::~EventLoop() {
	if (looping_ == false) {
		exit(11);
		//error
	}
	thisThreadLoop = nullptr;
}

void EventLoop::loop() {
	looping_ = true;
	quit_ = false;
	while (!quit_) {
		activeList_.clear();
		poller_->poll(kPollTimeMs, &activeList_);
		for (auto it = activeList_.begin(); it != activeList_.end(); ++it) {
			(*it)->handleEvent();
		}
	}

	looping_ = false;
}

void EventLoop::updateChannel(Channel* ch) {
	poller_->updateChannel(ch);
}

void EventLoop::removeChannel(Channel* ch) {
	poller_->removeChannel(ch);
}

