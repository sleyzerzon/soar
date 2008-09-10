#ifndef IOSERVICE_THREAD_H
#define IOSERVICE_THREAD_H

#include "thread_Thread.h"
#include "sock_DataSender.h"
#include "sock_Socket.h"
#include "sml_Connection.h"

#include <list>

#include <boost/bind.hpp>
#include <boost/asio.hpp>

using namespace boost::asio::ip;

namespace sml {

// A listener socket wrapped in a thread
class IOServiceThread : public soar_thread::Thread
{
protected:
	void Run() 
	{
		m_pIOService->run();
	}

	boost::asio::io_service* m_pIOService;

public:
	IOServiceThread( boost::asio::io_service* ioservice ) 
		: m_pIOService( ioservice )
	{
		assert( m_pIOService );
	}

	~IOServiceThread()
	{
		m_pIOService->stop();
		delete m_pIOService;
		m_pIOService = 0;
	}
} ;


} // Namespace

#endif	// IOSERVICE_THREAD_H
