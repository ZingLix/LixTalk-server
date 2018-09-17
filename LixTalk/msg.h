#ifndef LIXTALK_MESSAGE
#define LIXTALK_MESSAGE

#include <string>
#include <map>
#define RAPIDJSON_HAS_STDSTRING 1
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

class message
{
public:
	message() :allocator(document.GetAllocator()) {
		document.SetObject();
	}

	message(std::string str) :allocator(document.GetAllocator()) {
		document.Parse(str.c_str());
		if (document.HasParseError()) {
			throw std::invalid_argument("Parse Error");
		}
	}

	void add(std::string key, int val) {
		rapidjson::Value n(key.c_str(), allocator);
		document.AddMember(n, val, allocator);
	}

	void add(std::string key, std::string val) {
		rapidjson::Value k(key.c_str(), allocator);
		rapidjson::Value v(val.c_str(), allocator);
		document.AddMember(k, v, allocator);
	}

	void clear() {
		document.SetObject();
	}

	bool parse(std::string str);

	int getInt(const char* key) {
		return document[key].GetInt();
	}

	std::string getString() {
		rapidjson::StringBuffer buffer;
		buffer.Clear();
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		document.Accept(writer);
		return buffer.GetString();
	}

	std::string getString(const char* key) {
		return document[key].GetString();
	}

	rapidjson::MemoryPoolAllocator<>& getAllocator() {
		return allocator;
	}

	void add(std::string key,rapidjson::Value&& val) {
		rapidjson::Value k(key.c_str(), allocator);
		document.AddMember(k, val, allocator);
	}

private:
	rapidjson::Document document;
	rapidjson::MemoryPoolAllocator<>& allocator;
};

#endif
