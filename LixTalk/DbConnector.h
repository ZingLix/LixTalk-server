#ifndef LIXTALK_DBCONNECTOR
#define LIXTALK_DBCONNECTOR

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <memory>

class DbConnector
{
public:
	DbConnector():con(nullptr) {
		driver_ = get_driver_instance();
	}

	void connect(std::string username,std::string password) {
		con = driver_->connect("tcp://127.0.0.1:3306", username, password);
		con->setSchema("test");
	} 

	int getMaxUserID() {
		sql::PreparedStatement *stmt = con->prepareStatement("SELECT Max(id) from user");
		std::shared_ptr<sql::ResultSet> res (stmt->executeQuery());
		delete stmt;
		return res->getInt("id");
	}

	std::shared_ptr < sql::ResultSet > getUser(std::string username) {
		std::shared_ptr< sql::Statement > stmt (con->createStatement());
		//	stmt->setString(1, username);

		stmt->execute("SELECT * from user where username = '"+username+"'");
	//	std::cout<<stmt->getResultSet()->getString(1);
		return std::shared_ptr < sql::ResultSet >(stmt->getResultSet());
	}
	
	void addUser(std::string username, std::string password) {
		sql::PreparedStatement *stmt = con->prepareStatement("INSERT INTO user(id,username,password) VALUES (null,?,?)");
		stmt->setString(1, username);
		stmt->setString(2, password);
		stmt->executeUpdate();
		delete stmt;
	}

	void addFriend(int userID_1,int userID_2) {
		sql::PreparedStatement *stmt = con->prepareStatement("INSERT INTO friend(userID_1,userID_2) VALUES ("
			+ std::to_string(userID_1) + " , " + std::to_string(userID_2) +")");
		stmt->executeUpdate();
		delete stmt;
	}

	auto queryFriend(int id) {
		sql::PreparedStatement *stmt = con->prepareStatement("SELECT userID_1,userID_2,groupID_in_1,groupID_in_2 \
			from friend where userID_1 = "+std::to_string(id)+" or userID_2 = "+ std::to_string(id));
		std::shared_ptr<sql::ResultSet> res (stmt->executeQuery()) ;
		delete stmt;
		return res;
	}

	~DbConnector() {
		if(con!=nullptr)
		delete con;
	}

private:
	sql::Driver* driver_;
	sql::Connection* con;
};

#endif
