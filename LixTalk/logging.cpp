#include "logging.h"
#include <fcntl.h>



Logger::Logger():thread_(&Logger::loop,this) {
	std::time_t time = std::time(nullptr);
	auto res = std::gmtime(&time);
	char filename[50];
	std::strftime(filename, sizeof(filename), "%Y%m%d%H%M%S.log", res);
	fd = ::open(filename, O_RDWR | O_APPEND | O_CREAT,0777);
	looping_ = true;
	thread_.detach();
}

Logger::~Logger() {
	::close(fd);
}

void Logger::loop() {
	while(looping_) {
		if(buf1.size()>LOG_BUFFER_SIZE_LIMIT) {
			{
				std::lock_guard<std::mutex> lock(buf_mutex_);
				buf1.swap(buf2);
			}
			std::string entireLog;
			for(auto buf:buf2) {
				std::string log;
				for(auto str:buf) {
					log += str + " ";
				}
				entireLog += log;
			}
			buf2.clear();
			::write(fd, entireLog.c_str(), entireLog.length());
		}
	}
}
