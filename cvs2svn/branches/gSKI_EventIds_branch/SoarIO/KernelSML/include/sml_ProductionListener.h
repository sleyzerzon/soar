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
*      // Production Manager
*      gSKIEVENT_AFTER_PRODUCTION_ADDED,
*      gSKIEVENT_BEFORE_PRODUCTION_REMOVED,
*     //gSKIEVENT_BEFORE_PRODUCTION_FIRED,
*      gSKIEVENT_AFTER_PRODUCTION_FIRED,
*      gSKIEVENT_BEFORE_PRODUCTION_RETRACTED,
*
*/
/////////////////////////////////////////////////////////////////

#ifndef PRODUCTION_LISTENER_H
#define PRODUCTION_LISTENER_H

#include "gSKI_Events.h"
#include "gSKI_Enumerations.h"
#include "IgSKI_Iterator.h"
#include "IgSKI_Agent.h"
#include "IgSKI_Kernel.h"
#include "sml_EventManager.h"

#include <string>
#include <map>

namespace sml {

class KernelSML ;
class Connection ;

class ProductionListener : public gSKI::IProductionListener, public EventManager<egSKIProductionEventId>
{
protected:
	KernelSML*		m_pKernelSML ;
	gSKI::IAgent*	m_pAgent ;

public:
	ProductionListener(KernelSML* pKernelSML, gSKI::IAgent* pAgent)
	{
		m_pKernelSML = pKernelSML ;
		m_pAgent	 = pAgent ;
	}

	virtual ~ProductionListener()
	{
		Clear() ;
	}

	// Returns true if this is the first connection listening for this event
	virtual bool AddListener(egSKIProductionEventId eventID, Connection* pConnection) ;

	// Returns true if at least one connection remains listening for this event
	virtual bool RemoveListener(egSKIProductionEventId eventID, Connection* pConnection) ;

	// Called when a "ProductionEvent" occurs in the kernel
	virtual void HandleEvent(egSKIProductionEventId eventId, gSKI::IAgent* agentPtr, gSKI::IProduction* prod, gSKI::IProductionInstance* match) ;
} ;

}

#endif