#include "thread_base.h"

void thread_base::lock()
{

	pthread_mutex_lock(&m_mutex);

}
void thread_base::unlock()
{

	pthread_mutex_unlock(&m_mutex);

}

thread_base::thread_base()
{
	pthread_mutex_init(&m_mutex, NULL);
}

thread_base::~thread_base()
{

	pthread_mutex_destroy(&m_mutex);

}
