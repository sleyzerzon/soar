#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/////////////////////////////////////////////////////////////////
// KernelListener class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : October 2004
//
// This class's HandleEvent method is called when
// specific events occur within the kernel:
/*
*      // System events
*      gSKIEVENT_BEFORE_SHUTDOWN            = 1,
*      gSKIEVENT_AFTER_CONNECTION_LOST,
*      gSKIEVENT_BEFORE_RESTART,
*      gSKIEVENT_AFTER_RESTART,
*      gSKIEVENT_BEFORE_RHS_FUNCTION_ADDED,
*      gSKIEVENT_AFTER_RHS_FUNCTION_ADDED,
*      gSKIEVENT_BEFORE_RHS_FUNCTION_REMOVED,
*      gSKIEVENT_AFTER_RHS_FUNCTION_REMOVED,
*      gSKIEVENT_BEFORE_RHS_FUNCTION_EXECUTED,
*      gSKIEVENT_AFTER_RHS_FUNCTION_EXECUTED,
*/////////////////////////////////////////////////////////////////

#include "sml_KernelListener.h"
#include "sml_Connection.h"
#include "sml_StringOps.h"
#include "IgSKI_Kernel.h"
#include "IgSKI_AgentManager.h"
#include "IgSKI_Agent.h"
#include "sml_KernelSML.h"

using namespace sml ;

// Returns true if this is the first connection listening for this event
bool KernelListener::AddListener(egSKIEventId eventID, Connection* pConnection)
{
	bool first = BaseAddListener(eventID, pConnection) ;

	if (first)
	{
		if (IsSystemEvent(eventID))
			m_pKernelSML->GetKernel()->AddSystemListener(eventID, this) ;
		else if (IsAgentEvent(eventID))
			m_pKernelSML->GetKernel()->GetAgentManager()->AddAgentListener(eventID, this) ;
	}

	return first ;
}

// Returns true if at least one connection remains listening for this event
bool KernelListener::RemoveListener(egSKIEventId eventID, Connection* pConnection)
{
	bool last = BaseRemoveListener(eventID, pConnection) ;

	if (last)
	{
		if (IsSystemEvent(eventID))
			m_pKernelSML->GetKernel()->RemoveSystemListener(eventID, this) ;
		else if (IsAgentEvent(eventID))
			m_pKernelSML->GetKernel()->GetAgentManager()->RemoveAgentListener(eventID, this) ;
	}

	return last ;
}

// Start listening for a specific RHS user function.  When it fires we'll call to the client
// (over the connection) to implement it.
void KernelListener::AddRhsListener(char const* pFunctionName, Connection* pConnection)
{
	RhsMapIter mapIter = m_RhsMap.find(pFunctionName) ;

	ConnectionList* pList = NULL ;

	// Either create a new list or retrieve the existing list of listeners
	if (mapIter == m_RhsMap.end())
	{
		pList = new ConnectionList() ;
		m_RhsMap[pFunctionName] = pList ;
	}
	else
	{
		pList = mapIter->second ;
	}

	pList->push_back(pConnection) ;
}

ConnectionList* KernelListener::GetRhsListeners(char const* pFunctionName)
{
	RhsMapIter mapIter = m_RhsMap.find(pFunctionName) ;

	if (mapIter == m_RhsMap.end())
		return NULL ;
	else
		return mapIter->second ;
}

// Start listening for a specific RHS user function.  When it fires we'll call to the client
// (over the connection) to implement it.
void KernelListener::RemoveRhsListener(char const* pFunctionName, Connection* pConnection)
{
	ConnectionList* pList = GetRhsListeners(pFunctionName) ;

	// We have no record of anyone listening for this event
	// That's not an error -- it's OK to call this for all events in turn
	// to make sure a connection is removed completely.
	if (pList == NULL || pList->size() == 0)
		return ;

	pList->remove(pConnection) ;
}

// Initialize this listener
void KernelListener::Init(KernelSML* pKernel)
{
	m_pKernelSML = pKernel ;

	// We always listen for RHS functions because we use the same callback to implement
	// both "cmd" and "exec" and "cmd" is valid even if no clients are registered with us
	// as it's handled internally by KernelSML.
	// (Note -- this callback is only fired if we hit one of our RHS functions, not all RHS functions so
	// registering for it all of the time doesn't incur extra overhead).
	if (!m_bListeningRHS)
	{
		m_pKernelSML->GetKernel()->AddRhsListener(gSKIEVENT_RHS_USER_FUNCTION, this) ;	
		m_bListeningRHS = true ;
	}
}

// Release memory
void KernelListener::Clear()
{
	EventManager::Clear() ;

	// Release the RHS function lists
	m_RhsMap.clear() ;

	// Stop listening for RHS functions
	if (m_bListeningRHS)
	{
		m_pKernelSML->GetKernel()->RemoveRhsListener(gSKIEVENT_RHS_USER_FUNCTION, this) ;
		m_bListeningRHS = false ;
	}
}

void KernelListener::RemoveAllListeners(Connection* pConnection)
{
	EventManager::RemoveAllListeners(pConnection) ;

	// We need to walk the list of all rhs functions, removing this connection
	for (RhsMapIter mapIter = m_RhsMap.begin() ; mapIter != m_RhsMap.end() ; mapIter++)
	{
		std::string function  = mapIter->first ;
		ConnectionList* pList = mapIter->second ;

		pList->remove(pConnection) ;
	}
}

// Called when a "SystemEvent" occurs in the kernel
void KernelListener::HandleEvent(egSKIEventId eventID, gSKI::IKernel* kernel)
{
	// We don't send the kernel over because we only support a single kernel object in SML
	unused(kernel) ;

	ConnectionListIter connectionIter = GetBegin(eventID) ;

	// Nobody is listenening for this event.  That's an error as we should unregister from the kernel in that case.
	if (connectionIter == GetEnd(eventID))
		return ;

	// We need the first connection for when we're building the message.  Perhaps this is a sign that
	// we shouldn't have rolled these methods into Connection.
	Connection* pConnection = *connectionIter ;

	// Convert eventID to a string
	char event[kMinBufferSize] ;
	Int2String(eventID, event, sizeof(event)) ;

	// Build the SML message we're doing to send.
	ElementXML* pMsg = pConnection->CreateSMLCommand(sml_Names::kCommand_Event) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamEventID, event) ;

#ifdef _DEBUG
	// Generate a text form of the XML so we can look at it in the debugger.
	char* pStr = pMsg->GenerateXMLString(true) ;
	pMsg->DeleteString(pStr) ;
#endif

	// Send this message to all listeners
	ConnectionListIter end = GetEnd(eventID) ;

	AnalyzeXML response ;

	while (connectionIter != end)
	{
		pConnection = *connectionIter ;

		// It would be faster to just send a message here without waiting for a response
		// but that could produce incorrect behavior if the client expects to act *during*
		// the event that we're notifying them about (e.g. notification that we're in the input phase).
		pConnection->SendMessageGetResponse(&response, pMsg) ;

		connectionIter++ ;
	}

	// Clean up
	delete pMsg ;
}

// Called when an "AgentEvent" occurs in the kernel
void KernelListener::HandleEvent(egSKIEventId eventID, gSKI::IAgent* agentPtr)
{
	ConnectionListIter connectionIter = GetBegin(eventID) ;

	// Nobody is listenening for this event.  That's an error as we should unregister from the kernel in that case.
	if (connectionIter == GetEnd(eventID))
		return ;

	// We need the first connection for when we're building the message.  Perhaps this is a sign that
	// we shouldn't have rolled these methods into Connection.
	Connection* pConnection = *connectionIter ;

	// Convert eventID to a string
	char event[kMinBufferSize] ;
	Int2String(eventID, event, sizeof(event)) ;

	// Build the SML message we're doing to send.
	// Pass the agent in the "name" parameter not the "agent" parameter as this is a kernel
	// level event, not an agent level one (because you need to register with the kernel to get "agent created").
	ElementXML* pMsg = pConnection->CreateSMLCommand(sml_Names::kCommand_Event) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamName, agentPtr->GetName()) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamEventID, event) ;

#ifdef _DEBUG
	// Generate a text form of the XML so we can look at it in the debugger.
	char* pStr = pMsg->GenerateXMLString(true) ;
#endif

	// Send this message to all listeners
	ConnectionListIter end = GetEnd(eventID) ;

	AnalyzeXML response ;

	while (connectionIter != end)
	{
		pConnection = *connectionIter ;

		// It would be faster to just send a message here without waiting for a response
		// but that could produce incorrect behavior if the client expects to act *during*
		// the event that we're notifying them about (e.g. notification that we're in the input phase).
		pConnection->SendMessageGetResponse(&response, pMsg) ;

		connectionIter++ ;
	}

#ifdef _DEBUG
	// Release the string form we generated for the debugger
	pMsg->DeleteString(pStr) ;
#endif

	// Clean up
	delete pMsg ;
}

// Execute the command line by building up an XML message and submitting it to our regular command processor.
bool KernelListener::ExecuteCommandLine(gSKI::IAgent* pAgent, char const* pFunctionName, char const* pArgument, int maxLengthReturnValue, char* pReturnValue)
{
	KernelSML* pKernel = m_pKernelSML ;

	// We'll pretend this came from the local (embedded) connection.
	Connection* pConnection = pKernel->GetEmbeddedConnection() ;

	// Build up a single command line from our functionName + argument combination
	std::string commandLine = pFunctionName ;

	if (pArgument)
	{
		commandLine += " " ;
		commandLine += pArgument ;
	}

	// Build up a message to execute the command line
	bool rawOutput = true ;
	ElementXML* pMsg = pConnection->CreateSMLCommand(sml_Names::kCommand_CommandLine, rawOutput) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamAgent, pAgent->GetName());
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamLine, commandLine.c_str()) ;

	AnalyzeXML incoming ;
	incoming.Analyze(pMsg) ;

	// Create a response object which the command line can fill in
	ElementXML* pResponse = pConnection->CreateSMLResponse(pMsg) ;

	// Execute the command line
	bool ok = pKernel->ProcessCommand(sml_Names::kCommand_CommandLine, pConnection, &incoming, pResponse) ;

	if (ok)
	{
		// Take the result from executing the command line and fill it in to our "pReturnValue" array.
		AnalyzeXML response ;
		response.Analyze(pResponse) ;

		char const* pResult = response.GetResultString() ;

		if (pResult)
		{
			strncpy(pReturnValue, pResult, maxLengthReturnValue) ;
			pReturnValue[maxLengthReturnValue-1] = 0 ;
		}
	}

	// Clean up
	delete pMsg ;
	delete pResponse ;

	return ok ;
}

// Handler for RHS (right hand side) function firings
// pFunctionName and pArgument define the RHS function being called (the client may parse pArgument to extract other values)
// pResultValue is a string allocated by the caller than is of size maxLengthReturnValue that should be filled in with the return value.
// The bool return value should be "true" if a return value is filled in, otherwise return false.
bool KernelListener::HandleEvent(egSKIEventId eventID, gSKI::IAgent* pAgent, bool commandLine, char const* pFunctionName, char const* pArgument,
						    int maxLengthReturnValue, char* pReturnValue)
{
	// If this should be handled by the command line processor do so now without going
	// out to the clients...we just handle this inside kernelSML.
	if (commandLine)
	{
		return ExecuteCommandLine(pAgent, pFunctionName, pArgument, maxLengthReturnValue, pReturnValue) ;
	}

	bool result = false ;

	// Get the list of connections (clients) who have registered to implement this right hand side (RHS) function.
	ConnectionList* pList = GetRhsListeners(pFunctionName) ;

	// If nobody is listening we're done (not a bug as we register for all rhs functions and only forward specific ones that the client has registered)
	if (!pList)
		return result ;

	ConnectionListIter connectionIter = pList->begin() ;

	// We need the first connection for when we're building the message.  Perhaps this is a sign that
	// we shouldn't have rolled these methods into Connection.
	Connection* pConnection = *connectionIter ;

	// Convert eventID to a string
	char event[kMinBufferSize] ;
	Int2String(eventID, event, sizeof(event)) ;

	// Also convert the length to a string
	char length[kMinBufferSize] ;
	Int2String(maxLengthReturnValue, length, sizeof(length)) ;

	// Build the SML message we're doing to send.
	// Pass the agent in the "name" parameter not the "agent" parameter as this is a kernel
	// level event, not an agent level one (because you need to register with the kernel to get "agent created").
	ElementXML* pMsg = pConnection->CreateSMLCommand(sml_Names::kCommand_Event) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamName, pAgent->GetName()) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamEventID, event) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamFunction, pFunctionName) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamValue, pArgument) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamLength, length) ;

#ifdef _DEBUG
	// Generate a text form of the XML so we can look at it in the debugger.
	char* pStr = pMsg->GenerateXMLString(true) ;
#endif

	AnalyzeXML response ;

	// We want to call embedded connections first, so that we get the best performance
	// for these functions.  I don't want to sort the list or otherwise change it so
	// instead we'll just use a rather clumsy outer loop to do this.
	for (int phase = 0 ; phase < 2 && !result ; phase++)
	{
		// Only call to embedded connections
		bool embeddedPhase = (phase == 0) ;

		// Keep looping until we get a result
		while (connectionIter != pList->end() && !result)
		{
			pConnection = *connectionIter ;

			// We call all embedded connections (same process) first before
			// trying any remote methods.  This ensures that if multiple folks register
			// for the same function we execute the fastest one (w/o going over a socket for the result).
			if (pConnection->IsRemoteConnection() && embeddedPhase)
				continue ;

			// It would be faster to just send a message here without waiting for a response
			// but that could produce incorrect behavior if the client expects to act *during*
			// the event that we're notifying them about (e.g. notification that we're in the input phase).
			bool ok = pConnection->SendMessageGetResponse(&response, pMsg) ;

			if (ok)
			{
				char const* pResult = response.GetResultString() ;

				if (pResult != NULL)
				{
					// If the listener returns a result then take that
					// value and return it in "pReturnValue" to the caller.
					// If the client returns a longer string than the caller allowed we just truncate it.
					// (In practice this shouldn't be a problem--just need to make sure nobody crashes on a super long return string).
					strncpy(pReturnValue, pResult, maxLengthReturnValue) ;
					pReturnValue[maxLengthReturnValue-1] = 0 ;	// Make sure it's NULL terminated
					result = true ;
				}
			}

			connectionIter++ ;
		}
	}

#ifdef _DEBUG
	// Release the string form we generated for the debugger
	pMsg->DeleteString(pStr) ;
#endif

	// Clean up
	delete pMsg ;

	return result ;
}
