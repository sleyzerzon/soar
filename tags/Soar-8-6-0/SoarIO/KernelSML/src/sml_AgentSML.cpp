#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/////////////////////////////////////////////////////////////////
// AgentSML class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : September 2004
//
// This class is used to keep track of information needed by SML
// (Soar Markup Language) on an agent by agent basis.
//
/////////////////////////////////////////////////////////////////

#include "sml_AgentSML.h"
#include "sml_OutputListener.h"
#include "sml_StringOps.h"
#include "sml_KernelSML.h"

#include "gSKI_Events.h"
#include "IgSKI_Wme.h"
#include "IgSKI_Agent.h"
#include "IgSKI_InputProducer.h"
#include "IgSKI_WMObject.h"
#include "IgSKI_Symbol.h"
#include "IgSKI_AgentManager.h"
#include "IgSKI_Kernel.h"

#include <assert.h>

using namespace sml ;

AgentSML::AgentSML(KernelSML* pKernelSML, gSKI::IAgent* pAgent) : /*m_AgentListener(pKernelSML, pAgent)*/ m_ProductionListener(pKernelSML, pAgent), m_RunListener(pKernelSML, pAgent), m_PrintListener(pKernelSML, pAgent)
{
	m_pKernelSML = pKernelSML ;
	m_pIAgent = pAgent ;
	m_pInputProducer = NULL ;
	m_InputLinkRoot = NULL ;
	m_OutputLinkRoot = NULL ;
	m_SuppressRunEndsEvent = false ;

	m_pBeforeDestroyedListener = NULL ;

	// Create a listener for output events and other events we listen for
	m_pOutputListener = new OutputListener(pKernelSML, pAgent) ;

	// For KernelSML (us) to work correctly we need to listen for certain events, independently of what any client is interested in
	// Currently:
	// Listen for output callback events so we can send this output over to the clients
	// Listen for "before" init-soar events (we need to know when these happen so we can release all WMEs on the input link, otherwise gSKI will fail to re-init the kernel correctly.)
	// Listen for "after" init-soar events (we need to know when these happen so we can resend the output link over to the client)
	m_pOutputListener->RegisterForKernelSMLEvents() ;
}

AgentSML::~AgentSML()
{
	// Release any objects we still own
	Clear() ;

	// If we have an output listener object, delete it now.
	// NOTE: At this point we're assuming AgentSML objects live as long as the underlying gSKI IAgent object.
	// If not, we need to unregister this listener, but we shouldn't do that here as the IAgent object may
	// be invalid by the time this destructor is called.
	delete m_pOutputListener ;

	delete m_pInputProducer ;
}

// Release any objects or other data we are keeping.  We do this just
// prior to deleting AgentSML, but before the underlying gSKI agent has been deleted
void AgentSML::Clear()
{
	// Release any WME objects we still own.
	ReleaseAllWmes() ;

	m_ProductionListener.Clear();
	m_RunListener.Clear();
	m_PrintListener.Clear();
	m_pOutputListener->Clear() ;
}

void AgentSML::ReleaseAllWmes()
{
	// Release any WME objects we still own.
	for (TimeTagMapIter mapIter = m_TimeTagMap.begin() ; mapIter != m_TimeTagMap.end() ; mapIter++)
	{
		gSKI::IWme* pWme = mapIter->second ;
		std::string value = pWme->GetValue()->GetString() ;

		pWme->Release() ;
	}

	if (m_InputLinkRoot)
		m_InputLinkRoot->Release() ;
	m_InputLinkRoot = NULL ;
/*
	if (m_OutputLinkRoot)
		m_OutputLinkRoot->Release() ;
	m_OutputLinkRoot = NULL ;
*/
	m_TimeTagMap.clear() ;
	m_ToClientIdentifierMap.clear() ;
	m_IdentifierMap.clear() ;
}

void AgentSML::RemoveAllListeners(Connection* pConnection)
{
	m_ProductionListener.RemoveAllListeners(pConnection);
	m_RunListener.RemoveAllListeners(pConnection);
	m_PrintListener.RemoveAllListeners(pConnection);
	m_pOutputListener->RemoveAllListeners(pConnection) ; 
}

class AgentSML::AgentBeforeDestroyedListener: public gSKI::IAgentListener
{
public:
	// This handler is called right before the agent is actually deleted
	// inside gSKI.  We need to clean up any object we own now.
	virtual void HandleEvent(egSKIAgentEventId, gSKI::IAgent* pAgent)
	{
		KernelSML* pKernelSML = KernelSML::GetKernelSML() ;

		// Release any wmes or other objects we're keeping
		AgentSML* pAgentSML = pKernelSML->GetAgentSML(pAgent) ;
		pAgentSML->Clear() ;

		// Remove the listeners that KernelSML uses for this agent.
		// This is important.  Otherwise if we create a new agent using the same kernel object
		// the listener will still exist inside gSKI and will crash when an agent event is next generated.
		pAgentSML->GetOutputListener()->UnRegisterForKernelSMLEvents() ;

		// Unregister ourselves (this is important for the same reasons as listed above)
		pKernelSML->GetKernel()->GetAgentManager()->RemoveAgentListener(gSKIEVENT_BEFORE_AGENT_DESTROYED, this) ;

		// Then delete our matching agent sml information
		pKernelSML->DeleteAgentSML(pAgent) ;

		// Do self clean-up of this object as it's just called
		// prior to deleting the AgentSML structure.
		delete this ;
	}
};

void AgentSML::RegisterForBeforeAgentDestroyedEvent()
{
	// We should do this immediately before we delete the agent.
	// We shouldn't do it earlier or we can't be sure it'll be last on the list of listeners which is where we
	// need it to be (so that we clear our information about the gSKI agent *after* we've notified any of our listeners
	// about this event).
	m_pBeforeDestroyedListener = new AgentBeforeDestroyedListener() ;
	m_pKernelSML->GetKernel()->GetAgentManager()->AddAgentListener(gSKIEVENT_BEFORE_AGENT_DESTROYED, m_pBeforeDestroyedListener) ;
}

/*************************************************************
* @brief	When set, this flag will cause Soar to break when
*			output is next generated during a run.
*************************************************************/
bool AgentSML::SetStopOnOutput(bool state)
{
	if (!m_pOutputListener)
		return false ;

	m_pOutputListener->SetStopOnOutput(state) ;

	return true ;
}

/*************************************************************
* @brief	Converts an id from a client side value to a kernel side value.
*			We need to be able to do this because the client is adding a collection
*			of wmes at once, so it makes up the ids for those objects.
*			But the kernel will assign them a different value when the
*			wme is actually added in the kernel.
*************************************************************/
bool AgentSML::ConvertID(char const* pClientID, std::string* pKernelID)
{
	if (pClientID == NULL)
		return false ;

	IdentifierMapIter iter = m_IdentifierMap.find(pClientID) ;

	if (iter == m_IdentifierMap.end())
	{
		// If the client id is not in the map, then we may have been
		// passed a kernel id (this will happen at times).
		// So return the value we were passed
		*pKernelID = pClientID ;
		return false ;
	}
	else
	{
		// If we found a mapping, return the mapped value
		*pKernelID = iter->second ;
		return true ;
	}
}

void AgentSML::RecordIDMapping(char const* pClientID, char const* pKernelID)
{
	m_IdentifierMap[pClientID] = pKernelID ;

	// Record in both directions, so we can clean up (at which time we only know the kernel side ID).
	m_ToClientIdentifierMap[pKernelID] = pClientID ;
}

void AgentSML::RemoveID(char const* pKernelID)
{
	IdentifierMapIter iter = m_ToClientIdentifierMap.find(pKernelID) ;

	// This identifier should have been in the table
	assert (iter != m_ToClientIdentifierMap.end()) ;
	if (iter == m_ToClientIdentifierMap.end())
		return ;

	// Delete this mapping from both tables
	std::string clientID = iter->second ;
	m_IdentifierMap.erase(clientID) ;
	m_ToClientIdentifierMap.erase(pKernelID) ;
}

/*************************************************************
* @brief	Converts a time tag from a client side value to
*			a kernel side object
*************************************************************/
gSKI::IWme* AgentSML::ConvertTimeTag(char const* pTimeTag)
{
	if (pTimeTag == NULL)
		return NULL ;

	TimeTagMapIter iter = m_TimeTagMap.find(pTimeTag) ;

	if (iter == m_TimeTagMap.end())
	{
		return NULL ;
	}
	else
	{
		// If we found a mapping, return the mapped value
		gSKI::IWme* result = iter->second ;
		return result ;
	}
}

/*************************************************************
* @brief	Maps from a client side time tag to a kernel side WME.
*************************************************************/
void AgentSML::RecordTimeTag(char const* pTimeTag, gSKI::IWme* pWME)
{
	m_TimeTagMap[pTimeTag] = pWME ;
}

void AgentSML::RecordLongTimeTag(long timeTag, gSKI::IWme* pWME)
{
	char str[kMinBufferSize] ;
	Int2String(timeTag, str, sizeof(str)) ;

	m_TimeTagMap[str] = pWME ;
}

void AgentSML::RemoveTimeTag(char const* pTimeTag)
{
	m_TimeTagMap.erase(pTimeTag) ;
}

void AgentSML::RemoveLongTimeTag(long timeTag)
{
	char str[kMinBufferSize] ;
	Int2String(timeTag, str, sizeof(str)) ;

	m_TimeTagMap.erase(str) ;
}
