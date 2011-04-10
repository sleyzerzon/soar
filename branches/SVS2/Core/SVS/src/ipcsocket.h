#ifndef IPCSOCKET_H
#define IPCSOCKET_H

#include <string>
class sg_node;

class ipcsocket {
public:
	ipcsocket(std::string socketfile);
	~ipcsocket();
	
	bool send(const std::string &s);
	bool receive(std::string &msg);
	
private:
	bool accept();
	void disconnect();
	
	std::string recvbuf;
	int listenfd, fd;
	bool connected;
};

#endif
