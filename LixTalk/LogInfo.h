#ifndef LIXTALK_LOGINFO
#define LIXTALK_LOGINFO

#include "logging.h"
#include <vector>

class LogInfo
{
public:
	using LogLevel = Logger::LogLevel;

	LogInfo(LogLevel level, std::string filename, int line);

	//template <typename T>
	//LogInfo& operator<<(const T&content);

	LogInfo& operator<<(const char * content);
	LogInfo& operator<<(const std::string& content);
	~LogInfo();

private:
	std::string buf;
	LogLevel logLevel_;
	std::string sourcefile;
	int line_;
};


#define LOG_TRACE LogInfo(Logger::TRACE,__FILE__, __LINE__)
#define LOG_TRACE LogInfo(Logger::TRACE,__FILE__, __LINE__)
#define LOG_INFO LogInfo(Logger::INFO,__FILE__, __LINE__)
#define LOG_WARN LogInfo(Logger::WARN,__FILE__, __LINE__)
#define LOG_ERROR LogInfo(Logger::ERROR,__FILE__, __LINE__)
#define LOG_FATAL LogInfo(Logger::FATAL,__FILE__, __LINE__)


#endif
