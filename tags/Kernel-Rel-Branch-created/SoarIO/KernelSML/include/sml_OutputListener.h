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
#include "IgSKI_Agent.h"

#include <map>

namespace sml {

class KernelSML ;
class Connection ;

// This map is from time tag to bool to say whether a given tag has been seen in the latest event or not
typedef std::map<long, bool>		OutputTimeTagMap ;
typedef OutputTimeTagMap::iterator	OutputTimeTagIter ;

class OutputListener : public gSKI::IWorkingMemoryListener
{
protected:
	KernelSML*	m_KernelSML ;
	Connection* m_Connection ;

	// A list of the time tags of output wmes that we've already seen and sent to the client
	// This allows us to only send changes over.
	OutputTimeTagMap m_TimeTags ;

public:
	OutputListener(KernelSML* pKernelSML, Connection* pConnection)
	{
		m_KernelSML = pKernelSML ;
		m_Connection = pConnection ;
	}

	virtual void HandleEvent(egSKIEventId eventId, gSKI::IAgent* agentPtr, egSKIWorkingMemoryChange change, gSKI::tIWmeIterator* wmelist) ;

} ;

}

#endif