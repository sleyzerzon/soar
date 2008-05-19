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

#ifndef SML_AGENT_SML_H
#define SML_AGENT_SML_H

//#include "sml_AgentListener.h" 
#include "sml_ProductionListener.h"
#include "sml_RunListener.h"
#include "sml_PrintListener.h"
#include "sml_XMLListener.h"
#include "sml_OutputListener.h"
#include "sml_InputListener.h"

#include "callback.h"


// Forward declarations for gSKI
namespace gSKI {
	class IWme ;
	class IWMObject;
	class InputLink;
	struct Error;
}

// Forward definitions for kernel
typedef struct agent_struct agent;
typedef union symbol_union Symbol;
typedef struct cons_struct list;
typedef struct wme_struct wme;

#include <map>
#include <list>
#include <string>

namespace sml {

// Forward declarations for SML
class OutputListener ;
class KernelSML ;
class RhsFunction ;
class AgentRunCallback ;

// badbad: shouldn't we be using hash_maps for these?

// Map from a client side identifier to a kernel side one (e.g. "o3" => "O5")
typedef std::map< std::string, std::string >	IdentifierMap ;
typedef IdentifierMap::iterator				IdentifierMapIter ;
typedef IdentifierMap::const_iterator		IdentifierMapConstIter ;

// Keep track of instances of client side ids
typedef std::map< std::string, int >		IdentifierRefMap ;
typedef IdentifierRefMap::iterator			IdentifierRefMapIter ;
typedef IdentifierRefMap::const_iterator	IdentifierRefMapConstIter ;

// Map from a client side time tag (as a string) to a kernel side WME* object
// (Had planned to just map the time tag to a kernel time tag...but it turns out
//  there's no quick way to look up an object in the kernel from its time tag).
typedef std::map< std::string, gSKI::IWme* >	TimeTagMap ;
typedef TimeTagMap::iterator				TimeTagMapIter ;
typedef TimeTagMap::const_iterator			TimeTagMapConstIter ;

// Map from client side time tag to a kernel time tag
typedef std::map< std::string, long >	TimeMap ;
typedef TimeMap::iterator				TimeMapIter ;

// List of input messages waiting for the next input phase callback from the kernel
typedef std::list<ElementXML*>				PendingInputList ;
typedef PendingInputList::iterator			PendingInputListIter ;

typedef std::map< std::string, wme* >		KernelTimeTagMap ;
typedef KernelTimeTagMap::iterator			KernelTimeTagMapIter ;
typedef KernelTimeTagMap::const_iterator	KernelTimeTagMapConstIter ;

class AgentSML
{
	friend class KernelSML ;
	friend class InputListener ;

public:
	gSKI::InputLink*           m_inputlink;         /**< A pointer to this agent's input link. */

	static void InputPhaseCallback( soar_callback_agent agent, soar_callback_event_id eventid, soar_callback_data callbackdata, soar_call_data calldata );

protected:

	// A reference to the underlying kernel agent object
	agent*			m_agent ;

	// Pointer back to the owning kernel SML object
	KernelSML*		m_pKernelSML ;

	// Map from client side identifiers to kernel side ones
	IdentifierMap	m_IdentifierMap ;

	// Keep track of number of instances of client side identifiers
	IdentifierRefMap m_IdentifierRefMap;

	// Map from client side time tags (as strings) to kernel side WME* objects
	TimeTagMap			m_TimeTagMap ;
	KernelTimeTagMap	m_KernelTimeTagMap ;
	TimeMap				m_TimeMap ;

	// For cleanup we also need a map from kernel side identifiers to client side ones (for cleanup)
	IdentifierMap	m_ToClientIdentifierMap ;

	// Used to listen for kernel/gSKI events that are agent specific
	ProductionListener	m_ProductionListener;
	RunListener			m_RunListener;
	PrintListener		m_PrintListener;
	XMLListener			m_XMLListener ;

	// This is a callback we can register to listen for changes to the output-link
	OutputListener		m_OutputListener ;

	// This listener is called during the kernel's input phase callback
	InputListener		m_InputListener ;

	// We have to keep pointers to these objects so that we can release then during an init-soar.  Youch!
	gSKI::IWMObject*	m_InputLinkRoot ;
	gSKI::IWMObject*	m_OutputLinkRoot ;

	// Input changes waiting to be processed at next input phase callback
	PendingInputList	m_PendingInput ;

	// Used to listen for a before removed event
	//class AgentBeforeDestroyedListener ;
	//AgentBeforeDestroyedListener*	m_pBeforeDestroyedListener ;

	bool m_SuppressRunEndsEvent ;

	// Used to control runs
	bool m_ScheduledToRun ;
	bool m_WasOnRunList;
	bool m_OnStepList;
	//unsigned long	m_InitialStepCount ;
	unsigned long	m_InitialRunCount ;
	smlRunResult	m_ResultOfLastRun ;
    unsigned long	m_localRunCount;
    unsigned long	m_localStepCount;
	smlRunState		m_runState;          // Current agent run state
	unsigned long	m_interruptFlags;    // Flags indicating an interrupt request

	  // Used for update world events
	bool m_CompletedOutputPhase ;
	bool m_GeneratedOutput ;
	unsigned long m_OutputCounter ;

	RhsFunction*	m_pRhsInterrupt ;
	RhsFunction*	m_pRhsConcat ;
	RhsFunction*	m_pRhsExec ;
	RhsFunction*	m_pRhsCmd ;

	AgentRunCallback*	m_pAgentRunCallback ;

	~AgentSML() ;

public:
	AgentSML(KernelSML* pKernelSML, agent* pAgent) ;

	void InitListeners() ;
	void Init() ;

	// Release any objects or other data we are keeping.  We do this just
	// prior to deleting AgentSML, but before the underlying gSKI agent has been deleted
	void Clear(bool deletingThisAgent) ;

	void DeleteSelf() ;

	void RegisterForBeforeAgentDestroyedEvent() ;

	// Release all of the WMEs that we currently have references to
	// It's a little less severe than clear() which releases everything we own, not just wmes.
	// If flushPendingRemoves is true, make sure gSKI removes all wmes from Soar's working memory
	// that have been marked for removal but are still waiting for the next input phase to actually
	// be removed (this should generally be correct so we'll default to true for it).
	void ReleaseAllWmes(bool flushPendingRemoves = true) ;

	// Explicitly reinitialize the agent as part of init-soar
	bool Reinitialize() ;

	agent* GetSoarAgent()	  { return m_agent ; }
	KernelSML* GetKernelSML() { return m_pKernelSML ; }

	char const* GetName() ;

	void SetInputLinkRoot(gSKI::IWMObject* pRoot)   { m_InputLinkRoot = pRoot ; }
	gSKI::IWMObject* GetInputLinkRoot()				{ return m_InputLinkRoot ; }

	void SetOutputLinkRoot(gSKI::IWMObject* pRoot)  { m_OutputLinkRoot = pRoot ; }
	gSKI::IWMObject* GetOutputLinkRoot()			{ return m_OutputLinkRoot ; }

	OutputListener* GetOutputListener()							{ return &m_OutputListener ; }
	InputListener* GetInputListener()							{ return &m_InputListener ; }

	void SetSuppressRunEndsEvent(bool state) { m_SuppressRunEndsEvent = state ; }
	bool GetSuppressRunEndsEvent()			 { return m_SuppressRunEndsEvent ; }

	void AddProductionListener(smlProductionEventId eventID, Connection* pConnection)	{ m_ProductionListener.AddListener(eventID, pConnection) ; }
	void RemoveProductionListener(smlProductionEventId eventID, Connection* pConnection) { m_ProductionListener.RemoveListener(eventID, pConnection) ; }	
	void AddRunListener(smlRunEventId eventID, Connection* pConnection)	{ m_RunListener.AddListener(eventID, pConnection) ; }
	void RemoveRunListener(smlRunEventId eventID, Connection* pConnection) { m_RunListener.RemoveListener(eventID, pConnection) ; }	
	void AddPrintListener(smlPrintEventId eventID, Connection* pConnection)	{ m_PrintListener.AddListener(eventID, pConnection) ; }
	void RemovePrintListener(smlPrintEventId eventID, Connection* pConnection) { m_PrintListener.RemoveListener(eventID, pConnection) ; }	
	void AddXMLListener(smlXMLEventId eventID, Connection* pConnection) { m_XMLListener.AddListener(eventID, pConnection) ; }
	void RemoveXMLListener(smlXMLEventId eventID, Connection* pConnection) { m_XMLListener.RemoveListener(eventID, pConnection) ; }

	// Echo the list of wmes received back to any listeners
	void FireInputReceivedEvent(ElementXML const* pCommands) { m_XMLListener.FireInputReceivedEvent(pCommands) ; }

	void RemoveAllListeners(Connection* pConnection) ;

	/*************************************************************
	* @brief	Sometimes we wish to temporarily disable and then
	*			re-enable the print callback because we use the Kernel's
	*			print callback to report information that isn't really part of the trace.
	*			(One day we should no longer need to do this).
	*			Enabling/disabling affects all connections.
	*************************************************************/
	void DisablePrintCallback() { m_PrintListener.EnablePrintCallback(false) ; m_XMLListener.EnablePrintCallback(false) ; }
	void EnablePrintCallback()  { m_PrintListener.EnablePrintCallback(true) ; m_XMLListener.EnablePrintCallback(true) ; }

	/*************************************************************
	* @brief	Trigger an "echo" event.  This event allows one client
	*			to issue a command and have another client (typically the debugger)
	*			listen in on the output.
	*************************************************************/
	void FireEchoEvent(Connection* pConnection, char const* pMessage) 
	{ 
		m_PrintListener.OnEvent(smlEVENT_ECHO, this, pMessage) ; 
		m_PrintListener.FlushOutput(pConnection, smlEVENT_ECHO) ; 
	}

	/*************************************************************
	* @brief	This is the same as the regular FireEchoEvent, except
	*			it broadcasts to all listeners, even the originator of the command
	*			because the self field is not set in this case.
	*
	*			We shouldn't use this extensively and may choose to
	*			remove this and replace it with a different event later.
	*************************************************************/
	void FireEchoEventIncludingSelf(char const* pMessage) 
	{ 
		m_PrintListener.OnEvent(smlEVENT_ECHO, this, pMessage) ; 
		m_PrintListener.FlushOutput(NULL, smlEVENT_ECHO) ; 
	}

	/*************************************************************
	* @brief	Send XML and print events for a simple string message
	*************************************************************/
	void FireSimpleXML(char const* pMsg) ;

	/*************************************************************
	* @brief	Send a run event
	*************************************************************/
	void FireRunEvent(smlRunEventId eventId) ;

	/*************************************************************
	* @brief	Converts an id from a client side value to a kernel side value.
	*			We need to be able to do this because the client is adding a collection
	*			of wmes at once, so it makes up the ids for those objects.
	*			But the kernel will assign them a different value when the
	*			wme is actually added in the kernel.
	*
	*			RecordIDMapping should be called whenever an identifier is created 
	*			or linked to on the client side because it increments reference counts
	*			to that id so that shared ids work correctly
	*************************************************************/
	bool ConvertID(char const* pClientID, std::string* pKernelID) ;
	void RecordIDMapping(char const* pClientID, char const* pKernelID) ;
	void RemoveID(char const* pKernelID) ;

	/*************************************************************
	* @brief	Converts a time tag from a client side value to
	*			a kernel side one.
	*************************************************************/
	gSKI::IWme* ConvertTimeTag(char const* pTimeTag) ;
	wme* ConvertKernelTimeTag(char const* pTimeTag) ;
	long ConvertTime(char const* pTimeTag) ;

	// Debug method
	void PrintKernelTimeTags() ;

	void RecordTimeTag(char const* pTimeTag, gSKI::IWme* pWme) ;
	void RemoveTimeTag(char const* pTimeTag) ;
	void RecordKernelTimeTag(char const* pTimeTag, wme* pWme) ;
	void RecordTime(char const* pTimeTag, long time) ;
	// Used by callback from gSKI to remove records of a wme (timetag) that 
	// is getting removed due to an object (identifier) deletion
	void RemoveTimeTagByWmeSLOW(gSKI::IWme* pWme) ;	

	void RecordLongTimeTag(long timeTag, gSKI::IWme* pWme) ;
	void RemoveLongTimeTag(long timeTag) ;
	void RemoveKernelTimeTag(char const* pTimeTag) ;

	// Register a RHS function with the Soar kernel
	void RegisterRHSFunction(RhsFunction* pFunction) ;
	void RemoveRHSFunction(RhsFunction* pFunction) ;

	// Utility function (note it's static) for converting a symbol to a string
	static std::string SymbolToString(Symbol* pSymbol) ;

	// Execute a command line function (through the CLI processor)
	std::string ExecuteCommandLine(std::string const& commmandLine) ;

	/*************************************************************
	* @brief	Used to select which agents run on the next run command.
	*************************************************************/
	void ScheduleAgentToRun(bool state) ;
	void RemoveAgentFromRunList()       { m_ScheduledToRun = false ;}
	bool IsAgentScheduledToRun()		{ return m_ScheduledToRun ; }
	void PutAgentOnStepList(bool state) { m_OnStepList = state; }
	bool IsAgentOnStepList()		    { return m_OnStepList ; }
	bool WasAgentOnRunList()            { return m_WasOnRunList ; }

	smlRunResult	GetResultOfLastRun()		  { return m_ResultOfLastRun ; }
	void SetResultOfRun(smlRunResult runResult) { m_ResultOfLastRun = runResult ; }

	//void SetInitialStepCount(unsigned long count)	{ m_InitialStepCount = count ; }
	void SetInitialRunCount(unsigned long count)	{ m_InitialRunCount = count ; }
	//unsigned long GetInitialStepCount()				{ return m_InitialStepCount ; }
	unsigned long GetInitialRunCount()				{ return m_InitialRunCount ; }
	void ResetLocalRunCounters()                    { m_localRunCount = 0 ; m_localStepCount = 0 ; }
	void IncrementLocalRunCounter()                 { m_localRunCount++ ; }
	void IncrementLocalStepCounter()                { m_localStepCount++ ; }
	bool CompletedRunType(unsigned long count)      { return (count > (m_InitialRunCount + m_localRunCount)) ; }
	void SetCompletedOutputPhase(bool state)		{ m_CompletedOutputPhase = state ; }
	bool HasCompletedOutputPhase()					{ return m_CompletedOutputPhase ; }

	void SetGeneratedOutput(bool state)				{ m_GeneratedOutput = state ; }
	bool HasGeneratedOutput() 						{ return m_GeneratedOutput ; }

	void SetInitialOutputCount(unsigned long count)	{ m_OutputCounter = count ; }
	unsigned long GetInitialOutputCount()			{ return m_OutputCounter ; }

	unsigned long GetNumDecisionsExecuted() ;
	unsigned long GetNumDecisionCyclesExecuted() ;
	unsigned long GetNumElaborationsExecuted() ;
	unsigned long GetNumPhasesExecuted() ;
	unsigned long GetNumOutputsGenerated() ;
	unsigned long GetLastOutputCount() ;
	void ResetLastOutputCount() ;
	smlPhase GetCurrentPhase() ;
	AgentRunCallback* GetAgentRunCallback() { return m_pAgentRunCallback ; }

	bool IsSoar7Mode() ;

	unsigned long GetRunCounter(smlRunStepSize runStepSize) ;

	// Request that the agent stop soon.
	bool Interrupt(smlStopLocationFlags stopLoc) ;
	void ClearInterrupts() ;
	smlRunResult StepInClientThread(smlRunStepSize  stepSize) ;
	smlRunResult Step(smlRunStepSize stepSize) ;

	unsigned long GetInterruptFlags()		{ return m_interruptFlags ; }
	smlRunState GetRunState()				{ return m_runState ; }
	void SetRunState(smlRunState state)	{ m_runState = state ; }

protected:
	void InitializeRuntimeState() ;

	/*************************************************************
	* @brief	Add an input message to the pending input list
	*			-- it will be processed on the next input phase callback from the kernel.
	*			This agent takes ownership of the input message by adding a reference to it.
	*************************************************************/
	void AddToPendingInputList(ElementXML_Handle hInputMsgHandle) ;

	PendingInputList*	GetPendingInputList() { return &m_PendingInput ; }
} ;

class AgentRunCallback : public KernelCallback
{
	// This is the actual callback for the event
	virtual void OnKernelEvent(int eventID, AgentSML* pAgentSML, void*)
	{
		if (smlEVENT_AFTER_OUTPUT_PHASE == eventID) 	
		{
			pAgentSML->SetCompletedOutputPhase(true) ;
			// If the number of outputs generated by this agent has changed record it as
			// having generated output and possibly fire a notification event.
			// InitialOutputCount is updated when the OutputGenerated event is fired.
			unsigned long count = pAgentSML->GetNumOutputsGenerated() ;
			if (count != pAgentSML->GetInitialOutputCount())
			{
				pAgentSML->SetGeneratedOutput(true) ;
			}
		}
	}
} ;

}

#endif // SML_AGENT_SML_H
