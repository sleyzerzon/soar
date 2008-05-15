#include <portability.h>

/////////////////////////////////////////////////////////////////
// AgentOutputFlusher class file.
//
// Author: Jonathan Voigt
// Date  : February 2005
//
/////////////////////////////////////////////////////////////////

#include "sml_Utils.h"
#include "sml_AgentOutputFlusher.h"
#include "assert.h"
#include "sml_PrintListener.h"
#include "sml_XMLListener.h"

using namespace sml ;

AgentOutputFlusher::AgentOutputFlusher(PrintListener* pPrintListener, AgentSML* pAgent, smlPrintEventId eventID) : m_pPrintListener(pPrintListener)
{
	m_pXMLListener = NULL ;
	m_EventID = eventID ;
	this->SetAgentSML(pAgent) ;
	this->RegisterWithKernel(gSKIEVENT_AFTER_DECISION_CYCLE) ;
	this->RegisterWithKernel(gSKIEVENT_AFTER_RUNNING) ;
	//m_pAgent->AddRunListener(gSKIEVENT_AFTER_DECISION_CYCLE, this);
	//m_pAgent->AddRunListener(gSKIEVENT_AFTER_RUNNING, this);
}

AgentOutputFlusher::AgentOutputFlusher(XMLListener* pXMLListener, AgentSML* pAgent, smlXMLEventId eventID) : m_pXMLListener(pXMLListener)
{
	m_pPrintListener = NULL ;
	m_EventID = eventID ;
	this->SetAgentSML(pAgent) ;
	this->RegisterWithKernel(gSKIEVENT_AFTER_DECISION_CYCLE) ;
	this->RegisterWithKernel(gSKIEVENT_AFTER_RUNNING) ;
}

AgentOutputFlusher::~AgentOutputFlusher()
{
	this->UnregisterWithKernel(gSKIEVENT_AFTER_DECISION_CYCLE) ;
	this->UnregisterWithKernel(gSKIEVENT_AFTER_RUNNING) ;

	//m_pAgent->RemoveRunListener(gSKIEVENT_AFTER_DECISION_CYCLE, this);
	//m_pAgent->RemoveRunListener(gSKIEVENT_AFTER_RUNNING, this);
}

void AgentOutputFlusher::OnKernelEvent(int eventID, AgentSML* pAgentSML, void* pCallData)
{
	assert(eventID == gSKIEVENT_AFTER_DECISION_CYCLE || eventID == gSKIEVENT_AFTER_RUNNING);
	unused(eventID);
	unused(pAgentSML);
	unused(pCallData);

	if (m_pPrintListener)
		m_pPrintListener->FlushOutput(static_cast<smlPrintEventId>(m_EventID));
	if (m_pXMLListener)
		m_pXMLListener->FlushOutput(static_cast<smlXMLEventId>(m_EventID)) ;
}

/*
void AgentOutputFlusher::HandleEvent(smlRunEventId eventId, gSKI::Agent* agentPtr, egSKIPhaseType phase)
{
	assert(eventId == gSKIEVENT_AFTER_DECISION_CYCLE || eventId == gSKIEVENT_AFTER_RUNNING);
	assert(agentPtr == m_pAgent);
	unused(eventId);
	unused(agentPtr);
	unused(phase);

	if (m_pPrintListener)
		m_pPrintListener->FlushOutput((egSKIPrintEventId)m_EventID);
	if (m_pXMLListener)
		m_pXMLListener->FlushOutput((smlXMLEventId)m_EventID) ;
}
*/
