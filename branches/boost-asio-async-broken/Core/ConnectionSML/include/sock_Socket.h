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

#ifndef CT_SOCKET_H
#define CT_SOCKET_H

#include "sock_DataSender.h"

#include <string>

#include <boost/asio.hpp>

namespace sml {
	class Connection;
}

namespace sock {

class Socket : public DataSender
{
protected:
	sml::Connection* m_pConnection;
	boost::asio::ip::tcp::socket* m_pSocket;

	enum { HEADER_LENGTH = 4 };

	std::string m_RecvBuffer;

	// These objects are created through the ListenerSocket or ClientSocket classes.
public:
	Socket( sml::Connection* pConnection ) 
		: m_pConnection( pConnection )
		, m_pSocket( 0 ) 
	{
		RegisterHandlers();
	}

	Socket( sml::Connection* pConnection, boost::asio::ip::tcp::socket* socket ) 
		: m_pConnection( pConnection )
		, m_pSocket( socket ) 
	{
		RegisterHandlers();
	}

	Socket( sml::Connection* pConnection, boost::asio::ip::tcp::socket* socket, const std::string& name ) 
		: DataSender( name )
		, m_pConnection( pConnection )
		, m_pSocket( socket )
	{
		RegisterHandlers();
	}

public:
	// Destructor closes the socket
	virtual			~Socket() { Close();	delete m_pSocket; }

	// Note: When we try to read/write to the socket next we may
	// find this is no longer true--it's just the last state we know about.
	virtual bool	IsAlive()					{ return m_pSocket->is_open() ; }

	// Check if data is waiting to be read
	// Returns true if socket is closed--but then receiveMsg will know it's closed.
	// The timeout for waiting for data is secondsWait + millisecondsWait, where millisecondsWait < 1000
	virtual bool	IsReadDataAvailable() {	return m_pSocket->available() > 0; }

public:

	/////////////////////////////////////////////////////////////////////
	// Function name  : CTClientSocket::ConnectToServer
	// 
	// Return type    : bool 	
	// Argument       : char* pNetAddress	// Can be NULL -- in which case connect to "this machine"
	// Argument       : int port	
	// 
	// Description	  : Connect to a server
	//
	/////////////////////////////////////////////////////////////////////
	bool	ConnectToServer(char const* netAddress, unsigned short port) ;

protected:

	void RegisterHandlers();

	// Lower level buffer send and receive calls.
	virtual bool	SendBuffer(char const* pSendBuffer, size_t bufferSize) ;
	virtual bool	ReceiveBuffer(char* pRecvBuffer, size_t bufferSize) ;

	bool	ReceiveHeaderHandler( const boost::system::error_code& error, std::size_t bytes_transferred );
	bool	ReceiveBodyHandler( const boost::system::error_code& error, std::size_t bytes_transferred );

	// Close down our side of the socket
	virtual void	CloseInternal() { m_pSocket->close(); }

};

} // Namespace

#endif // CT_SOCKET_H
