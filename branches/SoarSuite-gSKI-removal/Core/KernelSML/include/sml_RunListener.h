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

#ifndef RUN_LISTENER_H
#define RUN_LISTENER_H

#include "gSKI_Events.h"
#include "gSKI_Enumerations.h"
#include "IgSKI_Iterator.h"
#include "gSKI_Agent.h"
#include "sml_EventManager.h"
#include <string>
#include <map>

namespace sml {

class KernelSML ;
class Connection ;
enum smlRunEventId; 

class RunListener : public EventManager<smlRunEventId>
{
protected:
	KernelSML*		m_pKernelSML ;
	gSKI::Agent*	m_pAgent ;

public:
	RunListener()
	{
		m_pKernelSML = 0 ;
		m_pAgent	 = 0 ;
	}

	virtual ~RunListener()
	{
		Clear() ;
	}

	void Init(KernelSML* pKernelSML, AgentSML* pAgentSML) ;

	// Called when an event occurs in the kernel
	virtual void OnKernelEvent(int eventID, AgentSML* pAgentSML, void* pCallData) ;

	// Returns true if this is the first connection listening for this event
	virtual bool AddListener(smlRunEventId eventID, Connection* pConnection) ;

	// Returns true if at least one connection remains listening for this event
	virtual bool RemoveListener(smlRunEventId eventID, Connection* pConnection) ;
} ;

}

#endif
