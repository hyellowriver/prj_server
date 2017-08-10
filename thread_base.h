#ifndef THREAD_BASE_H
#define THREAD_BASE_H

#include <pthread.h>

class thread_base
{

public:
	pthread_t m_thread;
	pthread_mutex_t m_mutex;
	pthread_mutex_t m_pending;
	pthread_cond_t m_cond;
public:
	void lock(pthread_mutex_t &m_mutexs);
	void unlock(pthread_mutex_t &m_mutexs);
	thread_base();
	~thread_base();
};

#endif