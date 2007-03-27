/////////////////////////////////////////////////////////////////
// ProductionListener class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : September 2004
//
// This class's HandleEvent method is called when
// specific events occur within the agent:
//
/*
*      gSKIEVENT_AFTER_PRODUCTION_ADDED,
*      gSKIEVENT_BEFORE_PRODUCTION_REMOVED,
*     //gSKIEVENT_BEFORE_PRODUCTION_FIRED,
*      gSKIEVENT_AFTER_PRODUCTION_FIRED,
*      gSKIEVENT_BEFORE_PRODUCTION_RETRACTED,
*/
/////////////////////////////////////////////////////////////////

#ifndef PRODUCTION_LISTENER_H
#define PRODUCTION_LISTENER_H

#include "gSKI_Events.h"
#include "gSKI_Enumerations.h"
#include "IgSKI_Iterator.h"
#include "gSKI_Agent.h"
#include "gSKI_Kernel.h"
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
	gSKI::Agent*	m_pAgent ;

public:
	ProductionListener()
	{
		m_pKernelSML = 0 ;
		m_pAgent	 = 0 ;
	}

	virtual ~ProductionListener()
	{
		Clear() ;
	}

	void Init(KernelSML* pKernelSML, AgentSML* pAgentSML) ;

	// Called when an event occurs in the kernel
	virtual void OnKernelEvent(int eventID, AgentSML* pAgentSML, void* pCallData) ;

	// Returns true if this is the first connection listening for this event
	virtual bool AddListener(egSKIProductionEventId eventID, Connection* pConnection) ;

	// Returns true if at least one connection remains listening for this event
	virtual bool RemoveListener(egSKIProductionEventId eventID, Connection* pConnection) ;

	// Called when a "ProductionEvent" occurs in the kernel
	virtual void HandleEvent(egSKIProductionEventId eventId, gSKI::Agent* agentPtr, gSKI::IProduction* prod, gSKI::IProductionInstance* match) ;
} ;

}

#endif
