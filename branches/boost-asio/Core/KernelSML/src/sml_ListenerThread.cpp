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

#include "sml_ListenerThread.h"

#include "sml_Utils.h"
#include "sml_ConnectionManager.h"
#include "sml_KernelSML.h"

#include <time.h>	// To get clock
#include <stdio.h> // To concat file name for socket file deletion

using namespace sml ;
using namespace sock ;

// From KernelSMLInterface (which handles embedded connections)
extern soarxml::ElementXML* ReceivedCall(Connection* pConnection, soarxml::ElementXML* pIncoming, void* pUserData) ;

void ListenerThread::Run()
{
	StartAccept();

	while (!m_QuitNow)
	{
		//sml::PrintDebug("Check for incoming connection") ;
		Connection::s_IOService.poll();

		// Sleep for a little before checking for a new connection
		// New connections will come in very infrequently so this doesn't
		// have to be very rapid.
		sml::Sleep(0, 50) ;
	}
}

void ListenerThread::CreateConnection(DataSender* pSender)
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

	StartAccept();
}
