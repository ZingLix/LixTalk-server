#include "CurThread.h"
#include <iostream>
#include <unistd.h>
#include "ChatServer.h"
#include "logging.h"
#include <fcntl.h>
#include "LogInfo.h"
#include <iomanip>

int main() {
    std::cout << "pid:" << getpid() << ", tid:" << CurThread::tid() << std::endl;

 //   ChatServer server(9981);
//    server.start();
	std::time_t t = std::time(nullptr);
	std::cout  << std::put_time(std::gmtime(&t), "%c %Z") << '\n';

    int cnt = 5000000;
    while (cnt--) {
        LOG_INFO << "asdf" << "??" << std::to_string(cnt) << "ieie";
    }

	std::time_t t2 = std::time(nullptr);
	std::cout  << std::put_time(std::gmtime(&t2), "%c %Z") << '\n';

    std::cin.get();
}
