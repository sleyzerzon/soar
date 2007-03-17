#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
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

#ifdef DEBUG_UPDATE
#include "sock_Debug.h"	// For PrintDebugFormat
#endif

#ifdef _WIN32
#define safeSprintf _snprintf
#else
#define safeSprintf snprintf
#endif

#include <assert.h>

using namespace sml ;

AgentSML::AgentSML(KernelSML* pKernelSML, gSKI::Agent* pIAgent, agent* pAgent)
: m_ProductionListener(pKernelSML, pIAgent), m_RunListener(pKernelSML, pIAgent),
  m_PrintListener(pKernelSML, pIAgent), m_XMLListener(pKernelSML, pIAgent)
{
	m_pKernelSML = pKernelSML ;
	m_pIAgent = pIAgent ;
	m_pInputProducer = NULL ;
	m_InputLinkRoot = NULL ;
	m_OutputLinkRoot = NULL ;
	m_SuppressRunEndsEvent = false ;

	m_agent = pAgent ;

	m_pRhsInterrupt = new InterruptRhsFunction(this) ;
	m_pRhsConcat    = new ConcatRhsFunction(this) ;
	RegisterRHSFunction(m_pRhsInterrupt) ;
	RegisterRHSFunction(m_pRhsConcat) ;

	// Set counters and flags used to control runs
	InitializeRuntimeState() ;

	m_pBeforeDestroyedListener = NULL ;

	// Create a listener for output events and other events we listen for
	m_pOutputListener = new OutputListener(pKernelSML, pIAgent) ;

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
	Clear(true) ;

	// If we have an output listener object, delete it now.
	// NOTE: At this point we're assuming AgentSML objects live as long as the underlying gSKI Agent object.
	// If not, we need to unregister this listener, but we shouldn't do that here as the Agent object may
	// be invalid by the time this destructor is called.
	delete m_pOutputListener ;

	delete m_pInputProducer ;
}

// Release any objects or other data we are keeping.  We do this just
// prior to deleting AgentSML, but before the underlying gSKI agent has been deleted
// 'deletingThisAgent' should only be true when we're actually in the destructor.
void AgentSML::Clear(bool deletingThisAgent)
{
#ifdef DEBUG_UPDATE
	PrintDebugFormat("AgentSML::Clear start %s", deletingThisAgent ? "deleting this agent." : "not deleting this agent.") ;
#endif

	// Release any WME objects we still own.
	// (Don't flush removes in this case as we're shutting down rather than just doing an init-soar).
	ReleaseAllWmes(!deletingThisAgent) ;

	RemoveRHSFunction(m_pRhsInterrupt) ; delete m_pRhsInterrupt ; m_pRhsInterrupt = NULL ;
	RemoveRHSFunction(m_pRhsConcat) ; delete m_pRhsConcat ; m_pRhsConcat = NULL ;

	m_ProductionListener.Clear();
	m_RunListener.Clear();
	m_PrintListener.Clear();
	m_pOutputListener->Clear() ;
	m_XMLListener.Clear() ;

#ifdef DEBUG_UPDATE
	PrintDebugFormat("AgentSML::Clear end %s", deletingThisAgent ? "deleting this agent." : "not deleting this agent.") ;
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
	PrintDebugFormat("****************************************************") ;
	PrintDebugFormat("%s AgentSML::ReleaseAllWmes start %s", this->GetIAgent()->GetName(), flushPendingRemoves ? "flush pending removes." : "do not flush pending removes.") ;
	PrintDebugFormat("****************************************************") ;
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

	if (m_InputLinkRoot)
	{
		m_InputLinkRoot->Release() ;
	}
	m_InputLinkRoot = NULL ;

	m_TimeTagMap.clear() ;
	m_ToClientIdentifierMap.clear() ;
	m_IdentifierMap.clear() ;

#ifdef DEBUG_UPDATE
	PrintDebugFormat("****************************************************") ;
	PrintDebugFormat("%s AgentSML::ReleaseAllWmes end %s", this->GetIAgent()->GetName(), flushPendingRemoves ? "flush pending removes." : "do not flush pending removes.") ;
	PrintDebugFormat("****************************************************") ;
#endif
}

void AgentSML::InitializeRuntimeState()
{
	m_WasOnRunList = false;
	m_ScheduledToRun = false ;
	m_OnStepList = false;
	m_ResultOfLastRun = gSKI_RUN_COMPLETED ;
	m_InitialRunCount = 0 ;
	m_CompletedOutputPhase = false ;
	m_GeneratedOutput = false ;
	m_OutputCounter = 0 ;
    m_localRunCount = 0 ;
    m_localStepCount= 0 ;
    m_runState = gSKI_RUNSTATE_STOPPED;
    m_interruptFlags = 0 ;
}


bool AgentSML::Reinitialize()
{
	bool ok = GetIAgent()->Reinitialize();
	InitializeRuntimeState() ;
	return ok ;
}

void AgentSML::RemoveAllListeners(Connection* pConnection)
{
	m_ProductionListener.RemoveAllListeners(pConnection);
	m_RunListener.RemoveAllListeners(pConnection);
	m_PrintListener.RemoveAllListeners(pConnection);
	m_pOutputListener->RemoveAllListeners(pConnection) ; 
	m_XMLListener.RemoveAllListeners(pConnection) ;
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
egSKIPhaseType AgentSML::GetCurrentPhase()
{
	return gSKI::EnumRemappings::ReMapPhaseType((unsigned short)m_agent->current_phase,0);
}

unsigned long AgentSML::GetRunCounter(egSKIRunType runStepSize)
{
	switch(runStepSize)
	{
	case gSKI_RUN_SMALLEST_STEP:
		assert(0) ;
		return 0 ;	// Not supported
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
	case gSKI_RUN_FOREVER:
		{
			unsigned long decs = GetNumDecisionCyclesExecuted() ;
			return decs ;
		}
	default:
		return 0;
	}
}

//=============================
// Request that the agent stop soon.
//=============================
bool AgentSML::Interrupt(egSKIStopLocation stopLoc, gSKI::Error* err)
{
  // This type of stopping requires full threading
  assert(stopLoc  != gSKI_STOP_ON_CALLBACK_RETURN) ; //, "This mode is not implemented.");
  assert(stopLoc  != gSKI_STOP_AFTER_ALL_CALLBACKS_RETURN) ; //, "This mode is not implemented.");
  if ((stopLoc  == gSKI_STOP_ON_CALLBACK_RETURN) ||
     (stopLoc  == gSKI_STOP_AFTER_ALL_CALLBACKS_RETURN))
  { 
	  SetError(err, gSKI::gSKIERR_NOT_IMPLEMENTED);
     return false;
  }

  // We are in the stuff we can implement
  ClearError(err);

  // Tell the agent where to stop
  m_interruptFlags = stopLoc;

  // If the request for interrupt is gSKI_STOP_AFTER_DECISION_CYCLE, then it 
  // will be caught in the RunScheduler::CompletedRunType() and/or IsAgentFinished().
  // We don't want to interrupt agents until the appropriate time if the request is  
  // gSKI_STOP_AFTER_DECISION_CYCLE.
  
  // These are immediate requests for interrupt, such as from RHS or application
  if ((gSKI_STOP_AFTER_SMALLEST_STEP == stopLoc) || (gSKI_STOP_AFTER_PHASE == stopLoc)) {
	  m_agent->stop_soar = TRUE;
	  // If the agent is not running, we should set the runState flag now so agent won't run
	  if (m_runState == gSKI_RUNSTATE_STOPPED)
	  {
		  m_runState = gSKI_RUNSTATE_INTERRUPTED;
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
  if(m_runState == gSKI_RUNSTATE_INTERRUPTED)
  {
    // We returned, and thus are stopped
    m_runState = gSKI_RUNSTATE_STOPPED;
  }
}

egSKIRunResult AgentSML::StepInClientThread(egSKIInterleaveType  stepSize, gSKI::Error* pError)
{
  // Agent is already running, we cannot run
  if(m_runState != gSKI_RUNSTATE_STOPPED)
  {
     if(m_runState == gSKI_RUNSTATE_HALTED)
		 SetError(pError, gSKI::gSKIERR_AGENT_HALTED);  // nothing ever tests for this...
     else
		 SetError(pError, gSKI::gSKIERR_AGENT_RUNNING);

     return gSKI_RUN_ERROR;
  }

  m_runState = gSKI_RUNSTATE_RUNNING;

  // Now clear error and do the run
  ClearError(pError);

  // This method does all the work
  return Step(stepSize, pError);
}

void AgentSML::FireRunEvent(egSKIRunEventId eventId) {
	GetIAgent()->FireRunEvent(eventId, (unsigned short)m_agent->current_phase) ;
}

void AgentSML::FireSimpleXML(char const* pMsg)
{
   GetIAgent()->FirePrintEvent(gSKIEVENT_PRINT, pMsg) ;
   GetIAgent()->FireXMLEvent(gSKIEVENT_XML_TRACE_OUTPUT, sml_Names::kFunctionBeginTag, sml_Names::kTagMessage, 0) ; 
   GetIAgent()->FireXMLEvent(gSKIEVENT_XML_TRACE_OUTPUT, sml_Names::kFunctionAddAttribute, sml_Names::kTypeString, pMsg) ; 
   GetIAgent()->FireXMLEvent(gSKIEVENT_XML_TRACE_OUTPUT, sml_Names::kFunctionEndTag, sml_Names::kTagMessage, 0) ; 

   /*
   PrintNotifier nfIntr(this, "Interrupt received.");
   m_printListeners.Notify(gSKIEVENT_PRINT, nfIntr);
   XMLNotifier xn1(this, kFunctionBeginTag, kTagMessage, 0) ;
   m_XMLListeners.Notify(gSKIEVENT_XML_TRACE_OUTPUT, xn1);
   XMLNotifier xn2(this, kFunctionAddAttribute, kTypeString, "Interrupt received.") ;
   m_XMLListeners.Notify(gSKIEVENT_XML_TRACE_OUTPUT, xn2);
   XMLNotifier xn3(this, kFunctionEndTag, kTagMessage, 0) ;
   m_XMLListeners.Notify(gSKIEVENT_XML_TRACE_OUTPUT, xn3);
   */
}

static bool maxStepsReached(unsigned long steps, unsigned long maxSteps)
{
	return (steps >= maxSteps);
}

egSKIRunResult AgentSML::Step(egSKIInterleaveType stepSize, gSKI::Error* pError)
{    
   // NOTE: This only works because they have the same ordering
   // BADBAD: Eventually we should dispose of one of these types and fold them into a single enum
   egSKIRunType runStepSize = (egSKIRunType)stepSize ;

   // This method runs a single agent
   unsigned long count = 1 ;
   unsigned long startCount        = GetRunCounter(runStepSize) ; // getReleventCounter(stepSize);
   const unsigned long  END_COUNT  = startCount + count ;

   bool interrupted  = (m_runState == gSKI_RUNSTATE_INTERRUPTED)? true: false;

   if (! interrupted) {
	   assert(!m_agent->system_halted) ; // , "System should not be halted here!");
	   // Notify that agent is about to execute. (NOT the start of a run, just a step)
	   FireRunEvent(gSKIEVENT_BEFORE_RUNNING) ;
	   //RunNotifier nfBeforeRunning(this,EnumRemappings::ReMapPhaseType(m_agent->current_phase,0));
	   //m_runListeners.Notify(gSKIEVENT_BEFORE_RUNNING, nfBeforeRunning);

	   switch (stepSize) 
	   {
	   case  gSKI_INTERLEAVE_SMALLEST_STEP:     run_for_n_elaboration_cycles(m_agent, count); break;
	   case  gSKI_INTERLEAVE_ELABORATION_PHASE: run_for_n_elaboration_cycles(m_agent, count); break;
	   case  gSKI_INTERLEAVE_PHASE:             run_for_n_phases(m_agent, count);             break;
	   case  gSKI_INTERLEAVE_DECISION_CYCLE:    run_for_n_decision_cycles(m_agent, count);    break;
	   case  gSKI_INTERLEAVE_OUTPUT:            run_for_n_modifications_of_output(m_agent, count); 
												break;
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
	   FireRunEvent(gSKIEVENT_AFTER_INTERRUPT) ;

	   /* This is probably redundant with the event above, which clients can listen for... */
	   FireSimpleXML("Interrupt received.") ;
   }
 
   egSKIRunResult retVal;

   // We've exited the run loop so we see what we should return
   if(m_agent->system_halted)
   {
	   // if the agent halted because it is in an infinite loop of no-change impasses
	   // interrupt the agents and allow the user to try to recover.
	   if ((long)m_agent->bottom_goal->id.level >=  m_agent->sysparams[MAX_GOAL_DEPTH])
	   {// the agent halted because it seems to be in an infinite loop, so throw interrupt
		   m_pKernelSML->InterruptAllAgents(gSKI_STOP_AFTER_PHASE, pError) ;
		   m_agent->system_halted = FALSE; // hack! otherwise won't run again.  
		   m_runState = gSKI_RUNSTATE_INTERRUPTED;
		   retVal     = gSKI_RUN_INTERRUPTED;
		   // Notify of the interrupt

		   FireRunEvent(gSKIEVENT_AFTER_INTERRUPT) ;

		   /* This is probably redundant with the event above, which clients can listen for... */
		   FireSimpleXML("Interrupt received.") ;
	   }
	   else {
	   // If we halted, we completed and our state is halted
	   m_runState    = gSKI_RUNSTATE_HALTED;
	   retVal        = gSKI_RUN_COMPLETED;

	   FireRunEvent(gSKIEVENT_AFTER_HALTED) ;

	   // fix for BUG 514  01-12-06
	   FireSimpleXML("This Agent halted.") ;
	   }
   }
   else if(maxStepsReached(GetRunCounter(runStepSize), END_COUNT)) 
   {
	   if(interrupted)
	   {
		   m_runState = gSKI_RUNSTATE_INTERRUPTED;
		   retVal     = gSKI_RUN_COMPLETED_AND_INTERRUPTED; 
		   //retVal     = gSKI_RUN_INTERRUPTED;
	   }
	   else
	   {
		   m_runState = gSKI_RUNSTATE_STOPPED;
		   retVal     = gSKI_RUN_COMPLETED;
	   }
   }
   else 
   {  
	   // We were interrupted before we could complete
	   assert(interrupted) ; //, "Should never get here if we aren't interrupted");

	   m_runState    = gSKI_RUNSTATE_INTERRUPTED;
	   retVal        = gSKI_RUN_INTERRUPTED;
   }    

   // Notify that agent stopped. (NOT the end of a run, just a step)
   // Use AFTER_RUN_ENDS if you want to trap the end of the complete run.
   FireRunEvent(gSKIEVENT_AFTER_RUNNING) ;

   return retVal;
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

		// Release any wmes or other objects we're keeping
		AgentSML* pAgentSML = pKernelSML->GetAgentSML(pAgent) ;
		pAgentSML->Clear(false) ;

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

#ifdef DEBUG_UPDATE
	PrintDebugFormat("AgentSML::AgentBeforeDestroyedListener end.") ;
#endif
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

void AgentSML::ScheduleAgentToRun(bool state) 
{ 
	if (m_pIAgent->GetRunState() != gSKI_RUNSTATE_HALTED) 
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

void AgentSML::RemoveLongTimeTag(long timeTag)
{
	char str[kMinBufferSize] ;
	Int2String(timeTag, str, sizeof(str)) ;

	m_TimeTagMap.erase(str) ;
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
	 safeSprintf(temp,128,"%c%lu",sym->id.name_letter,sym->id.name_number);
	 return std::string(temp);
	 break;
	case SYM_CONSTANT_SYMBOL_TYPE:
	 return (char *) (sym->sc.name);
	 break;
	case INT_CONSTANT_SYMBOL_TYPE:
	 safeSprintf(temp,128, "%ld",sym->ic.value);
	 return std::string(temp);
	 break;
	case FLOAT_CONSTANT_SYMBOL_TYPE:
	 safeSprintf(temp, 128, "%g",sym->fc.value);
	 return std::string(temp);
	 break;
	default:
	 assert( false ) ; // "This symbol type not supported";
	 return "";
	 break;
	}
}
