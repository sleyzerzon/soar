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
#include <map>

// Forward definitions for kernel
typedef struct agent_struct agent;
typedef union symbol_union Symbol;
typedef struct cons_struct list;

namespace sml {

class AgentSML ;

class KernelCallback {
protected:
	AgentSML*	m_pAgentSML ;
	std::map<int, bool> m_Registered ;

public:
	static void KernelCallbackStatic(void* pAgent, int eventID, void* pData, void* pCallData) ;
	static int  GetCallbackFromEventID(int eventID) ;

	KernelCallback() { m_pAgentSML = 0 ; }
	virtual ~KernelCallback() ;

	void SetAgentSML(AgentSML* pAgentSML) { m_pAgentSML = pAgentSML ; }
	void ClearKernelCallback() ;
	void RegisterWithKernel(int eventID) ;
	void UnregisterWithKernel(int eventID) ;

	virtual void OnKernelEvent(int eventID, AgentSML* pAgentSML, void* pCallData) = 0 ;
} ;

}


#endif // KERNEL_CALLBACK_H
