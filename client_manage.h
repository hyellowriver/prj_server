#ifndef CLIENT_MANAGE_H
#define CLIENT_MANAGE_H
class client_manage
{
public:
	client_manage(){}
	~client_manage();
public:
	int deal_request(int fd, char *buffer, int size);
	
};

#endif
