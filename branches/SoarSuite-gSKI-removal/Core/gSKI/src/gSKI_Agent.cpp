#include <portability.h>

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gSKI_Agent.cpp
*********************************************************************
* created:	   6/27/2002   10:44
*
* purpose: 
*********************************************************************/
#include "gSKI_Agent.h"
#include "gSKI_Error.h"
#include "MegaAssert.h"
#include "gSKI_ProductionManager.h"
#include "gSKI_InputLink.h"
#include "gSKI_OutputLink.h"
#include "gSKI_WorkingMemory.h"
#include "gSKI_EnumRemapping.h"
#include "gSKI_Kernel.h"
#include "gSKI_SymbolFactory.h"
#include "gSKI_Symbol.h"
#include "gSKI_ObjectToPtrIterator.h"
#include "gSKI_RhsFunction.h"
#include "agent.h"
#include "init_soar.h"
#include "rhsfun.h"
#include "production.h" // for struct multi_attributes
#include "print.h"      // for symboltostring
#include "gSKI_AgentPerformanceMonitor.h"
#include "gSKI_MultiAttribute.h"
#include "xmlTraceNames.h" // for constants for XML function types, tags and attributes
#include "decide.h"
#include "recmem.h"

using namespace xmlTraceNames;


// Private namespace for the workaround for callbacks
namespace {

   /** 
    * @brief Creates a Soar symbol based on a gSKI::ISymbol that is passed in
    *
    * This function is used as a helper for the function below to convert values
    *   returned by the RhsFunction object to Soar Symbols.
    *
    * @param thisAgent Pointer to the Soar agent from which to allocate the soar symbol
    * @param sym       Pointer to the ISymbol object to convert
    * @return A pointer to a soar symbol object with the same value as the given sym.
    *          This symbol will already have had its ref count incremented.
    */
   Symbol* createSoarSymbolFromISymbol(agent* thisAgent, gSKI::ISymbol* sym)
   {
      MegaAssert(sym->GetType() != gSKI_OBJECT, "Do not support returning WMOs from RHS functions.");
      MegaAssert(sym->GetType() != gSKI_VARIABLE, "Do not support returning parse variables from RHS functions.");
      switch(sym->GetType())
      {
      case gSKI_INT:
	return make_int_constant(thisAgent, sym->GetInt());
      case gSKI_DOUBLE:
	return make_float_constant(thisAgent, static_cast<float>(sym->GetDouble()));
      case gSKI_STRING:
	{
	  // Gotta create a temp buffer because make_sym_constant takes a non-const pointer
	  std::vector<char> tmpBuffer(sym->GetString(), sym->GetString() + strlen(sym->GetString()) + 1);
	  return make_sym_constant(thisAgent, &tmpBuffer[0]);
	}
      default:
	MegaAssert(sym->GetType() != gSKI_INT && sym->GetType() != gSKI_DOUBLE && sym->GetType() != gSKI_STRING, "Unsupported type returned from RHS function.") ;
	break ;
      }
      return NIL;
   }

   /** 
    * @brief Interfaces with Soar to map kernel rhs functions to gSKI rhs functions
    *
    *  This method gets registered with the underlying kernel and recieves callbacks
    *   for client defined RHS functions.   It uses the user-data member of the
    *   kernel RHS function structure to store the this pointer of the gSKI object
    *   then calls back the gSKI object as desired.
    */
   Symbol* rhsFunctionProxy(agent* thisAgent, list* args, void* user_data)
   {
      //typedef gSKI::ObjectToPtrIterator<gSKI::ISymbol*, std::vector<gSKI::gSymbol> > tIterator;
      typedef gSKI::Iterator<gSKI::ISymbol*, std::vector<gSKI::gSymbol*> > tIterator;

      // Since we registered this callback, we know what the user data is.
      gSKI::RhsFunction* rhsFunction = static_cast<gSKI::RhsFunction*>(user_data);

      // Prepare arguments

      // Event though the agent has its own symbol factory, we create a new one because
      //  we have no way to access the old one conveniently.  This should not cause
      //  any problems.
      gSKI::SymbolFactory sf(thisAgent);

      // List of symbols wrapped in gSymbols
      std::vector<gSKI::gSymbol*> symVector;
      for(; args != NIL; args = args = args->rest)
      {
         // Cannnot yet support this, so we have to check for identifier symbols
         //if(sym->sc.common_symbol_info.symbol_type != IDENTIFIER_SYMBOL_TYPE) {
	         symVector.push_back(new gSKI::gSymbol(thisAgent, static_cast<Symbol*>(args->first), 0, false));
	 //} else {
	 // Cannnot yet support this, so we have to check for identifier symbols
	 //MegaAssert(false,"Cannot yet support WMO's passed to RHS functions");
	 //}
      }

      Symbol* pSoarReturn = 0;

      // Check to make sure we have the right number of arguments.   
      if( (rhsFunction->GetNumExpectedParameters() == gSKI_PARAM_NUM_VARIABLE) ||
          ((int)symVector.size() == rhsFunction->GetNumExpectedParameters()) )
      {
         // Actually make the call.  We can do the dynamic cast because we passed in the
         //  symbol factory and thus know how the symbol was created.
         tIterator it(symVector);
         gSKI::ISymbol* pReturn = rhsFunction->Execute(&it, &sf);

         // Return the result, assuming it is not NIL
         if(rhsFunction->IsValueReturned() == true)
         {
            // There should be a return value
            MegaAssert(pReturn != 0, "Expected return value from RHS function.");
            if(pReturn != 0)
            {
               // This adds a reference count
               Symbol* s = createSoarSymbolFromISymbol(thisAgent, pReturn);

               // Return the result
               pSoarReturn = s;
            }
            else
            {
               // We have to return something to prevent a crash, so we return an error code
               pSoarReturn = make_sym_constant(thisAgent, "error_expected_rhs_function_to_return_value_but_it_did_NOT");
            }
         }
         else
         {
            MegaAssert(pReturn == 0, "Expected that the rhs function would not return a value, but it did.  Return value ignored.");
         }

         // In any case, we are done using the return value
         if(pReturn != 0)
            pReturn->Release();
      }
      else
      {
         MegaAssert(false, "Wrong number of arguments passed to rhsFunction.");

         if(rhsFunction->IsValueReturned() == true)
            // We can return anything we want to soar; we return an error message so at least the problem is obvious.
            pSoarReturn = make_sym_constant(thisAgent, "error_wrong_number_of_args_passed_to_rhs_function");
      }

      for(std::vector<gSKI::gSymbol*>::iterator it = symVector.begin(); it != symVector.end(); ++it)
      {
         (*it)->Release();
      }

      // If we allocated
      // Return this to soar if we don't have a symbol
      return pSoarReturn;
   }
}

namespace gSKI 
{

   Agent::Agent(agent* pSoarAgent, Kernel *kernel): 
      m_productionManager(0), 
      m_agent(0), 
      m_active(true),
      m_kernel(kernel),
	  m_pPerfMon(0)
   {
      MegaAssert(pSoarAgent != 0, "Null agent name pointer passed to agent constructor!");
      MegaAssert(kernel != 0, "Null kernel pointer passed to agent constructor!");

      // Why doesn't this call one of the initialize functions????
      initializeRuntimeState();

      m_agent = pSoarAgent ; // create_soar_agent(m_kernel->GetSoarKernel(), const_cast<char *>(agentName));     
      MegaAssert(m_agent != 0, "Unable to create soar agent!");	 
   }

  void Agent::Init()
  {
     // Temporary HACK.  This should be fixed in the kernel.
     m_agent->stop_soar = FALSE;

     // Creating the output link
     // NOTE: THE OUTPUT LINK CREATION MUST COME BEFORE THE INITIALIZE CALL
     // FOR THE OUTPUT LINK CALLBACK TO BE PROPERLY REGISTERED (see io.cpp for more 
     // details in the update_for_top_state_wme_addition function)
     m_outputlink = new OutputLink(this);

     // Initializing the soar agent
     initialize_soar_agent(m_kernel->GetSoarKernel(), m_agent);
     
     m_inputlink = new InputLink(this);
     m_workingMemory = new WorkingMemory(this);

     m_pPerfMon = new AgentPerformanceMonitor(this);
  }

  Agent::~Agent()
   {
	   /* RPM 9/06 added code from reinitialize_soar to clean up stuff hanging from last run
	               need to put it here instead of in destroy_soar_agent because gSKI is
				    cleaning up too much stuff and thus it will crash if called later */
	   clear_goal_stack (m_agent);
	   m_agent->active_level = 0; /* Signal that everything should be retracted */
	   m_agent->FIRING_TYPE = IE_PRODS;
	   do_preference_phase (m_agent);   /* allow all i-instantiations to retract */

      delete m_pPerfMon;

      // Cleaning up the input and output links and the working memory
      // object since these are wholly owned by the agent.
      delete m_inputlink;
      delete m_outputlink;
      delete m_workingMemory;
      delete m_productionManager;
   
      destroy_soar_agent(m_kernel->GetSoarKernel(), m_agent);
   }

   /*
   =============================

   =============================
   */
   void Agent::initializeRuntimeState()
   {
      m_phaseCount        = 0;
      m_elaborationCount  = 0;
      m_decisionCount     = 0;  // should be m_agent->d_cycle_count.  Can we delete this var?
      m_outputCount       = 0;
	  m_nilOutputCycles   = 0;

      // This tells run that we are starting a new cycle
      m_lastPhase         = gSKI_OUTPUT_PHASE; /* okay eventhough not correct for Soar 7 */
      m_nextPhase         = gSKI_INPUT_PHASE;

      // perhaps we need to tell the agent manager to stop all agents or to only stop this agent
      // BUG agent doesn't actually stop
      m_runState          = gSKI_RUNSTATE_STOPPED;

      // Clear out interrupts
      m_suspendOnInterrupt = false;
      m_interruptFlags     = 0;
   }

   bool Agent::Reinitialize(const char*       productionFileName, 
                            bool              learningOn,
                            egSKIOSupportMode oSupportMode,
                            Error*            err)
   {
     //MegaAssert(false, "Not implemented yet.");
      ClearError(err);

      /// INITIALIZATION HERE
      initializeRuntimeState();

      // Reinitializing the input and output links
      m_inputlink->Reinitialize();
      m_outputlink->Reinitialize();
      m_workingMemory->Reinitialize();

      // !!!
      // perhaps we need to tell the agent manager to stop all agents or to only stop this agent
      // tcl/tgd needs to know that the agent(s?) stop
      // tcl either runs all of them, or none at all, so our only option (right now) is to stop all (?)
      // -- has to be done outside of gski, gski can't know about it from an oop perspective

      // !! remember tgd clears all interrupts each time it calls run

      // reinitialize_soar cleans out the agents memory the 
      // init_agent_memory call adds back in the top state and
      // other misc. objects and wmes.
      bool ok = reinitialize_soar( m_agent );
      init_agent_memory( m_agent );

      // Tell listeners it is over
      //am->FireAfterAgentReinitialized(this);

      return ok;
   }

   /*
   =============================

   =============================
   */
   bool Agent::Interrupt(egSKIStopLocation    stopLoc, 
                         egSKIStopType        stopType,
                         Error*               err)
   {
      // This type of stopping requires full threading
      MegaAssert(stopType != gSKI_STOP_BY_SUSPENDING, "This mode is not implemented.");
      if(stopType == gSKI_STOP_BY_SUSPENDING)
      { 
         SetError(err, gSKIERR_NOT_IMPLEMENTED);
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
      m_suspendOnInterrupt = (stopType == gSKI_STOP_BY_SUSPENDING)? true: false;


      return true;
   }

   /*
   =============================

   =============================
   */
   void Agent::ClearInterrupts(Error* err)
   {
      ClearError(err);

      // Clear the interrupts whether running or not
      m_interruptFlags = 0;

      // Only change state of agent if it is running
      if(m_runState == gSKI_RUNSTATE_INTERRUPTED)
      {

         // Check about suspension
         if(m_suspendOnInterrupt)
         {
            // TODO: When we implement suspend capabilities, waking
            //  would go here...
            m_suspendOnInterrupt = false;

            // We were in the middle of running
            m_runState = gSKI_RUNSTATE_RUNNING;
         }
         else
         {
            // We returned, and thus are stopped
            m_runState = gSKI_RUNSTATE_STOPPED;
         }
      }
   }

   unsigned long Agent::GetInterruptFlags(Error* err)
   {
      ClearError(err);
      return m_interruptFlags;
   }

   /*
   =============================

   =============================
   */
   void Agent::Halt(Error* err)
   {
      ClearError(err);

      // Tell soar we halted (may have to lock here)
      m_agent->system_halted = TRUE;

      // If we are not running, set the run state to halted
      // If we are running, the step method will set the
      //   state to halted.
	  if(m_runState != gSKI_RUNSTATE_RUNNING) 
	  {
		  m_runState = gSKI_RUNSTATE_HALTED;

		   RunNotifier nfAfterHalt(this, m_lastPhase);
           m_runListeners.Notify(gSKIEVENT_AFTER_HALTED, nfAfterHalt);

		  // fix for BUG 514  01-12-06
		  PrintNotifier nfHalted(this, "This Agent halted.");
		  m_printListeners.Notify(gSKIEVENT_PRINT, nfHalted);
		  XMLNotifier xn1(this, kFunctionBeginTag, kTagMessage, 0) ;
		  m_XMLListeners.Notify(gSKIEVENT_XML_TRACE_OUTPUT, xn1);
		  XMLNotifier xn2(this, kFunctionAddAttribute, kTypeString, "This Agent halted.") ;
		  m_XMLListeners.Notify(gSKIEVENT_XML_TRACE_OUTPUT, xn2);
		  XMLNotifier xn3(this, kFunctionEndTag, kTagMessage, 0) ;
		  m_XMLListeners.Notify(gSKIEVENT_XML_TRACE_OUTPUT, xn3);
	  }
   }

   egSKIRunState Agent::GetRunState(Error* err)
   {
      ClearError(err);
      return m_runState;
   }
   	  
   void Agent::SetRunState(egSKIRunState state,Error* err)
   { 
      ClearError(err);
	  m_runState = state; 
   }

   bool Agent::AddClientRhsFunction(RhsFunction* rhsFunction, 
                                    Error*        err)
   {
      ClearError(err);

      // Tell Soar about it
      std::vector<char> tmpBuffer(rhsFunction->GetName(), rhsFunction->GetName() + strlen(rhsFunction->GetName()) + 1);
      add_rhs_function (m_agent, 
                        make_sym_constant(m_agent, &tmpBuffer[0]),
                        rhsFunctionProxy,
                        rhsFunction->GetNumExpectedParameters(),
                        rhsFunction->IsValueReturned(),
                        true,
                        static_cast<void*>(rhsFunction));

      return false;
   }

   bool Agent::RemoveClientRhsFunction(const char* szName, Error* err)
   {
      ClearError(err);
   
      // Tell the kernel we are done listening.
      std::vector<char> tmpBuffer(szName, szName + strlen(szName) + 1);

	  //RPM 9/06: removed symbol ref so symbol is released properly
	  Symbol* tmp = make_sym_constant(m_agent, &tmpBuffer[0]);
      remove_rhs_function(m_agent, tmp);
	  symbol_remove_ref (m_agent, tmp);

      // Do our stuff
      //m_rhsFunctions.erase(it);

      return false;
   }

   const char* Agent::GetName(Error* err)
   {
      ClearError(err);

      return m_agent->name;
   }



   ProductionManager* Agent::GetProductionManager(Error* err)
   {
      ClearError(err);

      if(m_productionManager == 0) {
         m_productionManager = new ProductionManager(this);
      }

      return m_productionManager;
   }

   IInputLink* Agent::GetInputLink(Error* err)
   {
      ClearError(err);
      return m_inputlink;
   }

   IOutputLink* Agent::GetOutputLink(Error* err)
   {
      ClearError(err);
      return m_outputlink;
   }

   IWorkingMemory* Agent::GetWorkingMemory(Error* err)
   {
      ClearError(err);
      return m_workingMemory;
   }

   bool Agent::IsLearningOn(Error* err)
   {
      ClearError(err);
      return m_agent->sysparams[LEARNING_ON_SYSPARAM] ? true : false;
   }

   void Agent::SetLearning(bool on, Error* err)
   {
      ClearError(err);
      m_agent->sysparams[LEARNING_ON_SYSPARAM] = on;
   }
 
   int Agent::GetMaxChunks(Error* err /*= 0*/)
   {
      ClearError(err);
      return m_agent->sysparams[MAX_CHUNKS_SYSPARAM];
   }
   void Agent::SetMaxChunks(int maxChunks, Error* err /*= 0*/)
   {
      ClearError(err);
      m_agent->sysparams[MAX_CHUNKS_SYSPARAM] = maxChunks;
   }

   int Agent::GetMaxElaborations(Error* err /*= 0*/)
   {
      ClearError(err);
      return m_agent->sysparams[MAX_ELABORATIONS_SYSPARAM];
   }
   void Agent::SetMaxElaborations(int maxElabs, Error* err /*= 0*/)
   {
      ClearError(err);
      m_agent->sysparams[MAX_ELABORATIONS_SYSPARAM] = maxElabs;
   }

   int Agent::GetMaxNilOutputCycles(Error* err /*= 0*/)
   {
      ClearError(err);
      return m_agent->sysparams[MAX_NIL_OUTPUT_CYCLES_SYSPARAM];
   }
   void Agent::SetMaxNilOutputCycles(int maxNils, Error* err /*= 0*/)
   {
      ClearError(err);
      m_agent->sysparams[MAX_NIL_OUTPUT_CYCLES_SYSPARAM] = maxNils;
   }

   bool Agent::IsWaitingOnStateNoChange(Error* err)
   {
      ClearError(err);
      return m_agent->waitsnc != 0;
   }

   void Agent::SetWaitOnStateNoChange(bool on, Error* err)
   {
      ClearError(err);
      m_agent->waitsnc = on;
   }

   egSKIOSupportMode Agent::GetOSupportMode(Error* err)
   {
      ClearError(err);

      egSKIOSupportMode m = gSKI_O_SUPPORT_MODE_4;
      switch(m_agent->o_support_calculation_type)
      {
      case 0: m = gSKI_O_SUPPORT_MODE_0; break;
      case 2: m = gSKI_O_SUPPORT_MODE_2; break;
      case 3: m = gSKI_O_SUPPORT_MODE_3; break;
      case 4: m = gSKI_O_SUPPORT_MODE_4; break;
      default:
         MegaAssert(false, "Invalid o-support-mode setting");
      }
      return m;
   }

   void Agent::SetOSupportMode(egSKIOSupportMode mode, Error* err) 
   {
      ClearError(err);
	  switch (mode) 
	  {
		  case gSKI_O_SUPPORT_MODE_0:
			  m_agent->o_support_calculation_type = 0;
			  break;
		  case gSKI_O_SUPPORT_MODE_2:
			  m_agent->o_support_calculation_type = 2;
			  break;
		  case gSKI_O_SUPPORT_MODE_3:
			  m_agent->o_support_calculation_type = 3;
			  break;
		  case gSKI_O_SUPPORT_MODE_4:
			  m_agent->o_support_calculation_type = 4;
			  break;
		  default:
			  MegaAssert(false, "Invalid o-support-mode");
	  }
   }

   egSKIUserSelectType Agent::GetIndifferentSelection(Error* err /*= 0*/)
   {
      int us = m_agent->sysparams[USER_SELECT_MODE_SYSPARAM];

      switch(us)
      {
      case USER_SELECT_FIRST  : return gSKI_USER_SELECT_FIRST;
      case USER_SELECT_ASK    : return gSKI_USER_SELECT_ASK;
      case USER_SELECT_RANDOM : return gSKI_USER_SELECT_RANDOM;
      case USER_SELECT_LAST   : return gSKI_USER_SELECT_LAST;
      default:
         MegaAssert(false, "Invalid indifferent selection setting");
         return gSKI_USER_SELECT_FIRST;
      }
   }

   void Agent::SetIndifferentSelection(egSKIUserSelectType t, Error* err /*= 0*/)
   {
      int us = -1;
      switch(t)
      {
      case gSKI_USER_SELECT_FIRST  : us = USER_SELECT_FIRST;   break;
      case gSKI_USER_SELECT_ASK    : us = USER_SELECT_ASK;     break;
      case gSKI_USER_SELECT_RANDOM : us = USER_SELECT_RANDOM;  break;
      case gSKI_USER_SELECT_LAST   : us = USER_SELECT_LAST;    break;
      default:
         MegaAssert(false, "Invalid indifferent selection setting");
      }
      if(us != -1)
      {
         m_agent->sysparams[USER_SELECT_MODE_SYSPARAM] = us;
      }
   }

   int Agent::GetDefaultWMEDepth(Error* err)
   {
	   return m_agent->default_wme_depth;
   }

   void Agent::SetDefaultWMEDepth(int d, Error* err)
   {
	   m_agent->default_wme_depth = d;
   }

   egSKIPhaseType Agent::GetCurrentPhase(Error* err)
   {
      ClearError(err);
      // return m_nextPhase;
	  // KJC:  shouldn't this really be 
	  return EnumRemappings::ReMapPhaseType(m_agent->current_phase,0);
	  // should we also set m_lastPhase??
   }

   void Agent::ResetNilOutputCounter(Error* err)
   {
	  ClearError(err);
	  m_nilOutputCycles = 0;
   }

      void Agent::AddRhsFunctionChangeListener(egSKISystemEventId    nEventId, 
                                        IRhsFunctionChangeListener*  pListener, 
                                        bool                         bAllowAsynch,
                                        Error*                       err)
      {
         ClearError(err);
      
      }


      void Agent::RemoveRhsFunctionChangeListener(egSKISystemEventId    nEventId,
                                           IRhsFunctionChangeListener*  pListener,
                                           Error*                       err)
      {
         ClearError(err);
      
      }


      void Agent::AddRhsFunctionListener(egSKISystemEventId  nEventId, 
                                  IRhsFunctionListener* pListener, 
                                  bool                  bAllowAsynch,
                                  Error*                err)
      {
         ClearError(err);
      
      }


      void Agent::AddRhsFunctionListenerNameFilter(egSKISystemEventId nEventId,
                                            IRhsFunctionListener* pListener,
                                            const char*           szRhsFuncNamePattern,
                                            bool                  bNegate,
                                            Error*                err)
      {
         ClearError(err);
      
      }

      void Agent::RemoveRhsFunctionListener(egSKISystemEventId      nEventId,
                                             IRhsFunctionListener*  pListener,
                                             Error*                 err)
      {
         ClearError(err);

      }


      void Agent::RemoveRhsFunctionListenerFilters(egSKISystemEventId     nEventId,
                                                    IRhsFunctionListener* pListener,
                                                    Error*                err)
      {
      
      }

   void Agent::HandleKernelXMLCallback(unsigned long			  eventId, 
                                            unsigned char         eventOccured,
                                            void*                 object, 
                                            agent*                soarAgent, 
                                            void*                 data)
   {
	   assert(false); // should not be used

	  //Agent* a = static_cast<Agent*>(object);
   //   gSKI_K_XMLCallbackData* xml_data = static_cast<gSKI_K_XMLCallbackData*>(data);

   //   // We have to change the the event id from a kernel id to a gSKI id

	  //XMLNotifier xn(a, xml_data->funcType, xml_data->attOrTag, xml_data->value);
   //   a->m_XMLListeners.Notify(EnumRemappings::Map_Kernel_to_gSKI_XMLEventId(eventId), xn);
   }


	// Called when a "RunEvent" occurs in the kernel
   void Agent::HandleEvent(egSKIRunEventId eventId, gSKI::Agent* agentPtr, egSKIPhaseType phase)
   {
   }

   /*
   =========================
	HandleRunEventCallback
   =========================
   */
   void Agent::HandleKernelRunEventCallback( soar_callback_agent agent,
											 soar_callback_event_id eventid,
					                         soar_callback_data callbackdata,
                                             soar_call_data calldata )
   {
	   // Kernel Decision cycle events have NULL calldata, so do phases...at least for now
	   // callbackdata holds the agent object and eventId
	RunEventCallbackData * e = static_cast<RunEventCallbackData*>(callbackdata);
	Agent*            a = e->a;
	egSKIRunEventId eventId = e->eventId;


	RunNotifier rn(a, EnumRemappings::ReMapPhaseType(a->m_agent->current_phase,0));
    a->m_runListeners.Notify(eventId, rn);

	if ((a->m_runListeners.GetNumListeners(gSKIEVENT_BEFORE_PHASE_EXECUTED) > 0) &&
		(IsBEFOREPhaseEventID(eventId)))
	{
		a->m_runListeners.Notify(gSKIEVENT_BEFORE_PHASE_EXECUTED , rn);
	}
	else if ((a->m_runListeners.GetNumListeners(gSKIEVENT_AFTER_PHASE_EXECUTED) > 0) &&
		(IsAFTERPhaseEventID(eventId)))
	{
		a->m_runListeners.Notify(gSKIEVENT_AFTER_PHASE_EXECUTED , rn);
	} 
	
   }

   void Agent::DeleteRunEventCallbackData (soar_callback_data data)
   {
	   delete static_cast <Agent::RunEventCallbackData*>  (data); 
   }

   // Listener to propagate the gSKI BEFORE_PHASE and AFTER_PHASE events 
   void Agent::HandleEventStatic(egSKIRunEventId eventId, Agent* a, egSKIPhaseType phase)
   {
	RunNotifier rn(a, EnumRemappings::ReMapPhaseType(a->m_agent->current_phase,0));
 
	// KJC 12/1/05:  everything should be going thru HandleKernelRunEventCallback
	//  or HandleRunEventCallback (for Elaborations only).
	if ((a->m_runListeners.GetNumListeners(gSKIEVENT_BEFORE_PHASE_EXECUTED) > 0) &&
		(IsBEFOREPhaseEventID(eventId)))
	{
		a->m_runListeners.Notify(gSKIEVENT_BEFORE_PHASE_EXECUTED , rn);
	}
	else if ((a->m_runListeners.GetNumListeners(gSKIEVENT_AFTER_PHASE_EXECUTED) > 0) &&
		(IsAFTERPhaseEventID(eventId)))
	{
		a->m_runListeners.Notify(gSKIEVENT_AFTER_PHASE_EXECUTED , rn);
	} 
}


   // TODO: Flesh out this function (dummy body allows compilation)
   IState* Agent::GetTopState(Error* err)
   {
      ClearError(err);

      return 0;
   }

   // TODO: Flesh out this function (dummy body allows compilation)
   IState* Agent::GetBottomState(Error* err)
   {
      ClearError(err);
      
      return 0;
   }

   tIMultiAttributeIterator* Agent::GetMultiAttributes(Error* pErr /*= 0*/)
   {
      typedef FwdContainerType< std::vector<IMultiAttribute * > >  tVec;
      typedef IteratorWithRelease<tVec::V, tVec::t>  tIter;
      
      tVec::t attrs;
      multi_attribute* m = m_agent->multi_attributes;
      while(m)
      {
         char tmp[1024];
         attrs.push_back(new MultiAttribute(this, 
                                            symbol_to_string(m_agent, m->symbol, TRUE, tmp, 1024), 
                                            m->value));
         m = m->next;
      }
      return new tIter(attrs);
   }

   IMultiAttribute* Agent::GetMultiAttribute(const char* attribute, Error* pErr /*= 0*/)
   {
      multi_attribute* m = m_agent->multi_attributes;
      std::vector<char> tmp(attribute, attribute + strlen(attribute) + 1);
      Symbol* s = make_sym_constant(m_agent, &tmp[0]);

      while (m) 
      {
         if (m->symbol == s) 
         {
            symbol_remove_ref(m_agent, s);
            return new MultiAttribute(this, attribute, m->value);
         }
         m = m->next;
      }
      symbol_remove_ref(m_agent, s);
      return 0;
   }

   void Agent::SetMultiAttribute(const char* attribute, 
                                    int priority,
                                    Error* pErr /*= 0*/)
   {
      multi_attribute* m = m_agent->multi_attributes;
      std::vector<char> tmp(attribute, attribute + strlen(attribute) + 1);
      Symbol* s = make_sym_constant(m_agent, &tmp[0]);

      while (m) 
      {
         if (m->symbol == s) 
         {
            m->value = priority;
            symbol_remove_ref(m_agent, s);
            return;
         }
         m = m->next;
      }
      /* sym wasn't in the table if we get here, so add it */
      m = (multi_attribute *) allocate_memory(m_agent, sizeof(multi_attribute), MISCELLANEOUS_MEM_USAGE);
      m->value = priority;
      m->symbol = s;
      m->next = m_agent->multi_attributes;
      m_agent->multi_attributes = m;
   }


   egSKINumericIndifferentMode Agent::GetNumericIndifferentMode(Error* pErr/* = 0*/) {

      egSKINumericIndifferentMode m = gSKI_NUMERIC_INDIFFERENT_MODE_AVG;

      switch(GetSoarAgent()->numeric_indifferent_mode) {
         case NUMERIC_INDIFFERENT_MODE_AVG:
            m = gSKI_NUMERIC_INDIFFERENT_MODE_AVG;
            break;
         case NUMERIC_INDIFFERENT_MODE_SUM:
            m =  gSKI_NUMERIC_INDIFFERENT_MODE_SUM;
            break;
         default:
            MegaAssert(false, "Invalid numeric indifferent mode");
            break;
      }

      return m;
   }

   void Agent::SetNumericIndifferentMode(egSKINumericIndifferentMode m, Error* pErr/* = 0*/) {
      switch(m) {
         case gSKI_NUMERIC_INDIFFERENT_MODE_AVG:
            GetSoarAgent()->numeric_indifferent_mode = NUMERIC_INDIFFERENT_MODE_AVG;
            return;
         case gSKI_NUMERIC_INDIFFERENT_MODE_SUM:
            GetSoarAgent()->numeric_indifferent_mode = NUMERIC_INDIFFERENT_MODE_SUM;
            return;
         default:
            MegaAssert(false, "Invalid numeric indifferent mode");
      }
   }

   int Agent::GetAttributePreferencesMode(Error* err) 
   {
	   return GetSoarAgent()->attribute_preferences_mode;
   }
	
   void Agent::SetAttributePreferencesMode(int mode, Error* err) 
   {
	   MegaAssert((mode >= 0) && (mode <= 2), "Attribute preferences mode must be 0, 1, or 2");
	   GetSoarAgent()->attribute_preferences_mode = mode;
   }
      
   int Agent::GetInputPeriod(Error* err) 
   {
	   return GetSoarAgent()->input_period;
   }
	
   void Agent::SetInputPeriod(int period, Error* err) 
   {
	   MegaAssert(period >= 0, "Input period must be non-negative");
	   GetSoarAgent()->input_period = period;
   }
      
	void Agent::FirePrintEvent(egSKIPrintEventId eventId, char const* pMsg)
	{
	   PrintNotifier nfIntr(this, pMsg);
	   m_printListeners.Notify(eventId, nfIntr);
	}

	void Agent::FireXMLEvent(egSKIXMLEventId eventId, const char* functionType, const char* attOrTag, const char* value)
	{
	   XMLNotifier xn3(this, functionType, attOrTag, value) ;
	   m_XMLListeners.Notify(eventId, xn3);
	}

   bool Agent::GetOperand2Mode() {
		return m_agent->operand2_mode ? true : false;
   }

   void Agent::SetOperand2Mode(bool mode) {
		m_agent->operand2_mode = mode ? TRUE : FALSE;
   }

   extern agent* GetSoarAgentPtr(Agent* agent)
   {
      return ((Agent*)agent)->GetSoarAgent();
   }


}
