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
#include "sml_Connection.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace sock ;

void Socket::RegisterHandlers()
{
	m_RecvBuffer.reserve( HEADER_LENGTH );
	boost::asio::async_read( *m_pSocket, boost::asio::buffer( m_RecvBuffer, HEADER_LENGTH ), 
		boost::bind( &Socket::ReceiveHeaderHandler, this, boost::asio::placeholders::error ) );
}

bool Socket::SendBuffer(char const* pSendBuffer, size_t bufferSize)
{
	try
	{
		boost::asio::write(*m_pSocket, boost::asio::buffer(pSendBuffer, bufferSize)); 
	}
	catch (std::exception& e)
	{
		std::cerr << "\nSocket::SendBuffer: is_open = " << m_pSocket->is_open() << std::endl;
		std::cerr << "\nSocket::SendBuffer: " << e.what() << std::endl;
		return false;
	}
	return true;
}

//bool Socket::ReceiveBuffer( char* pRecvBuffer, size_t bufferSize ) 
//{
//	assert( pRecvBuffer );
//
//	m_pRecvBuffer = pRecvBuffer;
//	m_RecvBufferSize = bufferSize;
//
//	try
//	{
//		boost::asio::read(*m_pSocket, boost::asio::buffer(pRecvBuffer, bufferSize));
//	}
//	catch (std::exception& e)
//	{
//		std::cerr << "\nSocket::ReceiveBuffer: is_open = " << m_pSocket->is_open() << std::endl;
//		std::cerr << "\nSocket::ReceiveBuffer: " << e.what() << std::endl;
//		return false;
//	}
//	return true;
//}
//
void Socket::ReceiveHeaderHandler( const boost::system::error_code& error )
{
	if( error ) 
	{
		if( error == boost::asio::error::operation_aborted )
		{
			return;
		}
		assert ( !"Abnormal boost receive error" );
	}

    char header[ HEADER_LENGTH + 1 ] = "";
	std::strncat( header, m_RecvBuffer.data(), HEADER_LENGTH );
	
	// BADBAD: atoi result not error checked
	assert( atoi( header ) ) != 0;
	m_RecvBuffer.resize( atoi( header ) ;)

	boost::asio::async_read( *m_pSocket, boost::asio::buffer( m_RecvBuffer, m_RecvBodyLength ), 
		boost::bind( &Socket::ReceiveBodyHandler, this, boost::asio::placeholders::error ) );
}

void Socket::ReceiveBodyHandler( const boost::system::error_code& error )
{
	// put the message on the queue
	m_pConnection->AddToIncomingMessageQueue( m_pRecvBuffer );

	boost::asio::async_read( *m_pSocket, boost::asio::buffer( m_RecvBuffer, HEADER_LENGTH ), 
		boost::bind( &Socket::ReceiveHeaderHandler, this, boost::asio::placeholders::error ) );
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