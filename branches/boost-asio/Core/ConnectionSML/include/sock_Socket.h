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


namespace sock {

class Socket : public DataSender
{
protected:
	boost::asio::ip::tcp::socket* m_pSocket;

	// These objects are created through the ListenerSocket or ClientSocket classes.
public:
	Socket() 
		: m_pSocket( 0 ) 
	{}

	Socket( boost::asio::ip::tcp::socket* socket ) 
		: m_pSocket( socket ) 
	{}

	Socket( boost::asio::ip::tcp::socket* socket, const std::string& name ) 
		: DataSender( name )
		, m_pSocket( socket )
	{}

public:
	// Destructor closes the socket
	virtual			~Socket() { Close();	delete m_pSocket; }

	// Note: When we try to read/write to the socket next we may
	// find this is no longer true--it's just the last state we know about.
	virtual bool	IsAlive()					
	{ 
		try
		{
			//m_pSocket->remote_endpoint();
			m_pSocket->available();
		}
		catch (std::exception& e)
		{
			return false;
		}
		return true;
	}

	// Check if data is waiting to be read
	// Returns true if socket is closed--but then receiveMsg will know it's closed.
	// The timeout for waiting for data is secondsWait + millisecondsWait, where millisecondsWait < 1000
	virtual bool	IsReadDataAvailable() {	return m_pSocket->available() > 0; }

	// Close down our side of the socket
	virtual void	Close()
	{ 
		if(!m_pSocket->is_open()) return;
		try
		{
			m_pSocket->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
		} catch (std::exception& e)
		{
			std::cerr << "\nSocket::CloseInternal: is_open = " << m_pSocket->is_open() << std::endl;
			std::cerr << "\nSocket::CloseInternal: " << e.what() << std::endl;
		}
		m_pSocket->close(); 
	}

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

	// Lower level buffer send and receive calls.
	virtual bool	SendBuffer(char const* pSendBuffer, size_t bufferSize) ;
	virtual bool	ReceiveBuffer(char* pRecvBuffer, size_t bufferSize) ;

};

} // Namespace

#endif // CT_SOCKET_H