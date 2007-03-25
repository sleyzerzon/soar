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

#ifndef KERNEL_CALLBACK_H
#define KERNEL_CALLBACK_H

#ifdef _MSC_VER
#pragma warning (disable : 4702)  // warning C4702: unreachable code, need to disable for VS.NET 2003 due to STL "bug" in certain cases
#endif
#include <list>
#include <map>
#ifdef _MSC_VER
#pragma warning (default : 4702)
#endif

#include <assert.h>
#include <string>

// Forward definitions for kernel
typedef struct agent_struct agent;
typedef union symbol_union Symbol;
typedef struct cons_struct list;

namespace sml {

class AgentSML ;

class KernelCallback {
protected:
	int			m_EventID ;
	AgentSML*	m_pAgentSML ;
	bool		m_Registered ;
	std::string m_CallbackID ;

public:
	static void KernelCallbackStatic(void* pAgent, void* pData, void* pCallData) ;
	static int  GetCallbackFromEventID(int eventID) ;

	KernelCallback(int eventID, AgentSML* pAgentSML)
	{
		m_EventID	= eventID ;
		m_pAgentSML = pAgentSML ;
		m_Registered = false ;
	}

	~KernelCallback()
	{
		if (m_Registered)
			UnregisterWithKernel() ;
	}

	void RegisterWithKernel() ;
	void UnregisterWithKernel() ;

	virtual void OnEvent(int eventID, AgentSML* pAgentSML, void* pCallData) = 0 ;
} ;

}


#endif // KERNEL_CALLBACK_H
