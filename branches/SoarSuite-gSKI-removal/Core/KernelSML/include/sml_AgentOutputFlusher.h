/////////////////////////////////////////////////////////////////
// AgentOutputFlusher class file.
//
// Author: Jonathan Voigt
// Date  : February 2005
//
/////////////////////////////////////////////////////////////////
#ifndef AGENT_OUTPUT_FLUSHER_H
#define AGENT_OUTPUT_FLUSHER_H

#include "gSKI_Events.h"
#include "gSKI_Enumerations.h"
#include "gSKI_Agent.h"
#include "sml_KernelCallback.h"

namespace sml {

class PrintListener;
class XMLListener ;

class AgentOutputFlusher : public KernelCallback
{
protected:
	//gSKI::Agent* m_pAgent;
	int m_EventID ;

	// Only one listener will be filled in.
	PrintListener* m_pPrintListener;
	XMLListener*   m_pXMLListener;

public:
	AgentOutputFlusher(PrintListener* pPrintListener, AgentSML* pAgent, egSKIPrintEventId eventID);
	AgentOutputFlusher(XMLListener* pXMLListener, AgentSML* pAgent, egSKIXMLEventId eventID);
	virtual ~AgentOutputFlusher();

	virtual void OnKernelEvent(int eventID, AgentSML* pAgentSML, void* pCallData) ;

//	virtual void HandleEvent(egSKIRunEventId eventId, gSKI::Agent* agentPtr, egSKIPhaseType phase);
};

}

#endif
