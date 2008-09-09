/////////////////////////////////////////////////////////////////
// ListenerThread class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : October 2004
//
// Listens for incoming remote connections and creates
// a new connection object for each one.
//
/////////////////////////////////////////////////////////////////

#ifndef LISTENER_THREAD_H
#define LISTENER_THREAD_H

#include "thread_Thread.h"
#include "sock_DataSender.h"
#include "sock_ListenerSocket.h"
#include "sock_SocketLib.h"
#include "sml_Connection.h"

#include <list>

#include <boost/bind.hpp>
#include <boost/asio.hpp>

using namespace boost::asio::ip;

namespace sml {

// Forward declarations
class ConnectionManager ;

// A listener socket wrapped in a thread
class ListenerThread : public soar_thread::Thread
{
protected:
	unsigned short				m_Port ;
	ConnectionManager*			m_Parent ;
	tcp::acceptor m_acceptor;

	void Run() ;

	void CreateConnection(sock::DataSender* pSender);

public:
	ListenerThread(ConnectionManager* parent, unsigned short port) : m_acceptor(Connection::s_IOService, tcp::endpoint(tcp::v4(), port)) 
	{ m_Parent = parent ; m_Port = port ; }

	void StartAccept()
	{
		// Create the listener
		//sml::PrintDebugFormat("Listening on port %d", m_Port) ;
		boost::asio::ip::tcp::socket* connectedSocket = new tcp::socket(m_acceptor.get_io_service());
		sock::Socket* pConnection = new sock::Socket(connectedSocket) ;
		m_acceptor.async_accept(*connectedSocket, boost::bind(&ListenerThread::CreateConnection, this, pConnection));
	}
} ;


} // Namespace

#endif	// LISTENER_THREAD_H
