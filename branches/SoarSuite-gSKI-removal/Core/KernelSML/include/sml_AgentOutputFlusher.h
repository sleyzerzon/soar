/////////////////////////////////////////////////////////////////
// AgentOutputFlusher class file.
//
// Author: Jonathan Voigt
// Date  : February 2005
//
/////////////////////////////////////////////////////////////////
#ifndef AGENT_OUTPUT_FLUSHER_H
#define AGENT_OUTPUT_FLUSHER_H

#include "sml_KernelCallback.h"

namespace sml {

class PrintListener;
class XMLListener ;

enum smlPrintEventId;
enum smlXMLEventId;

class AgentOutputFlusher : public KernelCallback
{
protected:
	int m_EventID ;

	// Only one listener will be filled in.
	PrintListener* m_pPrintListener;
	XMLListener*   m_pXMLListener;

public:
	AgentOutputFlusher(PrintListener* pPrintListener, AgentSML* pAgent, smlPrintEventId eventID);
	AgentOutputFlusher(XMLListener* pXMLListener, AgentSML* pAgent, smlXMLEventId eventID);
	virtual ~AgentOutputFlusher();

	virtual void OnKernelEvent(int eventID, AgentSML* pAgentSML, void* pCallData) ;
};

}

#endif
