#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/////////////////////////////////////////////////////////////////
// AgentListener class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : September 2004
//
// This class's HandleEvent method is called when
// specific events occur within the agent:
//
/*
*     // Agent events
*     @li gSKIEVENT_BEFORE_SMALLEST_STEP
*     @li gSKIEVENT_AFTER_SMALLEST_STEP
*     @li gSKIEVENT_BEFORE_ELABORATION_CYCLE
*     @li gSKIEVENT_AFTER_ELABORATION_CYCLE
*     @li gSKIEVENT_BEFORE_PHASE_EXECUTED
*     @li gSKIEVENT_AFTER_PHASE_EXECUTED
*     @li gSKIEVENT_BEFORE_DECISION_CYCLE
*     @li gSKIEVENT_AFTER_DECISION_CYCLE
*     @li gSKIEVENT_AFTER_INTERRUPT
*     @li gSKIEVENT_BEFORE_RUNNING
*     @li gSKIEVENT_AFTER_RUNNING
*
*      // Agent manager
*      gSKIEVENT_AFTER_AGENT_CREATED,
*      gSKIEVENT_BEFORE_AGENT_DESTROYED,
*      gSKIEVENT_BEFORE_AGENT_REINITIALIZED,
*      gSKIEVENT_AFTER_AGENT_REINITIALIZED,
*
*      // Production Manager
*      gSKIEVENT_AFTER_PRODUCTION_ADDED,
*      gSKIEVENT_BEFORE_PRODUCTION_REMOVED,
*     //gSKIEVENT_BEFORE_PRODUCTION_FIRED,
*      gSKIEVENT_AFTER_PRODUCTION_FIRED,
*      gSKIEVENT_BEFORE_PRODUCTION_RETRACTED,
*/
/////////////////////////////////////////////////////////////////

#include "sml_AgentListener.h"
#include "sml_Connection.h"
#include "sml_StringOps.h"
#include "IgSKI_AgentManager.h"
#include "sml_KernelSML.h"

#include "assert.h"

using namespace sml ;

// Uncomment this symbol to disable print output buffering.
// #define DISABLE_PRINT_OUTPUT_BUFFERING

// Returns true if this is the first connection listening for this event
bool AgentListener::AddListener(egSKIAgentEventId eventID, Connection* pConnection)
{
	bool first = EventManager<egSKIAgentEventId>::BaseAddListener(eventID, pConnection) ;

	if (first)
	{
		m_pKernelSML->GetKernel()->GetAgentManager()->AddAgentListener(eventID, this) ;
	}

	return first ;
}

// Returns true if at least one connection remains listening for this event
bool AgentListener::RemoveListener(egSKIAgentEventId eventID, Connection* pConnection)
{
    bool last = EventManager<egSKIAgentEventId>::BaseRemoveListener(eventID, pConnection) ;

	if (last)
	{
		m_pKernelSML->GetKernel()->GetAgentManager()->RemoveAgentListener(eventID, this) ;
	}

	return last ;
}

// Called when an "AgentEvent" occurs in the kernel
void AgentListener::HandleEvent(egSKIAgentEventId eventID, gSKI::IAgent* agentPtr)
{
	ConnectionListIter connectionIter = EventManager<egSKIAgentEventId>::GetBegin(eventID) ;

	// Nobody is listenening for this event.  That's an error as we should unregister from the kernel in that case.
	if (connectionIter == EventManager<egSKIAgentEventId>::GetEnd(eventID))
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
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamName, agentPtr == NULL ? "" : agentPtr->GetName()) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamEventID, event) ;

#ifdef _DEBUG
	// Generate a text form of the XML so we can look at it in the debugger.
	char* pStr = pMsg->GenerateXMLString(true) ;
#endif

	// Send this message to all listeners
	ConnectionListIter end = EventManager<egSKIAgentEventId>::GetEnd(eventID) ;

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

/////////////////////////////////////////////////////////////////
// AgentListener class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : September 2004
//
// This class's HandleEvent method is called when
// specific events occur within the agent:
//
/*
*     // Agent events
*     @li gSKIEVENT_BEFORE_SMALLEST_STEP
*     @li gSKIEVENT_AFTER_SMALLEST_STEP
*     @li gSKIEVENT_BEFORE_ELABORATION_CYCLE
*     @li gSKIEVENT_AFTER_ELABORATION_CYCLE
*     @li gSKIEVENT_BEFORE_PHASE_EXECUTED
*     @li gSKIEVENT_AFTER_PHASE_EXECUTED
*     @li gSKIEVENT_BEFORE_DECISION_CYCLE
*     @li gSKIEVENT_AFTER_DECISION_CYCLE
*     @li gSKIEVENT_AFTER_INTERRUPT
*     @li gSKIEVENT_BEFORE_RUNNING
*     @li gSKIEVENT_AFTER_RUNNING
*
*      // Agent manager
*      gSKIEVENT_AFTER_AGENT_CREATED,
*      gSKIEVENT_BEFORE_AGENT_DESTROYED,
*      gSKIEVENT_BEFORE_AGENT_REINITIALIZED,
*      gSKIEVENT_AFTER_AGENT_REINITIALIZED,
*
*      // Production Manager
*      gSKIEVENT_AFTER_PRODUCTION_ADDED,
*      gSKIEVENT_BEFORE_PRODUCTION_REMOVED,
*     //gSKIEVENT_BEFORE_PRODUCTION_FIRED,
*      gSKIEVENT_AFTER_PRODUCTION_FIRED,
*      gSKIEVENT_BEFORE_PRODUCTION_RETRACTED,
*/
/////////////////////////////////////////////////////////////////

/*#include "sml_AgentListener.h"
#include "sml_Connection.h"
#include "sml_StringOps.h"
#include "IgSKI_Production.h"
#include "IgSKI_ProductionManager.h"
#include "IgSKI_Kernel.h"
#include "sml_KernelSML.h"

#include "assert.h"

using namespace sml ;

// Uncomment this symbol to disable print output buffering.
// #define DISABLE_PRINT_OUTPUT_BUFFERING

// Returns true if this is the first connection listening for this event
bool AgentListener::AddListener(egSKIEventId eventID, Connection* pConnection)
{
	bool first = BaseAddListener(eventID, pConnection) ;

	if (first)
	{
		if (IsRunEvent(eventID))
			m_Agent->AddRunListener(eventID, this) ;
		else if (IsProductionEvent(eventID))
			m_Agent->GetProductionManager()->AddProductionListener(eventID, this) ;
		else if (IsPrintEvent(eventID))	
		{
#ifndef DISABLE_PRINT_OUTPUT_BUFFERING
			m_pAgentOutputFlusher = new AgentOutputFlusher(this, m_Agent);
#endif // DISABLE_PRINT_OUTPUT_BUFFERING
			m_Agent->AddPrintListener(eventID, this); 
		}
	}

	return first ;
}

// Returns true if at least one connection remains listening for this event
bool AgentListener::RemoveListener(egSKIEventId eventID, Connection* pConnection)
{
	bool last = BaseRemoveListener(eventID, pConnection) ;

	if (last)
	{
		if (IsRunEvent(eventID))
			m_Agent->RemoveRunListener(eventID, this) ;
		else if (IsProductionEvent(eventID))
			m_Agent->GetProductionManager()->RemoveProductionListener(eventID, this) ;
		else if (IsPrintEvent(eventID))
		{
			m_Agent->RemovePrintListener(eventID, this); 
#ifndef DISABLE_PRINT_OUTPUT_BUFFERING
			delete m_pAgentOutputFlusher;
			m_pAgentOutputFlusher = 0;
#endif // DISABLE_PRINT_OUTPUT_BUFFERING
		}
	}

	return last ;
}

// Called when a "RunEvent" occurs in the kernel
void AgentListener::HandleEvent(egSKIEventId eventID, gSKI::IAgent* agentPtr, egSKIPhaseType phase)
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

	// Convert phase to a string
	char phaseStr[kMinBufferSize] ;
	Int2String(phase, phaseStr, sizeof(phaseStr)) ;

	// Build the SML message we're doing to send.
	ElementXML* pMsg = pConnection->CreateSMLCommand(sml_Names::kCommand_Event) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamAgent, agentPtr->GetName()) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamEventID, event) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamPhase, phaseStr) ;

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

// Called when a "ProductionEvent" occurs in the kernel
void AgentListener::HandleEvent(egSKIEventId eventID, gSKI::IAgent* agentPtr, gSKI::IProduction* prod, gSKI::IProductionInstance* match)
{
	// This class isn't implemented in gSKI yet.
	unused(match) ;

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
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamAgent, agentPtr->GetName()) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamEventID, event) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamName, prod->GetName()) ;

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

// Called when a "PrintEvent" occurs in the kernel
void AgentListener::HandleEvent(egSKIEventId eventID, gSKI::IAgent* agentPtr, const char* msg) 
{
	unused(eventID);
	unused(agentPtr);

	// If we're getting anything but a print event here, that isn't right
	assert(eventID == gSKIEVENT_PRINT);

	// We're assuming this is correct in the flush output function, so we should check it here
	assert(agentPtr == m_Agent);

	// If the print callbacks have been disabled, then don't forward this message
	// on to the clients.  This allows us to use the print callback within the kernel to
	// retrieve information without it appearing in the trace.  (One day we won't need to do this enable/disable game).
	if (!m_EnablePrintCallback)
		return ;

	// Buffer print output to be flushed later
	m_BufferedPrintOutput += msg;

#ifdef DISABLE_PRINT_OUTPUT_BUFFERING
	FlushOutput();
#endif
}

void AgentListener::FlushOutput() 
{
	if (!m_BufferedPrintOutput.size())
		return;

	ConnectionListIter connectionIter = GetBegin(gSKIEVENT_PRINT);

	// Nobody is listenening for this event.  That's an error as we should unregister from the kernel in that case.
	if (connectionIter == GetEnd(gSKIEVENT_PRINT))
		return;

	// We need the first connection for when we're building the message.  Perhaps this is a sign that
	// we shouldn't have rolled these methods into Connection.
	Connection* pConnection = *connectionIter;

	// Convert eventID to a string
	char event[kMinBufferSize];
	Int2String(gSKIEVENT_PRINT, event, sizeof(event));

	// Build the SML message we're going to send.
	ElementXML* pMsg = pConnection->CreateSMLCommand(sml_Names::kCommand_Event);
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamAgent, m_Agent->GetName());
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamEventID, event);
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamMessage, m_BufferedPrintOutput.c_str());

	m_BufferedPrintOutput.clear();

#ifdef _DEBUG
	// Generate a text form of the XML so we can look at it in the debugger.
	char* pStr = pMsg->GenerateXMLString(true);
	pMsg->DeleteString(pStr);
#endif

	// Send this message to all listeners
	ConnectionListIter end = GetEnd(gSKIEVENT_PRINT);

	AnalyzeXML response;

	while (connectionIter != end)
	{
		pConnection = *connectionIter;

		// It would be faster to just send a message here without waiting for a response
		// but that could produce incorrect behavior if the client expects to act *during*
		// the event that we're notifying them about (e.g. notification that we're in the input phase).
		pConnection->SendMessageGetResponse(&response, pMsg);

		connectionIter++;
	}

	// Clean up
	delete pMsg;
}
*/

