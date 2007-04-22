/////////////////////////////////////////////////////////////////
// KernelCallback class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : March 2007
//
// This class handles the interface between SML events and
// kernel callbacks.
//
/////////////////////////////////////////////////////////////////

#include "sml_KernelCallback.h"
#include "sml_AgentSML.h"
#include "KernelHeaders.h"

#include <iostream>
#include <fstream>

using namespace sml ;

void KernelCallback::KernelCallbackStatic(void* pAgent, int eventID, void* pData, void* pCallData)
{
	KernelCallback* pThis = (KernelCallback*)pData ;
	agent* pKernelAgent   = (agent*)pAgent ;

	// Make sure everything matches up correctly.
	assert(pThis->m_pCallbackAgentSML->GetAgent() == pKernelAgent) ;

	// Make the callback to the non-static method
	pThis->OnKernelEvent(eventID, pThis->m_pCallbackAgentSML, pCallData) ;
}

// Not returning this as SOAR_CALLBACK_TYPE as that would expose the kernel headers through our headers.
// Want to keep kernel types internal to the implementation files so just using an int.
int KernelCallback::GetCallbackFromEventID(int eventID)
{
	switch (eventID)
	{
	case smlEVENT_AFTER_PRODUCTION_ADDED:		return PRODUCTION_JUST_ADDED_CALLBACK ;
	case smlEVENT_BEFORE_PRODUCTION_REMOVED:	return PRODUCTION_JUST_ABOUT_TO_BE_EXCISED_CALLBACK ;
	case smlEVENT_AFTER_PRODUCTION_FIRED:		return FIRING_CALLBACK ;
	case smlEVENT_BEFORE_PRODUCTION_RETRACTED:	return RETRACTION_CALLBACK ;
	default: assert(false) ;	// Unrecognized event id passed
	}

	return smlEVENT_INVALID_EVENT ;
}

KernelCallback::~KernelCallback()
{
	ClearKernelCallback() ;
}

void KernelCallback::ClearKernelCallback()
{
	for (std::map<int, bool>::iterator mapIter = m_Registered.begin() ; mapIter != m_Registered.end() ; mapIter++)
	{
		int eventID = mapIter->first ;
		bool registered = mapIter->second ;

		if (registered)
			UnregisterWithKernel(eventID) ;
	}
	m_Registered.clear() ;
}

bool KernelCallback::IsRegisteredWithKernel(int eventID)
{
	return (m_Registered[eventID] == true) ;
}

void KernelCallback::RegisterWithKernel(int eventID)
{
	// Should only register once
	assert(m_Registered[eventID] != true) ;
	m_Registered[eventID] = true ;

	// Base the id on the address of this object which ensures it's unique
	std::ostringstream buffer;
	buffer << "id_0x" << this << "_evt_" << eventID;
	std::string callbackID = buffer.str() ;

	// Did you remember to call SetAgentSML() before registering this callback?
	assert(m_pCallbackAgentSML) ;

	agent* pAgent = m_pCallbackAgentSML->GetAgent() ;
	SOAR_CALLBACK_TYPE callbackType = (SOAR_CALLBACK_TYPE)GetCallbackFromEventID(eventID) ;

	soar_add_callback (pAgent, pAgent, callbackType, KernelCallbackStatic, eventID, this, NULL, (char*)callbackID.c_str()) ;
}

void KernelCallback::UnregisterWithKernel(int eventID)
{
	if (m_Registered[eventID] != true)
		return ;

	m_Registered[eventID] = false ;

	std::ostringstream buffer;
	buffer << "id_0x" << this << "_evt_" << eventID;
	std::string callbackID = buffer.str() ;

	agent* pAgent = m_pCallbackAgentSML->GetAgent() ;
	SOAR_CALLBACK_TYPE callbackType = (SOAR_CALLBACK_TYPE)GetCallbackFromEventID(eventID) ;

	soar_remove_callback(pAgent, pAgent, callbackType, (char*)callbackID.c_str()) ;
}

