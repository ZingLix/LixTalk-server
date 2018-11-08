#ifndef LIXTALK_CHATSERVER
#define LIXTALK_CHATSERVER
#include "Server.h"
#include "user.h"
#include "DbConnector.h"

template<class T1,class T2>
class DuplexMap
{
public:
	DuplexMap():FdToId(),IdToFd() {}

	void insert(T1 fd, T2 id) {
		FdToId.insert(std::make_pair(fd, id));
		IdToFd.insert(std::make_pair(id, fd));
	}
	void removeById(T2 id) {
		int fd = IdToFd[id];
		FdToId.erase(fd);
		IdToFd.erase(id);
	}
	void removeByFd(T1 fd) {
		int id = FdToId[fd];
		FdToId.erase(fd);
		IdToFd.erase(id);
	}
	T1 getFd(T2 id) {
		auto it = IdToFd.find(id);
		if (it == IdToFd.end()) return -1;
		return it->second;
	}
	T2 getId(T1 fd) {
		auto it = FdToId.find(fd);
		if (it == FdToId.end()) return -1;
		return it->second;
	}

private:
	std::map<T1, T2> FdToId;
	std::map<T2, T1> IdToFd;
};


class ChatServer
{
public:
	ChatServer(in_port_t port);

	void start();

	void msgExec_login(int fd, message& msg);
	void msgExec_register(int fd, message& msg);
	void msgExec_err(int fd, std::string errMsg);
	void msgExec_err_fatal(int fd, std::string errMsg);
	void msgExec_friend(int fd, message& msg);
	bool onNewConn(int fd, const NetAddress& addr, Server* serv);
	void execUnsentMsg(int id);
	void waitingFirstMsg(int fd, std::string msg, Server* serv);

	void forwardMsg(int sender_id, int recver_id, std::string msg);

	void recvMsg(int fd, std::string msg, Server* serv);

	void sendMsg(const int fd,std::string msg) {
		server_.send(fd, msg.append("\r\n\r\n"));
	}

	void saveMsg(int sender_id, int recver_id,std::string& msg);
	int checkLoginInfo(message& msg);
	void logout(int fd);

	void friend_request(int sender_id, int recver_id,std::string content);
	void friend_accepted(int user1_id, int user2_id);
	void friend_refused(int user1_id, int user2_id);
	void friend_list(int id);

private:
	std::mutex mutex_;
	std::shared_ptr<std::vector<std::string>> split(const std::string& str);
	std::map<int, user> userMap_;
	DuplexMap<int,int> socketMap_;
	int cur_user_count;
	int cur_chatmsg_idx;
	int cur_chatmsg_count;
	Server server_;
	DbConnector db_;
};


#endif
