#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "ipcsocket.h"
#include "sg_node.h"
#include "linalg.h"

using namespace std;

const char TERMSTRING[] = "\n***\n";
const int BUFFERSIZE = 10240;

ipcsocket::ipcsocket(string socketfile) 
: recvbuf()
{
	int listenfd;
	socklen_t len;
	struct sockaddr_un addr, remote;
	
	if ((listenfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("ipcsocket::ipcsocket");
		exit(1);
	}
	
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, socketfile.c_str());
	unlink(addr.sun_path);
	len = strlen(addr.sun_path) + sizeof(addr.sun_family);
	if (bind(listenfd, (struct sockaddr *) &addr, len) == -1) {
		perror("ipcsocket::ipcsocket");
		exit(1);
	}

	if (listen(listenfd, 1) == -1) {
		perror("ipcsocket::ipcsocket");
		exit(1);
	}
	
	len = sizeof(struct sockaddr_un);
	if ((fd = accept(listenfd, (struct sockaddr *) &remote, &len)) == -1) {
		perror("ipcsocket::ipcsocket");
		exit(1);
	}
	close(listenfd);
}

ipcsocket::~ipcsocket() {
	close(fd);
}

void ipcsocket::sendall(const string &s) {
	int n, t = 0;
	while (t < s.size()) {
		if ((n = ::send(fd, s.c_str() + t, s.size() - t, 0)) < 0) {
			perror("ipcsocket::sendall");
			exit(1);
		}
		t += n;
	}
}

void ipcsocket::send(const string &header, int level, const string &msg) {
	stringstream ss;
	ss << header << ' ' << level << '\n' << msg << TERMSTRING;
	sendall(ss.str());
}

void ipcsocket::send(const string &header, const string &msg) {
	sendall(header + '\n' + msg + TERMSTRING);
}

void ipcsocket::receive(string &header, string &msg) {
	char buf[BUFFERSIZE+1];
	size_t p1, p2, n;
	
	while((p2 = recvbuf.find(TERMSTRING)) == string::npos) {
		if ((n = recv(fd, buf, BUFFERSIZE, 0)) == -1) {
			perror("ipcsocket::receive");
			exit(1);
		}
		buf[n] = '\0';
		recvbuf += buf;
	}
	
	p1 = recvbuf.find('\n');
	header.assign(recvbuf.substr(0, p1));
	msg.assign(recvbuf.substr(p1+1, p2-p1-1));
	recvbuf.erase(0, p2+strlen(TERMSTRING));
}

string ipcsocket::communicate(const string &header, int level, const string &msg, string &response) {
	string responseheader;
	send(header, level, msg);
	receive(responseheader, response);
	return responseheader;
}
