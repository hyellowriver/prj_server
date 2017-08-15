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
#include <map>
#include "thread_base.h"
#include "client_manage.h"
#define SERVER_IP "192.168.1.235"
#define SERVER_PORT 55555
#define MAX_EPOLL_EVENTS  10000
#define PENDING_TIMEOUT 180


using namespace std;

class io_mutex:public thread_base
{
public:
	queue<int > m_queue;
	map<int , time_t> m_pendingfd;
	map<int , client_manage *> m_manage;
public:
	//io_mutex *getInstance()
	io_mutex(){}
	~io_mutex();

//private:
	//static io_mutex *m_instance;
public:
	int socket_init(const char *ip, const int port);
	int set_nonblock(int fd); 
	void reset_oneshot(int epollfd, int fd);

};

#endif