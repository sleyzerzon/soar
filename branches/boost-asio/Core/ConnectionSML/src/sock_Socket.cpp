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

using namespace sock ;

bool Socket::SendBuffer(char const* pSendBuffer, size_t bufferSize)
{
	try
	{
		boost::asio::write(*m_pSocket, boost::asio::buffer(pSendBuffer, bufferSize)); 
	}
	catch (std::exception& e)
	{
		//std::cerr << "\nSocket::SendBuffer: is_open = " << m_pSocket->is_open() << std::endl;
		//std::cerr << "\nSocket::SendBuffer: " << e.what() << std::endl;
		return false;
	}
	return true;
}
bool Socket::ReceiveBuffer(char* pRecvBuffer, size_t bufferSize) 
{
	try
	{
		boost::asio::read(*m_pSocket, boost::asio::buffer(pRecvBuffer, bufferSize));
	}
	catch (std::exception& e)
	{
		//std::cerr << "\nSocket::ReceiveBuffer: is_open = " << m_pSocket->is_open() << std::endl;
		//std::cerr << "\nSocket::ReceiveBuffer: " << e.what() << std::endl;
		return false;
	}
	return true;
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

	CTDEBUG_ENTER_METHOD("Socket::ConnectToServer");

	if (pNetAddress == NULL) pNetAddress = kLocalHost ;

	// set the name of the datasender
	std::stringstream name;
	name << "port " << port;
	m_name = name.str();

	m_pSocket->connect( ip::tcp::endpoint(ip::address::from_string(pNetAddress), port) );

	return true ;
}