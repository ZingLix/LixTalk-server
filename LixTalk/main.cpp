#include "CurThread.h"
#include <unistd.h>
#include "ChatServer.h"
#include "psyche/psyche.h"
#include <functional>

int main() {
	LOG_INFO << "Running on pid:" << getpid() << ", tid:" << CurThread::tid();

	ChatServer server(9981);
	server.start();

	return 0;
}
