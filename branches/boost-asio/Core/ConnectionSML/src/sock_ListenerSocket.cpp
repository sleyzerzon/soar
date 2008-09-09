#include <portability.h>

/////////////////////////////////////////////////////////////////
// ListenerSocket class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : ~2001
//
// A server application creates a listener socket on a specific port.
// Clients then connect through this port to create a socket which is
// actually used to send data.
// 
/////////////////////////////////////////////////////////////////
#include "sock_ListenerSocket.h"
#include "sml_Utils.h"
#include "sock_OSspecific.h"

#include <sstream>

using namespace sock ;

/////////////////////////////////////////////////////////////////////
// Function name  : ListenerSocket::CreateListener
// 
// Return type    : bool 	
// Argument       : int port	
// 
// Description	  : Create a non-blocking socket that listens
//					on a specific port.
//
/////////////////////////////////////////////////////////////////////
bool ListenerSocket::CreateListener(unsigned short port, bool local)
{
	CTDEBUG_ENTER_METHOD("ListenerSocket::CreateListener");

	// Should only call this once
	if (m_acceptor.is_open())
	{
		sml::PrintDebug("Error: Already listening--closing the existing listener") ;
		m_acceptor.close();
	}

	// set the name of the datasender
	std::stringstream sname;
	sname << "port " << port;
	this->name = sname.str();

	boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);
	m_acceptor.open(endpoint.protocol());
	m_acceptor.set_option(boost::asio::socket_base::reuse_address(true));
	m_acceptor.bind(endpoint);

	const int kMaxBacklog = 10 ;

	m_acceptor.listen(kMaxBacklog);
	
	return true ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : ListenerSocket::CheckForClientConnection
// 
// Return type    : CTSocket* 	
// 
// Description	  : This function creates a new
//					socket (wrapped by CTSocket) if there is a connection
//					by a client to the listener socket.
//
//					NULL is returned if there is no new connection.
//
/////////////////////////////////////////////////////////////////////
Socket* ListenerSocket::CheckForClientConnection()
{
	CTDEBUG_ENTER_METHOD("ListenerSocket::CheckForClientConnection");

	//sml::PrintDebug("About to check for a connection") ;
	boost::asio::ip::tcp::socket* connectedSocket = new boost::asio::ip::tcp::socket(m_acceptor.get_io_service());
	m_acceptor.accept(*connectedSocket);

	// If we failed to find a valid socket we're done.
	if (!connectedSocket->is_open())
		return NULL ;

	//sml::PrintDebug("Received a connection") ;

	// Create a generic CTSocket because once the connection has been
	// made all sockets are both servers and clients.  No need to distinguish.
	Socket* pConnection = new Socket(connectedSocket) ;

	pConnection->name = this->name;

	return pConnection ;
}
