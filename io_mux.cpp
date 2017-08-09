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
void *worker(void *arg)
{
	pthread_detach(pthread_self());
	io_mutex *io_ctr = (io_mutex *)arg;
	while(true){
		io_ctr->lock();
		while(io_ctr->m_queue.empty()){
			pthread_cond_wait(&io_ctr->m_cond, &io_ctr->m_mutex);
		}
		int fd = io_ctr->m_queue.front();
		io_ctr->m_queue.pop();
		int nRet = 0;
		char buffer[1024];
		//while (1){
			memset(buffer, 0, 1024);
			nRet = recv(fd, buffer, sizeof(buffer), 0);
			if (nRet == -1){
				if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR){
					/*
						EAGAIN 和 EWOULDBLOCK：非阻塞状态下，系统提示再试一次，非错误
						EINTR：系统中断
					*/
					break;

				}
				close(fd);//返回了异常错误码，直接关闭
				break;
			} else if (nRet == 0){

				close(fd);	//客户端关闭
				break;
			} else {
				printf(" get data: %s\n ", buffer);

			}

		//}
		io_ctr->unlock();


	}


}

int main(int argc, char const *argv[])
{
	/*
	 * 信号处理统一函数
	 *
	 *
	 */
	int sockfd, epollfd, nfds;
	io_mutex *io = new io_mutex;

	sockfd = io->socket_init(SERVER_IP, SERVER_PORT);

	struct epoll_event events[MAX_EPOLL_EVENTS];
	struct epoll_event ep_event;
	epollfd = epoll_create1(0);
	ep_event.data.fd = sockfd;
	ep_event.events = EPOLLIN;
	io->set_nonblock(sockfd);
	/*
	 *	epoll默认水平触发，listen的fd不要设置边缘触发
	 */
	ep_event.events = EPOLLIN | EPOLLET;
	epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &ep_event);

	/*
	 *	开启工作线程，优化如何进行多线程高效率的同步？
	 */
	pthread_t pth[5];
	for ( int i = 0; i < 5; i++)
		pthread_create(&pth[i], NULL, worker, io);
	/*
	 * 创建一个线程来处理新fd连接超时时间，一定程度上防止ddos攻击，
	 * 提高程序处理效率
	 * pthread_t pth;
	 * int ret = pthread_create(&pth, NULL, check_timeout, io);
	 *
	 */


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
		printf("---------nfds = %d\n", nfds);
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
				ep_event.events = EPOLLIN | EPOLLET;
				if (epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &ep_event) < 0){
					close(connfd);
					continue;
				}
			} else {
				int tmp_sockfd;
				tmp_sockfd = events[i].data.fd;
				if (tmp_sockfd < 0){
					continue;
				}

				io->lock();
				io->m_queue.push(tmp_sockfd);
				printf("m_queue.size() = %d\n", io->m_queue.size());
				pthread_cond_broadcast(&io->m_cond);
				io->unlock();

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