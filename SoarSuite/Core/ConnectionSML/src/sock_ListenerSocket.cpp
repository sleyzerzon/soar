#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
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
#include "sock_Debug.h"
#include "sock_OSspecific.h"

#include <sstream>

using namespace sock ;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ListenerSocket::ListenerSocket()
{

}

ListenerSocket::~ListenerSocket()
{

}


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
	if (m_hSocket)
	{
		PrintDebug("Error: Already listening--closing the existing listener") ;

		NET_CLOSESOCKET(m_hSocket) ;
		m_hSocket = NO_CONNECTION ;
	}

	// Create the listener socket

	SOCKET hListener;

#ifdef ENABLE_LOCAL_SOCKETS
	if(local) {
		hListener = socket(AF_UNIX, SOCK_STREAM, 0);
	}
	else 
#endif
	{
		hListener = socket(AF_INET, SOCK_STREAM, 0) ;
	}

	if (hListener == INVALID_SOCKET)
	{
		PrintDebug("Error: Error creating the listener socket") ;
		return false ;
	}

	// Record the listener socket so we'll clean
	// up properly when the object is destroyed.
	m_hSocket = hListener ;

	// Reuse the address for this socket
	int reuse_addr = 1 ;
	setsockopt(hListener, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse_addr, sizeof(reuse_addr)) ;

	// Specify the port we are listening on
	sockaddr_in address ;

	int res;

#ifdef ENABLE_LOCAL_SOCKETS

	sockaddr_un local_address;

	if(local) {
		memset(&local_address, 0, sizeof(local_address));

		local_address.sun_family = AF_UNIX;
		sprintf(local_address.sun_path, "%s%u", sock::GetLocalSocketDir().c_str(), port); // buffer is 108 chars long, so this is safe

		// set the name of the datasender
		this->name = "file ";
		this->name.append(local_address.sun_path);

		// BADBAD: should check to see if it's in use?
		unlink(local_address.sun_path); // in case it already exists

		int len = SUN_LEN(&local_address);

		// create directory where socket file will live (if the directory already exists, this will fail, but we don't care)
		res = mkdir(sock::GetLocalSocketDir().c_str(), 0700);

		res = bind(hListener, (sockaddr*)&local_address, len) ;

		if (res != 0)
		{
			PrintDebug("Error: Error binding the local listener socket to its file") ;
			return false ;
		}

	} else 
#endif
	{
		// set the name of the datasender
		std::stringstream sname;
		sname << "port " << port;
		this->name = sname.str();

		memset(&address, 0, sizeof(address)) ;

		address.sin_family = AF_INET ;	// Indicates the type of data in this structure
		address.sin_port   = htons(port) ;
		address.sin_addr.s_addr = htonl(INADDR_ANY) ;

		// Bind the socket to the local port we're listening on
		res = bind(hListener, (sockaddr*)&address, sizeof(address)) ;

		if (res != 0)
		{
			PrintDebug("Error: Error binding the listener socket to its port number") ;
			return false ;
		}
	}

#ifdef NON_BLOCKING
	bool ok = MakeSocketNonBlocking(hListener) ;
	if (!ok)
	{
		PrintDebug("Error: Error setting the listener socket to be non-blocking") ;
		return false ;
	}
#endif

	const int kMaxBacklog = 10 ;

	// Listen for connections on this socket
	res = listen(hListener, kMaxBacklog) ;

	if (res != 0)
	{
		PrintDebug("Error: Error listening on the listener socket") ;
		return false ;
	}

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

	// If this is a blocking socket make sure
	// a connection is available before attempting to accept.
#ifndef NON_BLOCKING
	if (!IsReadDataAvailable())
		return NULL ;
#endif

	//PrintDebug("About to check for a connection") ;
	SOCKET connectedSocket = accept(m_hSocket, NULL, NULL) ;

	// If we failed to find a valid socket we're done.
	if (connectedSocket == INVALID_SOCKET)
		return NULL ;

	PrintDebug("Received a connection") ;

	// Create a generic CTSocket because once the connection has been
	// made all sockets are both servers and clients.  No need to distinguish.
	Socket* pConnection = new Socket(connectedSocket) ;

	pConnection->name = this->name;

	return pConnection ;
}
