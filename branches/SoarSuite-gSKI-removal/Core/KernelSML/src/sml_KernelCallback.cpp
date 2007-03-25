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

void KernelCallback::KernelCallbackStatic(void* pAgent, void* pData, void* pCallData)
{
	KernelCallback* pThis = (KernelCallback*)pData ;
	agent* pKernelAgent   = (agent*)pAgent ;

	// Make sure everything matches up correctly.
	assert(pThis->m_pAgentSML->GetAgent() == pKernelAgent) ;

	// Make the callback to the non-static method
	pThis->OnEvent(pThis->m_EventID, pThis->m_pAgentSML, pCallData) ;
}

// Not returning this as SOAR_CALLBACK_TYPE as that would expose the kernel headers through our headers.
// Want to keep kernel types internal to the implementation files so just using an int.
int KernelCallback::GetCallbackFromEventID(int eventID)
{
	return PRODUCTION_JUST_ADDED_CALLBACK ;
}

void KernelCallback::RegisterWithKernel()
{
	// Should only register once
	assert(!m_Registered) ;

	m_Registered = true ;

	// Base the id on the address of this object which ensures it's unique
	std::ostringstream buffer;
	buffer << "id_0x" << this;
	m_CallbackID = buffer.str();

	agent* pAgent = m_pAgentSML->GetAgent() ;
	SOAR_CALLBACK_TYPE callbackType = (SOAR_CALLBACK_TYPE)GetCallbackFromEventID(m_EventID) ;

	soar_add_callback (pAgent, pAgent, callbackType, KernelCallbackStatic, this, NULL, (char*)m_CallbackID.c_str()) ;
}

void KernelCallback::UnregisterWithKernel()
{
	if (!m_Registered)
		return ;

	m_Registered = false ;
	agent* pAgent = m_pAgentSML->GetAgent() ;
	SOAR_CALLBACK_TYPE callbackType = (SOAR_CALLBACK_TYPE)GetCallbackFromEventID(m_EventID) ;

	soar_remove_callback(pAgent, pAgent, callbackType, (char*)m_CallbackID.c_str()) ;
}

