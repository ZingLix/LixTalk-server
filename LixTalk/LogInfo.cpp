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
	return operator<<(str);
}

LogInfo& LogInfo::operator<<(const std::string& content) {
	buf.push_back(content);

	return *this;
}

LogInfo::LogInfo(LogLevel level, std::string filename, int line)
:buf(10),logLevel_(level),sourcefile(filename),line_(line){
	buf.push_back(LogLevelName[logLevel_]);
	buf.push_back(sourcefile);
	buf.push_back(std::to_string(line));
}

LogInfo::~LogInfo() {
	buf.push_back("\n");
	logger.addLog(std::move(buf));
}
