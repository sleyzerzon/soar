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
#include "sock_Socket.h"
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
	// boost::asio support
	tcp::acceptor m_acceptor;

	void Run() ;

	void CreateConnection(sock::DataSender* pSender);

public:
	ListenerThread( boost::asio::io_service& ioservice, unsigned short port) 
		: m_acceptor( ioservice, tcp::endpoint( tcp::v4(), port ) ) 
	{}

	~ListenerThread()
	{
		m_acceptor.get_io_service().stop();
	}

	void StartAccept();
	void StopAccept() ;
} ;


} // Namespace

#endif	// LISTENER_THREAD_H
