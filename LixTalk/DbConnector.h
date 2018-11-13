#ifndef LIXTALK_DBCONNECTOR
#define LIXTALK_DBCONNECTOR

#include <cppconn/resultset.h>
#include <cppconn/driver.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/exception.h>
#include <memory>
#include <hiredis/hiredis.h>
#include "LogInfo.h"

class DbConnector {
public:
	DbConnector();

	void connect(std::string username, std::string password);

	int getMaxUserID();

	int getChatMsgIdx();

	int getChatMsgCount(int idx);

	std::shared_ptr<sql::ResultSet> getUser(std::string username);

	void addUser(std::string username, std::string password);

	void addFriend(int userID_1, int userID_2);

	std::unique_ptr<sql::ResultSet> queryFriend(int id);

	void addOfflineMsg(int id, std::string msg);

	std::shared_ptr<std::vector<std::string>> getOfflineMsg(int id);

	void createUserSeqTable(int idx);

	void createChatMsgTable(int idx);

	void createUserTable();

	void createFriendTable();

	void createStatusTable();

	void initDb();

	void saveMsg(int sender_id, int recver_id, std::string& msg, int idx, int type);

	std::unique_ptr<sql::ResultSet> pullMsg(int id,int chatHisIdx, int num = 200);

	void updateStatusMsgIdx(int idx);

	~DbConnector();

	static const char * DbName;

private:
    sql::Driver* driver_;
    sql::Connection* con;
    redisContext *redisCon;
};


#endif
