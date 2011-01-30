#ifndef IPCSOCKET_H
#define IPCSOCKET_H

#include <string>
class sg_node;

class ipcsocket {
public:
	ipcsocket(std::string socketfile);
	~ipcsocket();
	
	void        send(const std::string &header, const std::string &msg);
	void        send(const std::string &type, int level, const std::string &msg);
	void        receive(std::string &type, std::string &msg);
	std::string communicate(const std::string &header, int level, const std::string &msg, std::string &resp);
	
private:
	void sendall(const std::string &s);
	
	std::string recvbuf;
	int fd;
};

#endif
