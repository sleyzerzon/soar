#ifndef IPCSOCKET_H
#define IPCSOCKET_H

#include <string>
class sg_node;

class ipcsocket {
public:
	ipcsocket(std::string socketfile);
	~ipcsocket();
	
	bool        send(const std::string &header, const std::string &msg);
	bool        send(const std::string &type, int level, const std::string &msg);
	bool        receive(std::string &type, std::string &msg);
	std::string communicate(const std::string &header, int level, const std::string &msg, std::string &resp);
	
private:
	bool accept();
	void disconnect();
	bool sendall(const std::string &s);
	
	std::string recvbuf;
	int listenfd, fd;
	bool connected;
};

#endif
