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

#include <boost/lexical_cast.hpp>
#include <boost/asio.hpp>

using namespace boost::asio::ip;

namespace sml {

// A listener socket wrapped in a thread
class ListenerThread : public soar_thread::Thread
{
protected:
	// boost::asio support
	tcp::acceptor m_acceptor;
	std::string m_port;

	bool m_stoppingAcceptor;

	void Run() ;

	void CreateConnection(sock::DataSender* pSender, const boost::system::error_code& error);
	void HandleStopAccept();
	void HandleStopIOService();

public:
	ListenerThread( boost::asio::io_service& ioservice, unsigned short port) 
		: m_acceptor( ioservice, tcp::endpoint( tcp::v4(), port ))
		, m_stoppingAcceptor(false)
	{
		m_acceptor.set_option(tcp::acceptor::reuse_address(true));
		m_port = boost::lexical_cast<std::string>(port);
	}

	~ListenerThread()
	{
		//m_acceptor.get_io_service().stop();
	}

	void StartAccept();
	void StopAccept(bool wait) ;
	void RequestStopIOService() ; // don't need a wait for this; just call thread::stop(true) on this thread
} ;


} // Namespace

#endif	// LISTENER_THREAD_H
