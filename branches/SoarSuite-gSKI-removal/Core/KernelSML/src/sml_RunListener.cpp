#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
//FIXME: #include <portability.h>

/////////////////////////////////////////////////////////////////
// RunListener class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : September 2004
//
// This class's HandleEvent method is called when
// specific events occur within the agent:
//
/*
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
*/
/////////////////////////////////////////////////////////////////

#include "sml_RunListener.h"
#include "sml_Connection.h"
#include "sml_StringOps.h"
#include "IgSKI_Production.h"
#include "gSKI_ProductionManager.h"
#include "gSKI_Kernel.h"
#include "sml_KernelSML.h"
#include "sml_AgentSML.h"

#include "assert.h"

using namespace sml ;

void RunListener::Init(sml::KernelSML *pKernelSML, AgentSML* pAgentSML)
{
	m_pKernelSML = pKernelSML ;
	m_pAgent	 = pAgentSML->GetIAgent() ;
	SetAgentSML(pAgentSML) ;
}

// Returns true if this is the first connection listening for this event
bool RunListener::AddListener(egSKIRunEventId eventID, Connection* pConnection)
{
	bool first = BaseAddListener(eventID, pConnection) ;

	if (first)
	{
		this->RegisterWithKernel(eventID) ;
		//m_pAgent->AddRunListener(eventID, this) ;
	}

	return first ;
}

// Returns true if at least one connection remains listening for this event
bool RunListener::RemoveListener(egSKIRunEventId eventID, Connection* pConnection)
{
	bool last = BaseRemoveListener(eventID, pConnection) ;

	if (last)
	{
		this->UnregisterWithKernel(eventID) ;
		//m_pAgent->RemoveRunListener(eventID, this) ;
	}

	return last ;
}

// Called when an event occurs in the kernel
void RunListener::OnKernelEvent(int eventID, AgentSML* pAgentSML, void* pCallData)
{
	// Get the first listener for this event (or return if there are none)
	ConnectionListIter connectionIter ;
	if (!EventManager<egSKIRunEventId>::GetBegin((egSKIRunEventId)eventID, &connectionIter))
		return ;

	// We need the first connection for when we're building the message.  Perhaps this is a sign that
	// we shouldn't have rolled these methods into Connection.
	Connection* pConnection = *connectionIter ;

	// Convert eventID to a string
	char const* event = m_pKernelSML->ConvertEventToString(eventID) ;

	// Convert phase to a string (cast through long long to prevent warning about pointer truncation from void*)
	int phase = (int)(long long)pCallData ;

	char phaseStr[kMinBufferSize] ;
	Int2String(phase, phaseStr, sizeof(phaseStr)) ;

	// Build the SML message we're doing to send.
	ElementXML* pMsg = pConnection->CreateSMLCommand(sml_Names::kCommand_Event) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamAgent, pAgentSML->GetName()) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamEventID, event) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamPhase, phaseStr) ;

	// Send the message out
	AnalyzeXML response ;
	SendEvent(pConnection, pMsg, &response, connectionIter, GetEnd((egSKIRunEventId)eventID)) ;

	// Clean up
	delete pMsg ;
}

// Called when a "RunEvent" occurs in the kernel
void RunListener::HandleEvent(egSKIRunEventId eventID, gSKI::Agent* agentPtr, egSKIPhaseType phase)
{
	// Get the first listener for this event (or return if there are none)
	ConnectionListIter connectionIter ;
	if (!EventManager<egSKIRunEventId>::GetBegin(eventID, &connectionIter))
		return ;

	// We need the first connection for when we're building the message.  Perhaps this is a sign that
	// we shouldn't have rolled these methods into Connection.
	Connection* pConnection = *connectionIter ;

	// Convert eventID to a string
	char const* event = m_pKernelSML->ConvertEventToString(eventID) ;

	// Convert phase to a string
	char phaseStr[kMinBufferSize] ;
	Int2String(phase, phaseStr, sizeof(phaseStr)) ;

	// Build the SML message we're doing to send.
	ElementXML* pMsg = pConnection->CreateSMLCommand(sml_Names::kCommand_Event) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamAgent, agentPtr->GetName()) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamEventID, event) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamPhase, phaseStr) ;

	// Send the message out
	AnalyzeXML response ;
	SendEvent(pConnection, pMsg, &response, connectionIter, GetEnd(eventID)) ;

	// Clean up
	delete pMsg ;
}
