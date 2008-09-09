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

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

namespace sock {

// Useful utility functions.
unsigned long GetLocalIP() ;
char*		  GetLocalIPAddress() ;

class ListenerSocket ;
class ClientSocket ;

class Socket : public DataSender
{
	// Allow these classes access to our constructor
	friend class ListenerSocket ;
	friend class ClientSocket ;

protected:
	boost::asio::ip::tcp::socket* m_pSocket;

	// Controls whether we dump out the messages we're sending and receiving.
	bool m_bTraceCommunications ;

	// These objects are created through the ListenerSocket or ClientSocket classes.
public:
	Socket();
	Socket(boost::asio::ip::tcp::socket* socket);

public:
	// Destructor closes the socket
	virtual		~Socket();

	// Note: When we try to read/write to the socket next we may
	// find this is no longer true--it's just the last state we know about.
	bool		IsAlive()					{ return m_pSocket->is_open() ; }

	// Returns handle for socket
	//SOCKET		GetSocketHandle()			{ return m_hSocket ; }

	// Check if data is waiting to be read
	// Returns true if socket is closed--but then receiveMsg will know it's closed.
	// The timeout for waiting for data is secondsWait + millisecondsWait, where millisecondsWait < 1000
	bool		IsReadDataAvailable(long secondsWait = 0, long millisecondsWait = 0) 
	{
		return m_pSocket->available() > 0;
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

	// Print out debug information about the messages we are sending and receiving.
	// NOTE: We still print out information about start up/shut down, errors etc. without this flag being true.
	void		SetTraceCommunications(bool state) { m_bTraceCommunications = state ; }

	// Send a string of characters.  Outgoing format will be "<4-byte length>"+string data
	bool		SendString(char const* pString) ;

	// Receive a string of characters.  Incoming format on socket should be "<4-byte length>"+string data
	bool		ReceiveString(std::string* pString) ;

protected:
	// Lower level buffer send and receive calls.
	bool		SendBuffer(char const* pSendBuffer, size_t bufferSize)
	{
		try
		{
			boost::asio::write(*m_pSocket, boost::asio::buffer(pSendBuffer, bufferSize)); 
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			return false;
		}
		return true;
	}
	bool		ReceiveBuffer(char* pRecvBuffer, size_t bufferSize) 
	{
		try
		{
			boost::asio::read(*m_pSocket, boost::asio::buffer(pRecvBuffer, bufferSize));
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			return false;
		}
		return true;
	}

	// Close down our side of the socket
	virtual void		CloseInternal() { m_pSocket->close(); }

};

} // Namespace

#endif // CT_SOCKET_H
