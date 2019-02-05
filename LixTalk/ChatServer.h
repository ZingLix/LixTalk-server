#ifndef LIXTALK_CHATSERVER
#define LIXTALK_CHATSERVER
#include "psyche/psyche.h"
#include "user.h"
#include "DbConnector.h"
#include <map>

//template<class T1,class T2>
//class DuplexMap
//{
//public:
//	DuplexMap():FdToId(),IdToFd() {}
//
//	void insert(T1 fd, T2 id) {
//		FdToId.insert(std::make_pair(fd, id));
//		IdToFd.insert(std::make_pair(id, fd));
//	}
//	void removeById(T2 id) {
//		psyche::Connection conn = IdToFd[id];
//		FdToId.erase(fd);
//		IdToFd.erase(id);
//	}
//	void removeByFd(T1 fd) {
//		int id = FdToId[fd];
//		FdToId.erase(fd);
//		IdToFd.erase(id);
//	}
//	T1 getFd(T2 id) {
//		auto it = IdToFd.find(id);
//		if (it == IdToFd.end()) return -1;
//		return it->second;
//	}
//	T2 getId(T1 fd) {
//		auto it = FdToId.find(fd);
//		if (it == FdToId.end()) return -1;
//		return it->second;
//	}
//
//private:
//	std::map<T1, T2> FdToId;
//	std::map<T2, T1> IdToFd;
//};


class ChatServer
{
public:
	ChatServer(in_port_t port);

	void start();

	void msgExec_login(psyche::Connection conn, message& msg);
	void msgExec_register(psyche::Connection conn, message& msg);
	void msgExec_err(psyche::Connection conn, std::string errMsg);
	void msgExec_err_fatal(psyche::Connection conn, std::string errMsg);
	void msgExec_friend(psyche::Connection conn, message& msg);
	bool onNewConn(psyche::Connection conn);
	void execUnsentMsg(int id);
	void waitingFirstMsg(psyche::Connection conn, psyche::Buffer buffer);

	void forwardMsg(int sender_id, int recver_id, std::string msg);
	void pullMsg(psyche::Connection conn, message& m);
	void recvMsg(psyche::Connection conn, psyche::Buffer buffer);

	void sendMsg(psyche::Connection conn,std::string msg) {
		conn.send(msg.append("\r\n\r\n"));
	}

	void saveMsg(int sender_id, int recver_id,std::string& msg);
	int checkLoginInfo(message& msg);
	void logout(psyche::Connection conn);

	void friend_request(int sender_id, int recver_id,std::string content);
	void friend_accepted(int user1_id, int user2_id);
	void friend_refused(int user1_id, int user2_id);
	void friend_list(int id);

private:
	std::shared_ptr<std::vector<std::string>> split(const std::string& str);

	std::mutex mutex_;
	std::set<psyche::Connection> vistor_;
	std::map<int, psyche::Connection> user_;
	std::map<psyche::Connection, int> con_to_id_;
	int cur_user_count;
	int cur_chatmsg_idx;
	int cur_chatmsg_count;
	psyche::Server server_;
	DbConnector db_;
};


#endif
