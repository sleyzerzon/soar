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
*      // Production Manager
*      gSKIEVENT_AFTER_PRODUCTION_ADDED,
*      gSKIEVENT_BEFORE_PRODUCTION_REMOVED,
*     //gSKIEVENT_BEFORE_PRODUCTION_FIRED,
*      gSKIEVENT_AFTER_PRODUCTION_FIRED,
*      gSKIEVENT_BEFORE_PRODUCTION_RETRACTED,
*
*/
/////////////////////////////////////////////////////////////////

#ifndef PRINT_LISTENER_H
#define PRINT_LISTENER_H

#include "gSKI_Events.h"
#include "gSKI_Enumerations.h"
#include "IgSKI_Iterator.h"
#include "IgSKI_Agent.h"
#include "IgSKI_Kernel.h"
#include "sml_EventManager.h"
#include "sml_AgentOutputFlusher.h"

#include <string>
#include <map>

namespace sml {

class KernelSML ;
class Connection ;

class PrintListener : public gSKI::IPrintListener, public EventManager<egSKIPrintEventId>
{
protected:
	KernelSML*		m_pKernelSML ;
	gSKI::IAgent*	m_pAgent ;
	std::string		m_BufferedPrintOutput;
	AgentOutputFlusher* m_pAgentOutputFlusher;

	// When false we don't forward print callback events to the listeners.  (Useful when we're backdooring into the kernel)
	bool			m_EnablePrintCallback ;

public:
	PrintListener(KernelSML* pKernelSML, gSKI::IAgent* pAgent)
	{
		m_pKernelSML = pKernelSML ;
		m_pAgent	 = pAgent ;
		m_EnablePrintCallback = true ;
		m_pAgentOutputFlusher = 0;
	}

	virtual ~PrintListener()
	{
		Clear() ;
	}

	// Returns true if this is the first connection listening for this event
	virtual bool AddListener(egSKIPrintEventId eventID, Connection* pConnection) ;

	// Returns true if at least one connection remains listening for this event
	virtual bool RemoveListener(egSKIPrintEventId eventID, Connection* pConnection) ;

	// Called when a "PrintEvent" occurs in the kernel
	virtual void HandleEvent(egSKIPrintEventId, gSKI::IAgent*, const char* msg);

	// Allows us to temporarily stop forwarding print callback output from the kernel to the SML listeners
	void EnablePrintCallback(bool enable) { m_EnablePrintCallback = enable ; }

	// Activate the print callback (flush output)
	void FlushOutput();

} ;

}

#endif
