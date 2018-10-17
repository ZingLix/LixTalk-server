#ifndef LIXTALK_LOGGING
#define LIXTALK_LOGGING
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>
#include <mutex>


class Logger
{
public:
	enum LogLevel
	{
		TRACE,
		DEBUG,
		INFO,
		WARN,
		ERROR,
		FATAL,
		NUM_LOG_LEVELS,
	};

	using Buffer = std::vector<std::string>;

	const size_t LOG_BUFFER_SIZE_LIMIT = 0;

	Logger();
	~Logger();

	template<typename T>
	void write(const T& content);

	void addLog(Buffer&& buf) {
		std::lock_guard<std::mutex> lock(buf_mutex_);
		buf1.push_back(buf);
	}

	void loop();

private:
	int fd;
	std::thread thread_;
	std::vector<Buffer> buf1, buf2;
	bool looping_;
	std::mutex buf_mutex_;
};


static Logger logger;

#endif
