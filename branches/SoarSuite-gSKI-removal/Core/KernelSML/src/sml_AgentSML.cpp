#include <portability.h>

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

#include "sml_Utils.h"
#include "sml_AgentSML.h"
#include "sml_OutputListener.h"
#include "sml_StringOps.h"
#include "sml_KernelSML.h"
#include "sml_RhsFunction.h"

#include "gSKI_Events.h"
#include "gSKI_Structures.h"
#include "IgSKI_Wme.h"
#include "gSKI_Agent.h"
#include "IgSKI_InputProducer.h"
#include "IgSKI_WMObject.h"
#include "IgSKI_Symbol.h"
#include "gSKI_AgentManager.h"
#include "gSKI_Kernel.h"
#include "IgSKI_InputLink.h"
#include "IgSKI_WorkingMemory.h"
#include "gSKI_EnumRemapping.h"

#include "KernelHeaders.h"

#ifdef _DEBUG
// Comment this in to debug init-soar and inputwme::update calls
//#define DEBUG_UPDATE
#endif

#include <assert.h>

using namespace sml ;

// A set of helper functions for tracing kernel wmes
static void Symbol2String(Symbol* pSymbol, 	bool refCounts, std::ostringstream& buffer) {
	if (pSymbol->common.symbol_type==IDENTIFIER_SYMBOL_TYPE) {
		buffer << pSymbol->id.name_letter ;
		buffer << pSymbol->id.name_number ;
	}
	else if (pSymbol->common.symbol_type==VARIABLE_SYMBOL_TYPE) {
		buffer << pSymbol->var.name ;
	}
	else if (pSymbol->common.symbol_type==SYM_CONSTANT_SYMBOL_TYPE) {
		buffer << pSymbol->sc.name ;
	}
	else if (pSymbol->common.symbol_type==INT_CONSTANT_SYMBOL_TYPE) {
		buffer << pSymbol->ic.value ;
	}
	else if (pSymbol->common.symbol_type==FLOAT_CONSTANT_SYMBOL_TYPE) {
		buffer << pSymbol->fc.value ;
	}

	if (refCounts)
		buffer << "[" << pSymbol->common.reference_count << "]" ;
}

static std::string Wme2String(wme* pWME, bool refCounts) {
	std::ostringstream buffer ;

	buffer << pWME->timetag << ":" ;

	Symbol2String(pWME->id, refCounts, buffer) ;
	buffer << " ^" ;
	Symbol2String(pWME->attr, refCounts, buffer) ;
	buffer << " " ;
	Symbol2String(pWME->value, refCounts, buffer) ;

	buffer << " 0x" << (uintptr_t)pWME ;

	return buffer.str() ;
}

static void PrintDebugWme(char const* pMsg, wme* pWME, bool refCounts = false) {
	std::string str = Wme2String(pWME, refCounts) ;
	PrintDebugFormat("%s %s", pMsg, str.c_str()) ;
}

AgentSML::AgentSML(KernelSML* pKernelSML, agent* pAgent)
{
	m_pKernelSML = pKernelSML ;
	m_pIAgent = NULL ;
	m_pInputProducer = NULL ;
	m_InputLinkRoot = NULL ;
	m_OutputLinkRoot = NULL ;
	m_SuppressRunEndsEvent = false ;
	m_pBeforeDestroyedListener = NULL ;

	m_pAgentRunCallback = new AgentRunCallback() ;
	m_pAgentRunCallback->SetAgentSML(this) ;

	m_agent = pAgent ;
}

void AgentSML::InitListeners()
{
	KernelSML* pKernelSML = m_pKernelSML ;

	m_PrintListener.Init(pKernelSML, this) ;
	m_XMLListener.Init(pKernelSML, this) ;
	m_RunListener.Init(pKernelSML, this) ;
	m_ProductionListener.Init(pKernelSML, this) ;
	m_OutputListener.Init(pKernelSML, this) ;
	m_InputListener.Init(pKernelSML, this) ;

	// For KernelSML (us) to work correctly we need to listen for certain events, independently of what any client is interested in
	// Currently:
	// Listen for output callback events so we can send this output over to the clients
	// Listen for "before" init-soar events (we need to know when these happen so we can release all WMEs on the input link, otherwise gSKI will fail to re-init the kernel correctly.)
	// Listen for "after" init-soar events (we need to know when these happen so we can resend the output link over to the client)
	m_OutputListener.RegisterForKernelSMLEvents() ;
	m_InputListener.RegisterForKernelSMLEvents() ;
}

// Can't call this until after the Soar agent has been initialized
void AgentSML::Init()
{
	m_pRhsInterrupt = new InterruptRhsFunction(this) ;
	m_pRhsConcat    = new ConcatRhsFunction(this) ;
	m_pRhsExec		= new ExecRhsFunction(this) ;
	m_pRhsCmd		= new CmdRhsFunction(this) ;
	RegisterRHSFunction(m_pRhsInterrupt) ;
	RegisterRHSFunction(m_pRhsConcat) ;
	RegisterRHSFunction(m_pRhsExec) ;
	RegisterRHSFunction(m_pRhsCmd) ;

	// Set counters and flags used to control runs
	InitializeRuntimeState() ;
}

class AgentSML::AgentBeforeDestroyedListener: public gSKI::IAgentListener
{
public:
	// This handler is called right before the agent is actually deleted
	// inside gSKI.  We need to clean up any object we own now.
	virtual void HandleEvent(egSKIAgentEventId, gSKI::Agent* pAgent)
	{
#ifdef DEBUG_UPDATE
	PrintDebugFormat("AgentSML::AgentBeforeDestroyedListener start.") ;
#endif
		KernelSML* pKernelSML = KernelSML::GetKernelSML() ;

		// Unregister ourselves (this is important for the same reasons as listed above)
		pKernelSML->GetKernel()->GetAgentManager()->RemoveAgentListener(gSKIEVENT_BEFORE_AGENT_DESTROYED, this) ;

		// Release any wmes or other objects we're keeping
		AgentSML* pAgentSML = pKernelSML->GetAgentSML(pAgent) ;
		pAgentSML->DeleteSelf() ;
		pAgentSML = NULL ;	// At this point the pointer is invalid so clear it.

#ifdef DEBUG_UPDATE
	PrintDebugFormat("AgentSML::AgentBeforeDestroyedListener end.") ;
#endif
	}
};

AgentSML::~AgentSML()
{
	// Release any objects we still own
	Clear(true) ;

	delete m_pAgentRunCallback ;
	delete m_pInputProducer ;
	delete m_pBeforeDestroyedListener ;
}

// Release any objects or other data we are keeping.  We do this just
// prior to deleting AgentSML, but before the underlying gSKI agent has been deleted
// 'deletingThisAgent' should only be true when we're actually in the destructor.
void AgentSML::Clear(bool deletingThisAgent)
{
#ifdef DEBUG_UPDATE
	sml::PrintDebugFormat("AgentSML::Clear start %s", deletingThisAgent ? "deleting this agent." : "not deleting this agent.") ;
#endif

	// Release any WME objects we still own.
	// (Don't flush removes in this case as we're shutting down rather than just doing an init-soar).
	ReleaseAllWmes(!deletingThisAgent) ;

	RemoveRHSFunction(m_pRhsInterrupt) ; delete m_pRhsInterrupt ; m_pRhsInterrupt = NULL ;
	RemoveRHSFunction(m_pRhsConcat)    ; delete m_pRhsConcat    ; m_pRhsConcat = NULL ;
	RemoveRHSFunction(m_pRhsExec)      ; delete m_pRhsExec      ; m_pRhsExec = NULL ;
	RemoveRHSFunction(m_pRhsCmd)       ; delete m_pRhsCmd       ; m_pRhsCmd = NULL ;

	m_ProductionListener.Clear();
	m_RunListener.Clear();
	m_PrintListener.Clear();
	m_OutputListener.Clear() ;
	m_XMLListener.Clear() ;

#ifdef DEBUG_UPDATE
	sml::PrintDebugFormat("AgentSML::Clear end %s", deletingThisAgent ? "deleting this agent." : "not deleting this agent.") ;
#endif
}

// Release all of the WMEs that we currently have references to
// It's a little less severe than clear() which releases everything we own, not just wmes.
// If flushPendingRemoves is true, make sure gSKI removes all wmes from Soar's working memory
// that have been marked for removal but are still waiting for the next input phase to actually
// be removed (this should generally be correct so we'll default to true for it).
void AgentSML::ReleaseAllWmes(bool flushPendingRemoves)
{
#ifdef DEBUG_UPDATE
	sml::PrintDebugFormat("****************************************************") ;
	sml::PrintDebugFormat("%s AgentSML::ReleaseAllWmes start %s", this->GetIAgent()->GetName(), flushPendingRemoves ? "flush pending removes." : "do not flush pending removes.") ;
	sml::PrintDebugFormat("****************************************************") ;
#endif

	if (flushPendingRemoves)
	{
		bool forceAdds = false ;	// It doesn't matter if we do these or not as we're about to release everything.  Seems best to not start things up.
		bool forceRemoves = true ;	// SML may have deleted a wme but gSKI has yet to act on this.  As SML has removed its object we have no way to free the gSKI object w/o doing this update.
		this->GetIAgent()->GetInputLink()->GetInputLinkMemory()->Update(forceAdds, forceRemoves) ;
	}

	// Release any WME objects we still own.
	for (TimeTagMapIter mapIter = m_TimeTagMap.begin() ; mapIter != m_TimeTagMap.end() ; mapIter++)
	{
		gSKI::IWme* pWme = mapIter->second ;
		std::string value = pWme->GetValue()->GetString() ;

		bool deleted = pWme->Release() ;

		if (!deleted)
		{
			// Can put a break point here to see if any wmes aren't being deleted when they are released.
			// Unforunately, we can't assert or do more because that can be valid (others places might validly have a reference to pWme).
			int x = 1 ;
			unused(x) ;
		}
	}

	//PrintDebugFormat("About to release kernel wmes") ;
	PrintKernelTimeTags() ;

	for (KernelTimeTagMapIter mapIter = m_KernelTimeTagMap.begin() ; mapIter != m_KernelTimeTagMap.end() ; mapIter++) {
		wme* wme = mapIter->second ;
		PrintDebugWme("Releasing ", wme, true) ;
		release_io_symbol(this->GetAgent(), wme->id) ;
		release_io_symbol(this->GetAgent(), wme->attr) ;
		release_io_symbol(this->GetAgent(), wme->value) ;
	}

	if (m_InputLinkRoot)
	{
		m_InputLinkRoot->Release() ;
	}
	m_InputLinkRoot = NULL ;

	for (PendingInputListIter iter = m_PendingInput.begin() ; iter != m_PendingInput.end() ; iter++)
	{
		ElementXML* pMsg = *iter ;
		delete pMsg ;
	}

	m_TimeTagMap.clear() ;
	m_PendingInput.clear() ;
	m_KernelTimeTagMap.clear() ;
	m_ToClientIdentifierMap.clear() ;
	m_IdentifierMap.clear() ;
	m_IdentifierRefMap.clear() ;

#ifdef DEBUG_UPDATE
	sml::PrintDebugFormat("****************************************************") ;
	sml::PrintDebugFormat("%s AgentSML::ReleaseAllWmes end %s", this->GetIAgent()->GetName(), flushPendingRemoves ? "flush pending removes." : "do not flush pending removes.") ;
	sml::PrintDebugFormat("****************************************************") ;
#endif
}

char const* AgentSML::GetName()
{
	return m_agent->name ;
}

void AgentSML::InitializeRuntimeState()
{
	m_WasOnRunList = false;
	m_ScheduledToRun = false ;
	m_OnStepList = false;
	m_ResultOfLastRun = sml_RUN_COMPLETED ;
	m_InitialRunCount = 0 ;
	m_CompletedOutputPhase = false ;
	m_GeneratedOutput = false ;
	m_OutputCounter = 0 ;
    m_localRunCount = 0 ;
    m_localStepCount= 0 ;
    m_runState = sml_RUNSTATE_STOPPED;
    m_interruptFlags = 0 ;
}


bool AgentSML::Reinitialize()
{
	m_pKernelSML->FireAgentEvent(this, smlEVENT_BEFORE_AGENT_REINITIALIZED) ;
	bool ok = GetIAgent()->Reinitialize();
	InitializeRuntimeState() ;
	m_pKernelSML->FireAgentEvent(this, smlEVENT_AFTER_AGENT_REINITIALIZED) ;
	return ok ;
}

void AgentSML::RemoveAllListeners(Connection* pConnection)
{
	m_ProductionListener.RemoveAllListeners(pConnection);
	m_RunListener.RemoveAllListeners(pConnection);
	m_PrintListener.RemoveAllListeners(pConnection);
	m_OutputListener.RemoveAllListeners(pConnection) ; 
	m_XMLListener.RemoveAllListeners(pConnection) ;
}

/*************************************************************
* @brief	Add an input message to the pending input list
*			-- it will be processed on the next input phase callback from the kernel.
*************************************************************/
void AgentSML::AddToPendingInputList(ElementXML_Handle hInputMsgHandle)
{
	// Create a new wrapper object for the message and store that with
	// an increased ref count so the caller can do whatever they want with the original message
	ElementXML* pMsg = new ElementXML(hInputMsgHandle) ;
	pMsg->AddRefOnHandle() ;

	m_PendingInput.push_back(pMsg) ;
}

//=============================
//Number of complete decision cycles (through to end of output)
//=============================
unsigned long AgentSML::GetNumDecisionCyclesExecuted()
{
  return m_agent->d_cycle_count;
}

//=============================
//Number of decisions (operator selections/impasses) instead of full D_cycles
//=============================
unsigned long AgentSML::GetNumDecisionsExecuted()
{
  return m_agent->decision_phases_count;
}

//=============================
//Number of phases since last init-soar
//=============================
unsigned long AgentSML::GetNumPhasesExecuted()
{
	return m_agent->run_phase_count;
}

//=============================
// Number of elaborations (or completed phases w/o rules firing) since last init-soar
// This definitions of elaborations matches with "run 1 -e".
// If you want to know how many sets of rules have fired you need a different counter.
//=============================
unsigned long AgentSML::GetNumElaborationsExecuted()
{
	return m_agent->run_elaboration_count ;
}

//=============================
//Number of outputs generated by this agent (or times reaching "max-nil-outputs" limit)
//=============================
unsigned long AgentSML::GetNumOutputsGenerated()
{
	return m_agent->run_generated_output_count ;
}

//=============================
//Number of output phases since this agent last generated output
//=============================
unsigned long AgentSML::GetLastOutputCount()
{
	return m_agent->run_last_output_count ;
}

//=============================
// Reset the count of how long since the agent last generated output
//=============================
void AgentSML::ResetLastOutputCount()
{
	m_agent->run_last_output_count = 0 ;
}

//=============================
// Special compatability mode to use Soar 7 decision cycle
// Intended to allow easier upgrade from Soar 7 to Soar 8.
//=============================
bool AgentSML::IsSoar7Mode()
{
	// This mode signals Soar 8
	if (m_agent->operand2_mode)
		return false ;

	return true ;
}

//=============================
// Returns the current phase (which generally means the phase that is next going to execute
// if you inspect this in between runs)
//=============================
smlPhase AgentSML::GetCurrentPhase()
{

	return (smlPhase)gSKI::EnumRemappings::ReMapPhaseType((unsigned short)m_agent->current_phase,0);
}

unsigned long AgentSML::GetRunCounter(smlRunStepSize runStepSize)
{
	switch(runStepSize)
	{
	case gSKI_RUN_PHASE:
		{
			unsigned long phases = GetNumPhasesExecuted() ;
			return phases ;
		}
	case gSKI_RUN_ELABORATION_CYCLE:
		{
			unsigned long elabs = GetNumElaborationsExecuted() ;
			return elabs ;
		}
	case gSKI_RUN_DECISION_CYCLE:
		{
			unsigned long decs = GetNumDecisionCyclesExecuted() ;
			return decs ;
		}
	case gSKI_RUN_UNTIL_OUTPUT:
		{
			unsigned long outputs = GetNumOutputsGenerated() ;
			return outputs ;
		}
	default:
		return 0;
	}
}

//=============================
// Request that the agent stop soon.
//=============================
bool AgentSML::Interrupt(smlStopLocationFlags stopLoc)
{
  // Tell the agent where to stop
  m_interruptFlags = stopLoc;

  // If the request for interrupt is gSKI_STOP_AFTER_DECISION_CYCLE, then it 
  // will be caught in the RunScheduler::CompletedRunType() and/or IsAgentFinished().
  // We don't want to interrupt agents until the appropriate time if the request is  
  // gSKI_STOP_AFTER_DECISION_CYCLE.
  
  // These are immediate requests for interrupt, such as from RHS or application
  if ((sml_STOP_AFTER_SMALLEST_STEP == stopLoc) || (sml_STOP_AFTER_PHASE == stopLoc)) {
	  m_agent->stop_soar = TRUE;
	  // If the agent is not running, we should set the runState flag now so agent won't run
	  if (m_runState == sml_RUNSTATE_STOPPED)
	  {
		  m_runState = sml_RUNSTATE_INTERRUPTED;
	  }
	  // Running agents must test stopLoc & stop_soar in Step method to see if interrupted.
	  // Because we set m_agent->stop_soar == TRUE above, any running agents should return to
	  // gSKI at the end of the current phase, even if interleaving by larger steps.  KJC
  }

  // If  we implement suspend, it goes in the run method, not
  //  here.
  //m_suspendOnInterrupt = (stopType == gSKI_STOP_BY_SUSPENDING)? true: false;

  return true;
}

//=============================
// Clear any existing interrupt requests
// (Generally done before starting a run)
//=============================
void AgentSML::ClearInterrupts()
{
  // Clear the interrupts whether running or not
  m_interruptFlags = 0;

  // Only change state of agent if it is running
  if(m_runState == sml_RUNSTATE_INTERRUPTED)
  {
    // We returned, and thus are stopped
    m_runState = sml_RUNSTATE_STOPPED;
  }
}

smlRunResult AgentSML::StepInClientThread(smlRunStepSize  stepSize, gSKI::Error* pError)
{
  // Agent is already running, we cannot run
  if(m_runState != sml_RUNSTATE_STOPPED)
  {
     if(m_runState == sml_RUNSTATE_HALTED)
		 SetError(pError, gSKI::gSKIERR_AGENT_HALTED);  // nothing ever tests for this...
     else
		 SetError(pError, gSKI::gSKIERR_AGENT_RUNNING);

     return sml_RUN_ERROR;
  }

  m_runState = sml_RUNSTATE_RUNNING;

  // Now clear error and do the run
  ClearError(pError);

  // This method does all the work
  return Step(stepSize);
}

void AgentSML::FireRunEvent(smlRunEventId eventId) {
	// Trigger a callback from the kernel to propagate the event out to listeners.
	// This allows us to use a single uniform model for all run events (even when some are really originating here in SML).
	int callbackEvent = KernelCallback::GetCallbackFromEventID(eventId) ;
	soar_invoke_callbacks(m_agent, m_agent, (SOAR_CALLBACK_TYPE)callbackEvent,(soar_call_data) m_agent->current_phase);
}

void AgentSML::FireSimpleXML(char const* pMsg)
{
   GetIAgent()->FirePrintEvent(gSKIEVENT_PRINT, pMsg) ;
   GetIAgent()->FireXMLEvent(gSKIEVENT_XML_TRACE_OUTPUT, sml_Names::kFunctionBeginTag, sml_Names::kTagMessage, 0) ; 
   GetIAgent()->FireXMLEvent(gSKIEVENT_XML_TRACE_OUTPUT, sml_Names::kFunctionAddAttribute, sml_Names::kTypeString, pMsg) ; 
   GetIAgent()->FireXMLEvent(gSKIEVENT_XML_TRACE_OUTPUT, sml_Names::kFunctionEndTag, sml_Names::kTagMessage, 0) ; 
}

static bool maxStepsReached(unsigned long steps, unsigned long maxSteps)
{
	return (steps >= maxSteps);
}

smlRunResult AgentSML::Step(smlRunStepSize stepSize)
{    
   // NOTE: This only works because they have the same ordering
   // BADBAD: Eventually we should dispose of one of these types and fold them into a single enum
   smlRunStepSize runStepSize = (smlRunStepSize)stepSize ;

   // This method runs a single agent
   unsigned long count = 1 ;
   unsigned long startCount        = GetRunCounter(runStepSize) ; // getReleventCounter(stepSize);
   const unsigned long  END_COUNT  = startCount + count ;

   bool interrupted  = (m_runState == sml_RUNSTATE_INTERRUPTED)? true: false;

   if (! interrupted) {
	   assert(!m_agent->system_halted) ; // , "System should not be halted here!");
	   // Notify that agent is about to execute. (NOT the start of a run, just a step)
	   FireRunEvent(smlEVENT_BEFORE_RUNNING) ;
	   //RunNotifier nfBeforeRunning(this,EnumRemappings::ReMapPhaseType(m_agent->current_phase,0));
	   //m_runListeners.Notify(gSKIEVENT_BEFORE_RUNNING, nfBeforeRunning);

	   switch (stepSize) 
	   {
	   case  sml_ELABORATION:	run_for_n_elaboration_cycles(m_agent, count); break;
	   case  sml_PHASE:         run_for_n_phases(m_agent, count);             break;
	   case  sml_DECISION:		run_for_n_decision_cycles(m_agent, count);    break;
	   case  sml_UNTIL_OUTPUT:  run_for_n_modifications_of_output(m_agent, count); break;
	   }
   }

   if ((m_interruptFlags & gSKI_STOP_AFTER_SMALLEST_STEP) || 
	   (m_interruptFlags & gSKI_STOP_AFTER_PHASE))
   {
	   interrupted = true;
   }

   // KJC: If a gSKI_STOP_AFTER_DECISION_CYCLE has been requested, need to
   // check that agent phase is at the proper stopping point before interrupting.
   // If not at the right phase, but interrupt was requested, then the SML scheduler
   // method IsAgentFinished will return true and MoveTo_StopBeforePhase will
   // step the agent by phases until this test is satisfied.
   if ((m_interruptFlags & gSKI_STOP_AFTER_DECISION_CYCLE) && 
	   (m_agent->current_phase == m_pKernelSML->GetStopBefore()))
   {
	   interrupted = true;
   }

   		   
   if (interrupted) 
   {
       // Notify of the interrupt
	   FireRunEvent(smlEVENT_AFTER_INTERRUPT) ;

	   /* This is probably redundant with the event above, which clients can listen for... */
	   FireSimpleXML("Interrupt received.") ;
   }
 
   smlRunResult retVal;

   // We've exited the run loop so we see what we should return
   if(m_agent->system_halted)
   {
	   // if the agent halted because it is in an infinite loop of no-change impasses
	   // interrupt the agents and allow the user to try to recover.
	   if ((long)m_agent->bottom_goal->id.level >=  m_agent->sysparams[MAX_GOAL_DEPTH])
	   {// the agent halted because it seems to be in an infinite loop, so throw interrupt
		   m_pKernelSML->InterruptAllAgents(sml_STOP_AFTER_PHASE) ;
		   m_agent->system_halted = FALSE; // hack! otherwise won't run again.  
		   m_runState = sml_RUNSTATE_INTERRUPTED;
		   retVal     = sml_RUN_INTERRUPTED;
		   // Notify of the interrupt

		   FireRunEvent(smlEVENT_AFTER_INTERRUPT) ;

		   /* This is probably redundant with the event above, which clients can listen for... */
		   FireSimpleXML("Interrupt received.") ;
	   }
	   else {
	   // If we halted, we completed and our state is halted
	   m_runState    = sml_RUNSTATE_HALTED;
	   retVal        = sml_RUN_COMPLETED;

	   FireRunEvent(smlEVENT_AFTER_HALTED) ;

	   // fix for BUG 514  01-12-06
	   FireSimpleXML("This Agent halted.") ;
	   }
   }
   else if(maxStepsReached(GetRunCounter(runStepSize), END_COUNT)) 
   {
	   if(interrupted)
	   {
		   m_runState = sml_RUNSTATE_INTERRUPTED;
		   retVal     = sml_RUN_COMPLETED_AND_INTERRUPTED; 
		   //retVal     = sml_RUN_INTERRUPTED;
	   }
	   else
	   {
		   m_runState = sml_RUNSTATE_STOPPED;
		   retVal     = sml_RUN_COMPLETED;
	   }
   }
   else 
   {  
	   // We were interrupted before we could complete
	   assert(interrupted) ; //, "Should never get here if we aren't interrupted");

	   m_runState    = sml_RUNSTATE_INTERRUPTED;
	   retVal        = sml_RUN_INTERRUPTED;
   }    

   // Notify that agent stopped. (NOT the end of a run, just a step)
   // Use AFTER_RUN_ENDS if you want to trap the end of the complete run.
   FireRunEvent(smlEVENT_AFTER_RUNNING) ;

   return retVal;
}

void AgentSML::DeleteSelf()
{
	this->Clear(false) ;

	// Remove the listeners that KernelSML uses for this agent.
	// This is important.  Otherwise if we create a new agent using the same kernel object
	// the listener will still exist inside gSKI and will crash when an agent event is next generated.
	this->GetOutputListener()->UnRegisterForKernelSMLEvents() ;

	this->GetInputListener()->UnRegisterForKernelSMLEvents() ;

	// Unregister ourselves (this is important for the same reasons as listed above)
	//m_pKernelSML->GetKernel()->GetAgentManager()->RemoveAgentListener(gSKIEVENT_BEFORE_AGENT_DESTROYED, this) ;

	// Then delete our matching agent sml information
	m_pKernelSML->DeleteAgentSML(this->GetIAgent()) ;

	// Do self clean-up of this object as it's just called
	// prior to deleting the AgentSML structure.
	delete this ;
}

void AgentSML::RegisterForBeforeAgentDestroyedEvent()
{
	// We should do this immediately before we delete the agent.
	// We shouldn't do it earlier or we can't be sure it'll be last on the list of listeners which is where we
	// need it to be (so that we clear our information about the gSKI agent *after* we've notified any of our listeners
	// about this event).
	m_pBeforeDestroyedListener = new AgentBeforeDestroyedListener() ;
	m_pKernelSML->GetKernel()->GetAgentManager()->AddAgentListener(gSKIEVENT_BEFORE_AGENT_DESTROYED, m_pBeforeDestroyedListener) ;
}

void AgentSML::ScheduleAgentToRun(bool state) 
{ 
	if (this->GetRunState() != sml_RUNSTATE_HALTED) 
	{
		m_ScheduledToRun = state ; 
		m_WasOnRunList = state; 
	}
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
	// Do we already have a mapping?
	IdentifierMapIter iter = m_IdentifierMap.find(pClientID);

	if (iter == m_IdentifierMap.end())
	{
		// We don't, create a mapping, this indicates a reference count of 1
	m_IdentifierMap[pClientID] = pKernelID ;

	// Record in both directions, so we can clean up (at which time we only know the kernel side ID).
	m_ToClientIdentifierMap[pKernelID] = pClientID ;

		// Note that we leave the entry out of m_IdentifierRefMap, we only use
		// that for counts of two or greater
}
	else
	{
		// The mapping already exists, so we need to check and see if we have a reference
		// count for it yet
		IdentifierRefMapIter iter = m_IdentifierRefMap.find(pClientID);
		if (iter == m_IdentifierRefMap.end())
		{
			// there is no reference count and this is the second reference, so set it to two
			m_IdentifierRefMap[pClientID] = 2 ;
		}
		else 
		{
			// there is a reference count, increment it
			iter->second += 1;
		}
	}
}

void AgentSML::RemoveID(char const* pKernelID)
{
	// first, find the identifer
	IdentifierMapIter iter = m_ToClientIdentifierMap.find(pKernelID) ;

	// This identifier should have been in the table
	// Note: sometimes this is called by gSKI when it is removing wmes. gSKI doesn't know if
	// we're a direct connection and therefore aren't using this map, so in that case we need to not
	// update this. Therefore, we can't assert here.
	//assert (iter != m_ToClientIdentifierMap.end()) ;
	if (iter == m_ToClientIdentifierMap.end())
		return ;

	// cache the identifer value
	std::string& clientID = iter->second ;

	// decrement the reference count and remove the identifier from the maps if it is there
	IdentifierRefMapIter refIter = m_IdentifierRefMap.find(clientID);
	if (refIter == m_IdentifierRefMap.end())
	{
		// when we have an entry in the m_IdentifierMap but not m_IdentifierRefMap, this 
		// means our ref count is one, so we're decrementing to zero, so we remove it
	m_IdentifierMap.erase(clientID) ;
	m_ToClientIdentifierMap.erase(pKernelID) ;
		return;
}
	else 
	{
		// if we have an entry, decrement it
		refIter->second -= 1;

		// if the count falls to 1, remove it from this map since presence in the map requires 
		// at least a ref count of two
		if (refIter->second < 2) {
			m_IdentifierRefMap.erase(refIter);
		}
	}
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

long AgentSML::ConvertTime(char const* pTimeTag)
{
	if (pTimeTag == NULL)
		return 0 ;

	TimeMapIter iter = m_TimeMap.find(pTimeTag) ;

	if (iter == m_TimeMap.end())
	{
		return 0 ;
	}
	else
	{
		// If we found a mapping, return the mapped value
		return iter->second ;
	}
}

/*************************************************************
* @brief	Converts a time tag from a client side value to
*			a kernel side object
*************************************************************/
wme* AgentSML::ConvertKernelTimeTag(char const* pTimeTag)
{
	if (pTimeTag == NULL)
		return NULL ;

	KernelTimeTagMapIter iter = m_KernelTimeTagMap.find(pTimeTag) ;

	if (iter == m_KernelTimeTagMap.end())
	{
		return NULL ;
	}
	else
	{
		// If we found a mapping, return the mapped value
		wme* result = iter->second ;
		return result ;
	}
}

// Debug method
void AgentSML::PrintKernelTimeTags()
{
	for (KernelTimeTagMapIter mapIter = m_KernelTimeTagMap.begin() ; mapIter != m_KernelTimeTagMap.end() ; mapIter++) {
		wme* wme = mapIter->second ;
		PrintDebugWme("Recorded wme ", wme, true) ;
	}
}

/*************************************************************
* @brief	Maps from a client side time tag to a kernel side WME.
*************************************************************/
void AgentSML::RecordTimeTag(char const* pTimeTag, gSKI::IWme* pWME)
{
#ifdef _DEBUG
	// I believe it correct that a time tag should never be re-used in this context
	// so I'm including this assert.  However, it's possible this assumption is wrong (in particular after an init-soar?)
	// so I'm only including it in debug builds and if the assert fails, check the context and make sure that this re-use
	// in indeed a mistake.
	// If you fail to call commit() after creating a new input wme and then issue an init-soar this assert may fire.
	// If so, the fix is to call commit().
	assert (m_TimeTagMap.find(pTimeTag) == m_TimeTagMap.end()) ;
#endif

	m_TimeTagMap[pTimeTag] = pWME ;
}

void AgentSML::RecordTime(char const* pTimeTag, long time)
{
	m_TimeMap[pTimeTag] = time ;
}

void AgentSML::RecordKernelTimeTag(char const* pTimeTag, wme* pWme)
{
#ifdef _DEBUG
	// I believe it correct that a time tag should never be re-used in this context
	// so I'm including this assert.  However, it's possible this assumption is wrong (in particular after an init-soar?)
	// so I'm only including it in debug builds and if the assert fails, check the context and make sure that this re-use
	// in indeed a mistake.
	// If you fail to call commit() after creating a new input wme and then issue an init-soar this assert may fire.
	// If so, the fix is to call commit().
	assert (m_KernelTimeTagMap.find(pTimeTag) == m_KernelTimeTagMap.end()) ;
#endif

	PrintDebugWme("Recording wme ", pWme, true) ;
	m_KernelTimeTagMap[pTimeTag] = pWme ;
}

void AgentSML::RecordLongTimeTag(long timeTag, gSKI::IWme* pWME)
{
	// Make sure it's a valid time tag
	assert(timeTag != 0) ;

	char str[kMinBufferSize] ;
	Int2String(timeTag, str, sizeof(str)) ;

	RecordTimeTag(str, pWME) ;
}

void AgentSML::RemoveTimeTag(char const* pTimeTag)
{
	m_TimeTagMap.erase(pTimeTag) ;
}

void AgentSML::RemoveTimeTagByWmeSLOW(gSKI::IWme* pWme)
{
	// This is used by KernelSML::RemoveInputWMERecords to remove a timetag
	// when gSKI is removing wmes from the kernel due to an identifier deletion.

	// It's slow because it is linear with respect to the number of wmes.
	// TODO: There should be a two-way mapping.
	for ( TimeTagMapIter iter = m_TimeTagMap.begin(); iter != m_TimeTagMap.end(); ++iter )
	{
		if ( iter->second == pWme )
		{
			m_TimeTagMap.erase( iter );
			return;
		}
	}
}

void AgentSML::RemoveLongTimeTag(long timeTag)
{
	char str[kMinBufferSize] ;
	Int2String(timeTag, str, sizeof(str)) ;

	m_TimeTagMap.erase(str) ;
}

void AgentSML::RemoveKernelTimeTag(char const* pTimeTag)
{
	m_KernelTimeTagMap.erase(pTimeTag) ;
}

void AgentSML::RegisterRHSFunction(RhsFunction* rhsFunction)
{
	// Tell Soar about it
	add_rhs_function (m_agent, 
					make_sym_constant(m_agent, rhsFunction->GetName()),
					RhsFunction::RhsFunctionCallback,
					rhsFunction->GetNumExpectedParameters(),
					rhsFunction->IsValueReturned(),
					true,
					static_cast<void*>(rhsFunction));
}

void AgentSML::RemoveRHSFunction(RhsFunction* rhsFunction)
{
	if (rhsFunction == NULL)
		return ;

	char const* szName = rhsFunction->GetName() ;

	// Tell the kernel we are done listening.
	//RPM 9/06: removed symbol ref so symbol is released properly
	Symbol* tmp = make_sym_constant(m_agent, szName);
	remove_rhs_function(m_agent, tmp);
	symbol_remove_ref (m_agent, tmp);
}

std::string AgentSML::SymbolToString(Symbol* sym)
{
	if ( sym == 0 ) return "";

	// No buffer overrun problems but resulting string may be 
	// truncated.
	char temp[128];

	switch (sym->common.symbol_type) {
	case VARIABLE_SYMBOL_TYPE:
	 return (char*) (sym->var.name);
	 break;
	case IDENTIFIER_SYMBOL_TYPE:
	 SNPRINTF(temp,128,"%c%lu",sym->id.name_letter,sym->id.name_number);
	 return std::string(temp);
	 break;
	case SYM_CONSTANT_SYMBOL_TYPE:
	 return (char *) (sym->sc.name);
	 break;
	case INT_CONSTANT_SYMBOL_TYPE:
	 SNPRINTF(temp,128, "%ld",sym->ic.value);
	 return std::string(temp);
	 break;
	case FLOAT_CONSTANT_SYMBOL_TYPE:
	 SNPRINTF(temp, 128, "%g",sym->fc.value);
	 return std::string(temp);
	 break;
	default:
	 assert( false ) ; // "This symbol type not supported";
	 return "";
	 break;
	}
}

std::string AgentSML::ExecuteCommandLine(std::string const& commandLine)
{
	KernelSML* pKernel = m_pKernelSML ;

	// We'll pretend this came from the local (embedded) connection.
	Connection* pConnection = pKernel->GetEmbeddedConnection() ;

	// Build up a message to execute the command line
	bool rawOutput = true ;
	ElementXML* pMsg = pConnection->CreateSMLCommand(sml_Names::kCommand_CommandLine, rawOutput) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamAgent, this->GetName());
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamLine, commandLine.c_str()) ;

	AnalyzeXML incoming ;
	incoming.Analyze(pMsg) ;

	// Create a response object which the command line can fill in
	ElementXML* pResponse = pConnection->CreateSMLResponse(pMsg) ;

	// Execute the command line
	bool ok = pKernel->ProcessCommand(sml_Names::kCommand_CommandLine, pConnection, &incoming, pResponse) ;

	std::string result ;

	if (ok)
	{
		// Take the result from executing the command line and fill it in to our "pReturnValue" array.
		AnalyzeXML response ;
		response.Analyze(pResponse) ;

		char const* pRes = response.GetResultString() ;

		if (pRes)
			result = pRes ;
	}
	else
	{
		result = std::string("Error executing command ") + commandLine ;
	}

	// Clean up
	delete pMsg ;
	delete pResponse ;

	return result ;
}
