#ifndef IO_MUTEX_H
#define IO_MUTEX_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/epoll.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <queue>

#include "thread_base.h"
#define SERVER_IP "192.168.1.235"
#define SERVER_PORT 55555
#define MAX_EPOLL_EVENTS  10000

using namespace std;

class io_mutex:public thread_base
{
public:
	queue<int > m_queue;
public:
	//io_mutex *getInstance()
	io_mutex(){}
	~io_mutex();

//private:
	//static io_mutex *m_instance;
public:
	int socket_init(const char *ip, const int port);
	int set_nonblock(int fd); 


};

#endif