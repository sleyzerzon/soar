#include <portability.h>

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

#include "sml_Utils.h"
#include "sml_ConnectionManager.h"
#include "sml_ListenerThreads.h"
#include "sml_KernelSML.h"
#include "sock_ClientNamedPipe.h"

#include <time.h>	// To get clock
#include <stdio.h> // To concat file name for socket file deletion

using namespace sml ;
using namespace sock ;

// From KernelSMLInterface (which handles embedded connections)
extern ElementXML* ReceivedCall(Connection* pConnection, ElementXML* pIncoming, void* pUserData) ;

ListenerThreads::ListenerThreads(ConnectionManager* parent, unsigned short port) 
: m_Parent(parent), m_Port(port)
{
	m_Main = 0;
#ifdef ENABLE_LOCAL_SOCKETS
	m_Local = 0;
#endif
#ifdef ENABLE_NAMED_PIPES
	m_Pipe = 0;
#endif
}

void ListenerThreads::Listen()
{
	assert(m_Main == 0);
	m_Main = new ListenerThreadMain(this);
	m_Main->Start();

#ifdef ENABLE_LOCAL_SOCKETS
	assert(m_Local == 0);
	m_Local = new ListenerThreadLocal(this);
	m_Local->Start();
#endif

#ifdef ENABLE_NAMED_PIPES
	assert(m_Pipe == 0);
	m_Pipe = new ListenerThreadPipe(this);
	m_Pipe->Start();
#endif
}

void ListenerThreads::Shutdown()
{
	if (m_Main)
		m_Main->Stop(false);
	m_Main->Close();
#ifdef ENABLE_LOCAL_SOCKETS
	if (m_Local)
		m_Local->Stop(false);
	m_Local->Close();
#endif
#ifdef ENABLE_NAMED_PIPES
	if (m_Pipe)
		m_Pipe->Stop(false);
	m_Pipe->Close();
#endif


	if (m_Main)
		m_Main->Stop(true);
	m_Main = 0;
#ifdef ENABLE_LOCAL_SOCKETS
	if (m_Local)
		m_Local->Stop(true);
	m_Local = 0;
#endif
#ifdef ENABLE_NAMED_PIPES
	if (m_Pipe)
		m_Pipe->Stop(true);
	m_Pipe = 0;
#endif
}

void ListenerThreads::CreateConnection(DataSender* pSender)
{
	sml::PrintDebugFormat("Got new connection on %s", pSender->GetName().c_str()) ;

	// Create a new connection object for this socket
	Connection* pConnection = Connection::CreateRemoteConnection(pSender) ;

	// Record our kernel object with this connection.  I think we only want one kernel
	// object even if there are many connections (because there's only one kernel) so for now
	// that's how things are set up.
	pConnection->SetUserData(KernelSML::GetKernelSML()) ;

	// For debugging record that this is on the kernel side
	pConnection->SetIsKernelSide(true) ;

	// Register for "calls" from the client.
	pConnection->RegisterCallback(ReceivedCall, NULL, sml_Names::kDocType_Call, true) ;

	// Set up our tracing state to match the user's preference
	pConnection->SetTraceCommunications(KernelSML::GetKernelSML()->IsTracingCommunications()) ;

	// Record the new connection in our list of connections
	KernelSML::GetKernelSML()->AddConnection(pConnection) ;
}

void ListenerThreadMain::Run()
{
	// Create the listener
	sml::PrintDebugFormat("Listening on port %d", m_Control->GetPort()) ;
	bool ok = m_ListenerSocket.CreateListener(m_Control->GetPort()) ;
	
	if (!ok)
	{
		sml::PrintDebug("Failed to create the listener socket.  Shutting down thread.") ;
		return ;
	}

	while (!m_QuitNow)
	{
		Socket* pSocket = m_ListenerSocket.CheckForClientConnectionBlocking() ;
		if (pSocket) m_Control->CreateConnection(pSocket);
	}

	// Shut down our listener socket
	m_ListenerSocket.Close() ;
}
void ListenerThreadMain::Close()
{
	m_ListenerSocket.Close();
}

#ifdef ENABLE_LOCAL_SOCKETS
void ListenerThreadLocal::Run()
{
	// Create the listener
	sml::PrintDebugFormat("Listening on file %s%d", sock::GetLocalSocketDir().c_str(), m_Control->GetPort()) ;

	bool ok = m_LocalListenerSocket.CreateListener(m_Control->GetPort(), true);

	if (!ok)
	{
		sml::PrintDebug("Failed to create the local listener socket.  Shutting down thread.") ;
		return ;
	}

	while (!m_QuitNow)
	{
		Socket* pLocalSocket = m_LocalListenerSocket.CheckForClientConnectionBlocking();
		if (pLocalSocket) m_Control->CreateConnection(pLocalSocket);
	}

	m_LocalListenerSocket.Close();

// This code shouldn't be necessary since the we unlink existing files on start
// and since the file is in the user's home, permissions for that won't be an issue
/*
	char f_name[256];
	sprintf( f_name, "%s%d", sock::GetLocalSocketDir().c_str(), m_Control->GetPort() );
	sml::PrintDebugFormat( "Attempting to delete %s", f_name );
	int del_status = unlink( f_name );
	if ( del_status == 0 )
		sml::PrintDebug( "Delete succeeded!" );
	else
		sml::PrintDebugFormat( "Error occurred during delete attempt, error code %d", errno );
*/
}
void ListenerThreadLocal::Close()
{
	m_ListenerSocket.Close();
}

#endif

#ifdef ENABLE_NAMED_PIPES
void ListenerThreadPipe::Run()
{
	unsigned long usernamesize = UNLEN+1;
	char username[UNLEN+1];
	GetUserName(username,&usernamesize);

	std::stringstream pipeName;
	pipeName << "\\\\.\\pipe\\" << username << "-" << m_Control->GetPort();

	// Create the listener
	sml::PrintDebugFormat("Listening on pipe %s", pipeName.str().c_str()) ;

	bool ok = m_ListenerNamedPipe.CreateListener(pipeName.str().c_str()) ;

	if (!ok)
	{
		sml::PrintDebug("Failed to create the listener pipe.  Shutting down thread.") ;
		return ;
	}

	while (!m_QuitNow)
	{
		NamedPipe* pNamedPipe = m_ListenerNamedPipe.CheckForClientConnectionBlocking();
		if (m_QuitNow) break;

		if (pNamedPipe) {
			m_Control->CreateConnection(pNamedPipe);
			ok = m_ListenerNamedPipe.CreateListener(pipeName.str().c_str()) ;

			if (!ok)
			{
				sml::PrintDebug("Failed to create the listener pipe.  Shutting down thread.") ;
				return ;
			}
		}
	}

	m_ListenerNamedPipe.Close();
}
void ListenerThreadPipe::Close()
{
	sock::ClientNamedPipe* pNamedPipe = new sock::ClientNamedPipe() ;

	std::stringstream name;
	name << m_Control->GetPort();
	
	pNamedPipe->ConnectToServer(name.str().c_str()) ;
	pNamedPipe->Close();
}

#endif

