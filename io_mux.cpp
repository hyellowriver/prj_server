#include "io_mutex.h"
#include <pthread.h>
#if 0
io_mutex *io_mutex::m_instance = NULL;

io_mutex *io_mutex::getInstance()
{
	if (io_mutex == NULL) {
		m_instance = new io_mutex;
	}
	return m_instance;

}
 

io_mutex::~io_mutex()
{
	m_instance = NULL;
}
#endif
/*
*/

int epollfd;
struct epoll_event ep_event;
int io_mutex::socket_init(const char *ip, const int port)
{
	struct sockaddr_in server_addr;
	int sockfd, reuse = 1;

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);

	if (inet_pton(PF_INET, ip, &server_addr.sin_addr) == -1)
		printf("inet_pton() error \n");
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		printf("socket() error \n");
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)
		printf("setsockopt() error \n");
	if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
		printf("bind() errno \n");
	if (listen(sockfd, 5) == -1)
		printf("listen() errno \n");

	return sockfd;
}


/*
	设置文件描述符为非阻塞
*/
int io_mutex::set_nonblock(int fd)
{
	int opt = fcntl(fd, F_GETFD);
	fcntl(fd, F_SETFD, opt | O_NONBLOCK);
	return opt;
}

#if 0
void io_mutex::reset_oneshot(int epollfd, int fd)
{
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET | EPOLLONSHOT;
	epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);



}
#endif
void *check_pend_timeout(void *arg)
{
	pthread_detach(pthread_self());
	io_mutex *io_ctr = (io_mutex *)arg;
	while (1){
		sleep(1);
		time_t tNow;
		time(&tNow);
		map<int, time_t>::iterator pos;
		io_ctr->lock(io_ctr->m_pending);
		for(pos = io_ctr->m_pendingfd.begin(); pos != io_ctr->m_pendingfd.end();)
		{
			time_t fd_timeBegin = pos->second;
			if (tNow - fd_timeBegin > PENDING_TIMEOUT){
				int fd = pos->first;
				//struct epoll_event ev;
				//ev.data.fd = fd;
				if (-1 == epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ep_event)){
					printf("error EPOLL_CTL_DEL \n");
				}
				printf("success EPOLL_CTL_DEL \n");
				io_ctr->m_pendingfd.erase(pos++);
				close(fd);
			}
			else{
				++pos;
			}
		}


		io_ctr->unlock(io_ctr->m_pending);


	}


}
size_t recv_data(int fd, char *buffer, size_t n)
{
	int nRet = 0;
	int pos = 0;
	memset(buffer, 0, n);
	while (1){
		/*确保较长的数据一次性读完，在ET模式下可以使用while循环读取*/
		nRet = recv(fd, buffer + pos, n - pos, 0);
		if (nRet == -1){
			if (errno == EAGAIN || errno == EWOULDBLOCK ){
				/*
				*EAGAIN 和 EWOULDBLOCK：非阻塞状态下，系统提示再试一次，非错误
				*在非阻塞ET模式下，即代表数据读完，但是通常情况下还需要比对私有
				*头标记的数据长度，如果数据长度不一致，需要再次读
				*/
				printf("get all data len = %d\n", pos);
				return pos;
			} else if (errno == EINTR){
			/*
			 *	
			 *	EINTR：系统中断
			 */
				continue;

			} else {
				printf("recv error\n");
				close(fd);//返回了异常错误码，直接关闭
				break;
			}


		} else if (nRet == 0){
			printf("client close\n");
			close(fd);	//客户端关闭
			break;
		} else {
			pos += nRet;
			printf(" get data: %s\n ", buffer);

			break;
		}
	}
			return pos;
}
void *worker(void *arg)
{
	pthread_detach(pthread_self());
	io_mutex *io_ctr = (io_mutex *)arg;
	int fd;
	while(true){
		io_ctr->lock(io_ctr->m_mutex);
		while(io_ctr->m_queue.empty()){
			pthread_cond_wait(&io_ctr->m_cond, &io_ctr->m_mutex);
		}
		fd = io_ctr->m_queue.front();
		io_ctr->m_queue.pop();
		printf("	   queue size = %d\n", io_ctr->m_queue.size());
		io_ctr->unlock(io_ctr->m_mutex);
		char buffer[1024] = {0};

		/*
		*工作线程，接收完整的数据，业务层解析私有协议
		*对于非法的连接或者协议不正确，关掉socket
		*
		*/
		size_t res = recv_data(fd, buffer, 1024);
		/*
		*目前私有协议头并没有规划好
		*在标识客户端连接和服务器处理客户端的对象
		*用fd来映射
		*
		*/
		client_manage *cli_m = new client_manage;
		io_ctr->m_manage.insert(pair<int, client_manage *>(fd, cli_m));
		//cli_m->deal_request(fd, buffer, res);


		ep_event.data.fd = fd;
		ep_event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;//读完数据重新修改属性
		epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ep_event);
	}
}


int main(int argc, char const *argv[])
{
	/*
	 * 信号处理统一函数
	 *
	 *
	 */

	double a = 1/3.0;
	printf("%f\n", a);
	int sockfd, nfds;
	io_mutex *io = new io_mutex;

	sockfd = io->socket_init(SERVER_IP, SERVER_PORT);

	struct epoll_event events[MAX_EPOLL_EVENTS];

	epollfd = epoll_create1(0);
	ep_event.data.fd = sockfd;
	ep_event.events = EPOLLIN;
	io->set_nonblock(sockfd);

	ep_event.events = EPOLLIN | EPOLLET;
	epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &ep_event);

	/*
	 *
	 *	开启工作线程，优化如何进行多线程高效率的同步？
	 */
	pthread_t pth[10];
	for ( int i = 0; i < 10; i++)
		pthread_create(&pth[i], NULL, worker, io);
	/*
	 * 
	 *创建一个线程来处理新fd连接超时时间
	 */
	pthread_t pth_pend;
	int ret = pthread_create(&pth_pend, NULL, check_pend_timeout, io);



	/*
	 *	主线程主要监听socket,不做业务处理
	 *	具体工作由工作线程处理，提高程序处理的效率
     *
     *
	 */
	while(true){
		nfds = epoll_wait(epollfd, events, MAX_EPOLL_EVENTS, -1);
		if (nfds < 0) {
			printf("epoll_wait error = %s\n", strerror(errno));
			break;
		}
		for (int i = 0; i < nfds; ++i){
			if (events[i].data.fd == sockfd) {
				struct sockaddr_in client_addr;
				socklen_t client_addrlen;
				int connfd;
				memset(&client_addr, 0, sizeof(sockaddr_in));
				client_addrlen = sizeof(client_addr);
				connfd = accept(sockfd, (sockaddr *)&client_addr, &client_addrlen);
				if (connfd < 0){
					continue;
				}
				io->set_nonblock(connfd);
				ep_event.data.fd = connfd;
				/*
				 *	在ET模式下，一个socket上的某个事件还是可能被触发多次，多线程同事操作同一socket
				 *	
				 *	确保socket连接在任一时刻都只被一个线程处理,可以使用EPOLLONESHOT
				 *
				 *
				 */
				ep_event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
				if (epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &ep_event) < 0){
					close(connfd);
					continue;
				}
				io->lock(io->m_pending);
				io->m_pendingfd.insert(pair<int, time_t>(connfd, time(NULL)));//超时检查连接映射表
				io->unlock(io->m_pending);

			} else {
				int tmp_sockfd;
				tmp_sockfd = events[i].data.fd;
				if (tmp_sockfd < 0){
					continue;
				}

				/*
				*更新最后一次对应客户端访问时间
				*/
				time_t tNow;
				time(&tNow);
				io->lock(io->m_pending);
				map<int, time_t>::iterator pos = io->m_pendingfd.find(tmp_sockfd);
				if (pos != io->m_pendingfd.end()) {
					pos->second = tNow;
				}
				io->unlock(io->m_pending);

				io->lock(io->m_mutex);
				io->m_queue.push(tmp_sockfd);
				printf("m_queue.size() = %d\n", io->m_queue.size());
				pthread_cond_broadcast(&io->m_cond);
				io->unlock(io->m_mutex);

			}
		}
		/*
		 *	防止高并发下，只会有一个fd被处理，此后一直阻塞在epoll_wait(),导致别的连接无法accept
		 *	还有另一种解决方法，使用while一直accpet直到返回-1，EAGAIN时推出循环
		 */
		ep_event.data.fd = sockfd;
		ep_event.events = EPOLLIN;
		epoll_ctl(epollfd, EPOLL_CTL_MOD, sockfd, &ep_event);

	}
	return 0;
}