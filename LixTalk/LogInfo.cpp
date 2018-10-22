#include "LogInfo.h"
#include <type_traits>

const char* LogLevelName[Logger::NUM_LOG_LEVELS] =
{
  "TRACE ",
  "DEBUG ",
  "INFO  ",
  "WARN  ",
  "ERROR ",
  "FATAL ",
};

//template <typename T>
//LogInfo& LogInfo::operator<<(const T& content) {
//	buf.push_back(content.begin(),content.end());
//	return *this;
//}


LogInfo & LogInfo::operator<<(const char * content)
{
	std::string str(content);
	return operator<<(" " + str);
}

LogInfo& LogInfo::operator<<(const std::string& content) {
	buf += " " + content;
	return *this;
}

LogInfo::LogInfo(LogLevel level, std::string filename, int line)
:buf(),logLevel_(level),sourcefile(filename),line_(line){
	buf+=LogLevelName[logLevel_];
	operator<<(sourcefile);
	operator<<(std::to_string(line));
}

LogInfo::~LogInfo() {
	buf+=("\n");
	logger.addLog(std::move(buf));
}
