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

#ifdef ENABLE_NAMED_PIPES
#include "sock_ListenerNamedPipe.h"
#endif

#include <list>

namespace sml {

// Forward declarations
class ConnectionManager ;
class ListenerThreads;

class ListenerThreadMain : public soar_thread::Thread
{
protected:
	ListenerThreads*			m_Control;
	sock::ListenerSocket		m_ListenerSocket ;

	void Run() ;

public:
	ListenerThreadMain(ListenerThreads* control) : m_Control(control) { }
	void Close();
} ;

#ifdef ENABLE_LOCAL_SOCKETS
class ListenerThreadLocal : public soar_thread::Thread
{
protected:
	ListenerThreads*			m_Control;
	sock::ListenerSocket		m_LocalListenerSocket;

	void Run() ;

public:
	ListenerThreadLocal(ListenerThreads* control) : m_Control(control) { }
	void Close();
} ;
#endif

#ifdef ENABLE_NAMED_PIPES
class ListenerThreadPipe : public soar_thread::Thread
{
protected:
	ListenerThreads*			m_Control;
	sock::ListenerNamedPipe		m_ListenerNamedPipe ;

	void Run() ;

public:
	ListenerThreadPipe(ListenerThreads* control) : m_Control(control) { }
	void Close();
} ;
#endif

class ListenerThreads
{
protected:
	unsigned short				m_Port ;
	ConnectionManager*			m_Parent ;

	ListenerThreadMain*			m_Main;
#ifdef ENABLE_LOCAL_SOCKETS
	ListenerThreadLocal*		m_Local;
#endif
#ifdef ENABLE_NAMED_PIPES
	ListenerThreadPipe*			m_Pipe;
#endif

public:
	ListenerThreads(ConnectionManager* parent, unsigned short port);

	void Listen();
	unsigned short GetPort() { return m_Port; }
	void CreateConnection(sock::DataSender* pSender);
	void Shutdown();
} ;

} // Namespace

#endif	// LISTENER_THREAD_H
