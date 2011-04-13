#ifndef IPCSOCKET_H
#define IPCSOCKET_H

#include <string>
class sg_node;

class ipcsocket {
public:
	ipcsocket(std::string socketfile, bool recvfirst);
	~ipcsocket();
	
	bool send(const std::string &s);
	bool receive(std::string &msg);
	
private:
	bool accept();
	void disconnect();
	
	std::string recvbuf;
	int listenfd, fd;
	bool connected;
	
	/* Is the incoming connection expecting to perform a send (hence
	   you're receiving) as its first action? This needs to be specified
	   to prevent deadlock.
	*/
	bool recvfirst;
};

#endif
