#pragma once
#include "msg.h"

class user
{
public:
	user():id_(0) {
		
	}

	user(int id):id_(id) {
		//cur_seq_ = user.getseq();
		//max_seq_ = enlargeSeq(cur_seq_);
	}
	user(int id,int fd) :id_(id) {
		//cur_seq_ = user.getseq();
		//max_seq_ = enlargeSeq(cur_seq_);
	}
	user(std::string user_name) {
		user(get_id(user_name));
	}

	int get_id(std::string user_name) {
		return 10000;
	}

	bool operator== (const user& rhs) const {
		return rhs.id_ == id_;
	}

	int id_;
	long long cur_seq_;
	long long max_seq_;
};
