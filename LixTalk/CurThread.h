#ifndef LIXTALK_NET_CURTHREAD
#define LIXTALK_NET_CURTHREAD
#include <pthread.h>

namespace CurThread
{
	inline pthread_t tid() {
		return pthread_self();
	} 
}


#endif
