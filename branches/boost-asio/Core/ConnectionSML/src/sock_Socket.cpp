#include <portability.h>

/////////////////////////////////////////////////////////////////
// Socket class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : ~2001
//
// Represents a socket.
//
// Instances of this class are not created directly.
//
// A server creates a listener socket (a derived class)
// which is used to listen for incoming connections on a particular port.
//
// A client then connects to that listener socket (it needs to know the IP address
// and port to connect to) through the "client socket" class.
//
// The client continues to use the client socket object it created.
// The server is passed a new socket when it checks for incoming connections
// on the listener socket.
// 
/////////////////////////////////////////////////////////////////

#include <stdio.h>
#include "sml_Utils.h"
#include "sock_Socket.h"

#include <cstdlib>
#include <assert.h>

#ifdef NON_BLOCKING
#include "sml_Utils.h"	// For sml::Sleep
#endif

using namespace sock ;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Socket::Socket()
{
	m_pSocket = 0;
	m_bTraceCommunications = false ;
}

Socket::Socket(boost::asio::ip::tcp::socket* socket)
{
	m_pSocket = socket ;
	m_bTraceCommunications = false ;
}

Socket::~Socket()
{
	Close();
	delete m_pSocket;
}

const char* kLocalHost = "127.0.0.1" ; // Special IP address meaning "this machine"

/////////////////////////////////////////////////////////////////////
// Function name  : Socket::ConnectToServer
// 
// Return type    : bool 	
// Argument       : char* pNetAddress	// Can be NULL -- in which case connect to "this machine"
// Argument       : int port	
// 
// Description	  : Connect to a server
//
/////////////////////////////////////////////////////////////////////
bool Socket::ConnectToServer(char const* pNetAddress, unsigned short port)
{
	using namespace boost::asio;

	CTDEBUG_ENTER_METHOD("ClientSocket::ConnectToServer");

	std::string address;
	ip::address a;
	if(pNetAddress) a = ip::address::from_string(pNetAddress);
	else a = ip::address::from_string(kLocalHost);
	//m_pSocket = new ip::tcp::socket(ioservice);
	m_pSocket->connect( ip::tcp::endpoint(a, port) );

	return true ;
}