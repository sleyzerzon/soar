/////////////////////////////////////////////////////////////////
// OutputListener class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : September 2004
//
// This class's HandleEvent method is called when
// the agent adds wmes to the output link.
//
/////////////////////////////////////////////////////////////////

#ifndef OUTPUT_LISTENER_H
#define OUTPUT_LISTENER_H

#include "gSKI_Events.h"
#include "gSKI_Enumerations.h"
#include "IgSKI_Iterator.h"
#include "gSKI_Agent.h"
#include "sml_EventManager.h"

#include <map>

typedef struct io_wme_struct io_wme;
typedef struct wme_struct wme;

namespace sml {

class KernelSML ;
class Connection ;
class TagWme ;

// This map is from time tag to bool to say whether a given tag has been seen in the latest event or not
typedef std::map< long, bool >		OutputTimeTagMap ;
typedef OutputTimeTagMap::iterator	OutputTimeTagIter ;

class OutputListener : public gSKI::IAgentListener, public EventManager<egSKIWorkingMemoryEventId>
{
protected:
	KernelSML*		m_KernelSML ;
	gSKI::Agent*	m_Agent ;

	// A list of the time tags of output wmes that we've already seen and sent to the client
	// This allows us to only send changes over.
	OutputTimeTagMap m_TimeTags ;

public:
	OutputListener()
	{
		m_KernelSML = 0 ;
		m_Agent = 0 ;
	}

	virtual ~OutputListener()
	{
		Clear() ;
	}

	static TagWme* CreateTagWme(wme* wme) ;
	static TagWme* CreateTagIOWme(io_wme* wme) ;

	void Init(KernelSML* pKernelSML, AgentSML* pAgentSML);

	// Called when an event occurs in the kernel
	virtual void OnKernelEvent(int eventID, AgentSML* pAgentSML, void* pCallData) ;

	// Send output out to the clients
	virtual void SendOutput(egSKIWorkingMemoryEventId eventID, AgentSML* pAgentSML, int outputMode, io_wme* io_wmelist) ;

	// Register for the events that KernelSML itself needs to know about in order to work correctly.
	void RegisterForKernelSMLEvents() ;

	// UnRegister for the events that KernelSML itself needs to know about in order to work correctly.
	void UnRegisterForKernelSMLEvents() ;

	// Returns true if this is the first connection listening for this event
	virtual bool AddListener(egSKIWorkingMemoryEventId eventID, Connection* pConnection) ;

	// Returns true if at least one connection remains listening for this event
	virtual bool RemoveListener(egSKIWorkingMemoryEventId eventID, Connection* pConnection) ;

	// Working memory event listener (called when the agent generates output)
	virtual void HandleEvent(egSKIWorkingMemoryEventId eventId, gSKI::Agent* agentPtr, egSKIWorkingMemoryChange change, gSKI::tIWmeIterator* wmelist) ;

	// Agent event listener (called when soar is re-initialized)
	virtual void HandleEvent(egSKIAgentEventId eventId, gSKI::Agent* agentPtr) ;
} ;

}

#endif
