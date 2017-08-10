#include "thread_base.h"

void thread_base::lock(pthread_mutex_t &m_mutexs)//使用引用方法来达到对共享互斥量统一使用
{

	pthread_mutex_lock(&m_mutexs);

}
void thread_base::unlock(pthread_mutex_t &m_mutexs)
{

	pthread_mutex_unlock(&m_mutexs);

}

thread_base::thread_base()
{

	pthread_mutex_init(&m_mutex, NULL);
	pthread_mutex_init(&m_pending, NULL);
}

thread_base::~thread_base()
{

	pthread_mutex_destroy(&m_mutex);
	pthread_mutex_destroy(&m_pending);

}
