#ifndef LIXTALK_MESSAGE
#define LIXTALK_MESSAGE

#include <string>
#include <map>
#define RAPIDJSON_HAS_STDSTRING 1
#include "rapidjson/document.h"

class message
{
public:
	message(std::string str) {
		document.Parse(str.c_str());
		if (/*!(document.HasMember("sender_id")&&
			document.HasMember("recver_id")&&
			document.HasMember("content")&&
			document.HasMember("type"))||*/
			document.HasParseError()) {
			throw;
		}
	}

	bool parse(std::string str);

	int getInt(const char* key) {
		return document[key].GetInt();
	}

	std::string getString(const char* key) {
		return document[key].GetString();
	}

private:
	rapidjson::Document document;
};

#endif
