/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file igski_agent.h 
*********************************************************************
* created:	   6/13/2002   12:18
*
* purpose: 
*********************************************************************/
#ifndef IGSKI_AGENT_H
#define IGSKI_AGENT_H

#include "IgSKI_Iterator.h"

#include "gSKI_Enumerations.h"
#include "gSKI_Events.h"

//
//
// NOTE!  THIS IS A WORKAROUND HACK!!!  THIS SHOULD NOT STAY HERE!!!
//        IT IS NEEDED FOR THE TSI UNTIL IT CAN GET REWRITTEN!
//
//
typedef struct agent_struct agent;

namespace gSKI {

   struct Error;
   class  IState;
   class  IProductionManager;
   class  IInputLink;
   class  IOutputLink;
   class  IWorkingMemory;
   class  IAgentThreadGroup;
   class  IRhsFunction;
   class  IAgentPerformanceMonitor;

   /**
   * @brief Interface for a Soar Agent
   *
   * The agent interface provides access to agent internals such as
   *  working memory, the input and output links and loaded productions.
   *
   */
   class IAgent : public gSKI::IRunListener {
   public:
    
      /** 
       * @brief Destructor for an agent
       *
       * The agent destructor cleans up memory used by the agent
       *  including the following:
       *
       * @li Working memory pools
       * @li Rete memory
       * @li Production memory
       * 
       * It also cleans up all sub-objects (such as the i/o links).
       *
       */
      virtual ~IAgent() { };
 
      /** 
       * @brief Reinitializes the agent
       *
       * Agents are initialized when added to the agent manager.  Call this
       *  method to reinitialize an agent that has already been initialized one
       *  or more times in the past.
       *
       * The initialization process erases all current working memory and
       *  all current production memory.  If you only want to erase working
       *  memory and keep production memory and any old settings, use
       *  the IAgentManager::ReinitializeWithOldSettings method.
       *
       * Agents can be running when you call Reinitialize.  In this case the
       *  agent will be reinitialized at the next available stop time
       *  (see IAgentThreadGroup::Stop).  During reinitialization, the thread
       *  group running the agent is paused.  After reinitialization, the 
       *  agent begins executing (assuming it is active, see IAgent::SetActive)
       *  again along with other agents in its thread group.
       *
       * Possible Errors:
       *   @li Any error returned by IProductionManager::LoadSoarFile if a production filename
       *         is supplied and the file doesn't exist or is somehow not usable (e.g. bad format)
       *
       * @param prodFileName (Optional) Name of a soar production file to load with the agent.  
       *                      If you don't want to load a production file with the agent, set
       *                      this parameter to 0.
       * @param learningOn  Pass true to initialize learning to on for this agent
       *                     Pass false to disable learning for this agent.  You
       *                     can change the learning setting after the agent
       *                     is created (even while it is running).
       * @param  oSupportMode O support mode to use for this agent.  The o-support
       *                      mode determines how o-support is calculated for
       *                      wmes.  You can only set the o-support mode when 
       *                      initializing agents. See egSKIOSupportMode for mode details.
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @return true if the agent was reinitialized successfully, false if there
       *           was an error and the agent could not be completely reinitialized.
       */
      virtual bool Reinitialize(const char*       productionFileName = 0, 
                                bool              learningOn         = false,
                                egSKIOSupportMode oSupportMode       = gSKI_O_SUPPORT_MODE_4,
                                Error*            err                = 0) = 0;

      /** 
       * @brief Reinitializes the agent using the current productions and settings
       *
       * This method is very similar to IAgent::Reinitialize except it does
       *  not excise any productions.  The current learning setting and
       *  o-support mode are used.  See IAgent::Reinitialize for more details.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @return true if the agent was reinitialized successfully, false if there
       *           was an error and the agent could not be completely reinitialized.
       */
      virtual bool ReinitializeWithOldSettings(Error* err = 0) = 0;

      /**
       * @brief Runs this agent in a separate thread
       *
       * This method creates a separate thread and calls RunInClientThread
       *  from within the new thread.
       *
       * This method will only work if the agent is currently stopped (not
       *  interrupted or running).
       *
       * This method returns immediately after starting the agents running.
       *  It does not block until run completion.
       *
       * Possible Errors:
       *   @li gSKIERR_AGENT_RUNNING if the agents is running or interrupted
       *
       * @see egSKIRunType
       *
       * @param runLength How long to run the system.  Choices are       
       *          gSKI_RUN_SMALLEST_STEP, gSKI_RUN_ELABORATION_PHASE, gSKI_RUN_PHASE,
       *          gSKI_RUN_DECISION_CYCLE, gSKI_RUN_UNTIL_OUTPUT, and
       *          gSKI_RUN_FOREVER.  See egSKIRunType for details.
       * @param  count For gSKI_RUN_SMALLEST_STEP, gSKI_RUN_PHASE, gSKI_RUN_ELABORATION_PHASE
       *          and gSKI_RUN_DECISION_CYCLE this parameter tells the method
       *          how many elaboration phases, decision phase or decision cycles
       *          to run before the thread groups return. For other run types
       *          this parameter is ignored.
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @return An egSKIRunResult value indicating how the state of the system
       *            after continuation executed. @see egSKIRunResult
       */
      virtual egSKIRunResult RunInSeparateThread(egSKIRunType        runLength     = gSKI_RUN_FOREVER, 
                                                 unsigned long       count         = 1,
                                                 Error*              err           = 0) = 0;

      /** 
       * @brief Runs this agent in a client owned thread.
       *
       * Call this method to execute an agent in a client-owned thread.  Typical
       *  reasons for doing this include running agents in a single threaded
       *  application, and managing threading and agent scheduling in the client.
       *  
       * This method will only work if the agent is currently stopped (not
       *  interrupted or running).
       *
       * This method will not return until the the given run length has elapsed
       *  (unless there is an error or Stop is called). After this method
       *  returns, you may call this method again to continue a run at the
       *  point the agents left off.
       *
       * Possible Errors:
       *   @li gSKIERR_AGENT_RUNNING if the agents are currently being
       *         run individually or as a group.
       *
       * @see egSKIRunType
       *
       * @param runLength How long to run the system.  Choices are       
       *          gSKI_RUN_SMALLEST_STEP, gSKI_RUN_ELABORATION_PHASE, gSKI_RUN_PHASE,
       *          gSKI_RUN_DECISION_CYCLE, gSKI_RUN_UNTIL_OUTPUT, and
       *          gSKI_RUN_FOREVER.  See egSKIRunType for details.
       * @param  count For gSKI_RUN_SMALLEST_STEP, gSKI_RUN_PHASE, gSKI_RUN_ELABORATION_PHASE
       *          and gSKI_RUN_DECISION_CYCLE this parameter tells the method
       *          how many elaboration phases, decision phase or decision cycles
       *          to run before returning. For other run types this parameter
       *          is ignored.
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @return An egSKIRunResult value indicating how the state of the system
       *            after continuation executed. @see egSKIRunResult  This method
       *            will never return gSKI_RUN_INTERRUPTED or 
       *            gSKI_RUN_COMPLETED_AND_INTERRUPTED because it returns before
       *            any interrupt can occur.
       */
      virtual egSKIRunResult RunInClientThread(egSKIRunType        runLength     = gSKI_RUN_FOREVER, 
                                               unsigned long       count         = 1,
                                               Error*              err           = 0) = 0;
      virtual egSKIRunResult StepInClientThread(egSKIInterleaveType   stepSize   = gSKI_INTERLEAVE_DECISION_CYCLE, 
                                                unsigned long  stepCount         = 1,
                                                Error*         err               = 0) = 0;
      /**
       * @brief Interrupts agent execution
       *
       * Call this method to interrupt agent execution.  If the agent is not running,
       *  nothing happens.
       *
       * Agents do not stop immediately upon being notified to interrupt. They stop
       *  at one of the safe stopping points listed in the egSKIStopLocation 
       *  enumeration.  Essentially this Interrupt method is a request to the agents
       *  to stop processing.  It will stop when it gets a chance.  
       *
       * Be careful when calling Interrupt from a single threaded application.   If
       *  you call interrupt in a single threaded app with stopType = gSKI_STOP_BY_SUSPEND
       *  you will put your whole process to sleep with no chance of waking.
       *
       * If the agent is already stopped or interrupted, the interrupt is still
       *  set.  You need to call ClearInterrupts to clear an interrupt created
       *  with the Interrupt method.
       *
       * Possible Errors:
       *   @li gSKIERR_CANNOT_SUSPEND if you specify gSKI_STOP_BY_SUSPEND
       *         from the client-owned thread that created the agents. (Only
       *         applicable when running agents using IAgentManager::RunInClientThread
       *         or IAgent::RunInClientThread).
       *   @li gSKIERR_CANNOT_STOP_FOR_CALLBACKS if you specify 
       *           gSKI_STOP_ON_CALLBACK_RETURN or gSKI_STOP_AFTER_ALL_CALLBACKS_RETURN
       *           together with gSKI_STOP_BY_RETURNING.  These settings are 
       *           incompatible because the system cannot safely return after
       *           callbacks (it can suspend, however).
       * 
       * @see egSKIStopLocation
       * @see egSKIStopType
       *
       * @param stopLoc Where to interrupt agent execution.  Possible locations are
       *         gSKI_STOP_ON_CALLBACK_RETURN, gSKI_STOP_AFTER_ALL_CALLBACKS_RETURN,
       *         gSKI_STOP_AFTER_CURRENT_PHASE, gSKI_STOP_NEXT_ELABORATION_CYCLE,
       *         gSKI_STOP_NEXT_PHASE, and gSKI_STOP_NEXT_DECISION_CYCLE.
       *         See egSKIStopLocation for more details.
       * @param stopType How to stop the agent.  This can be one of
       *               gSKI_STOP_BY_RETURNING or gSKI_STOP_BY_SLEEPING.
       *               See egSKIStopType for more details.
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @return true if the agent could be stopped.  false if there was an
       *           error preventing it from being stopped.  See err for 
       *           detailed error information.
       */
      virtual bool Interrupt(egSKIStopLocation    stopLoc, 
                             egSKIStopType        stopType,
                             Error*               err   = 0) = 0;
	  virtual unsigned long GetInterruptFlags (Error* err = 0) = 0;

      /** 
       * @brief Clears any interrupts currently set for this agent
       *
       * Call to cause an agent to clear any currently set interrupts.
       *  After calling this method the runstate of the agent is
       *  set to gSKI_RUNSTATE_STOPPED.  
       *
       * If the agent was stopped by suspending the thread, this method
       *  will wake the thread and it will begin processing again
       *
       * Calling this method when the agent is stopped or running
       *  has no effect.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       */
      virtual void ClearInterrupts(Error* err = 0) = 0;

      /** 
       * @brief Halts an agent permanently
       *
       * Call this method to immediately halt the agent permanently. A halted
       *  agent must be reinitialized before it can run again after halting.
       *  The halt actually occurs as soon as the agent completes the current
       *  smallest step.  This method results in the agent run state being
       *  set to gSKI_RUNSTATE_HALTED.
       *
       * If the agent is already halted, this method does nothing.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       */
      virtual void Halt(Error* err = 0) = 0;

      /**
       * @brief Gets the current state of execution for this agent.
       *
       * An agent can be stopped, running or interrupted.
       *
       * @see egSKIRunState
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       * 
       * @returns an enumerated value indicating the current state of execution
       *           for this agent.
       */
      virtual egSKIRunState GetRunState(Error* err = 0) = 0;
	  virtual void          SetRunState(egSKIRunState state, Error* err = 0) = 0;


      /**
       * @brief Gets the name of the agent
       *
       * Every agent has a name.  This name is assigned when the
       *  agent is added to the agent manager and must be unique
       *  within the scope of the agent manager that created the
       *  agent.
       * 
       * @param  err Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       * 
       * @returns A pointer to a C style string containing the name
       *            of the agent.  The pointer may be 0 if an
       *            error occurs.
       */
      virtual const char* GetName(Error* err = 0) = 0;

      /**
       * @brief Returns a pointer to this agent's production manager
       *
       * Use the production manager to load and excise productions and
       *  to iterate over productions and matches.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       * 
       * @returns A pointer to the production manager for this agent.  This
       *           pointer may be 0 if the function fails.
       */
      virtual IProductionManager* GetProductionManager(Error* err = 0) = 0;

       /**
       * @brief Returns a pointer to the input link for this agent
       *
       * Use the input link object to add and remove data from the agent's input
       *  link memory.  The input link is a form of working memory so it can
       *  be manipulated the same way other working memory is manipulated.
       *
       * @see IInputLink 
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       * 
       * @returns A pointer to the input link for this agent.  This pointer
       *             may be 0 if the function fails.
       */
      virtual IInputLink* GetInputLink(Error* err = 0) = 0;

       /**
       * @brief Returns a pointer to the output link for this agent
       *
       * Use the output link to process any commands issued by the agent.
       *  Typically, commands are processed by registering output consumers
       *  with the output link; however, you can also directly read the
       *  the output link to process commands.
       *
       * The output link is a form of working memory so it can be manipulated
       *  the same way other working memory is manipulated.
       *
       * @see IOutputLink
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       * 
       * @returns A pointer to the output link for this agent.  This pointer
       *             may be 0 if the function fails.
       */
      virtual IOutputLink* GetOutputLink(Error* err = 0) = 0;

       /**
       * @brief Returns a pointer to the agents main working memory
       *
       * Use the main working memory object to iterate over agent memory
       *  for debugging or agent introspection purposes.  As a general
       *  rule it is not a good idea to change main working memory
       *  from the client program because this may confuse the agent
       *  (though it is not prohibited).
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       * 
       * @returns A pointer to the main memory object for this agent.  This pointer
       *             may be 0 if the function fails.
       */
      virtual IWorkingMemory* GetWorkingMemory(Error* err = 0) = 0;

      /** 
       * @brief Get the agent's top state. 
       *
       * Call this method to get the agent's top state.  The top state has
       *  limitted functionality since you cannot traverse to substates
       *  from the top-state.  If you need to get the current state (goal)
       *  stack, use the IAgent::GetBottomState and then iterate up to
       *  the top state using IState::GetSuperstate.
       *
       * The top state exists for the lifetime of this agent.
       *
       * @see IAgent::GetBottomState
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       * 
       * @returns A pointer to the top state of this object. This pointer may
       *           be 0 if an error occurs.
       */
      virtual IState* GetTopState(Error* err = 0) = 0;

      /** 
       * @brief Get the agent's bottom state
       *
       * The bottom state is the agent's lowest substate.  This state is a
       *  superstate containing all substates above it including the top state.
       *  Therefore, you can use this method, combined with iteration using
       *  IState::GetSuperstate to traverse the goal stack.
       *
       * If there is no substate, this method returns the top state
       *
       * @note Since the bottom state changes frequently, it is usually not
       *        very useful to call this method while the agent is running.
       *        You can do this, but the state object will probably be removed
       *        from memory before anything useful can be done with it.  The
       *        system will not crash, but results will be questionable.
       *
       * @see IAgent::GetTopState
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       * 
       * @returns A pointer to the bottom state of this object. This pointer may
       *           be 0 if an error occurs.
       */
      virtual IState* GetBottomState(Error* err = 0) = 0;

      /**
       * @brief  Gets whether or not learning is turned on for this agent
       *
       * Learning can be either on or off.  When learning is turned on, chunks 
       *  (special productions) are created when impasses are resolved by
       *  a working memory element being returned from a substate.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       * 
       * @returns true if learning is currently turned on, false if it is currently
       *            turned off.
       */
      /* TODO: Learning mode investigation */
      virtual bool IsLearningOn(Error* err = 0) = 0;

       /**
       * @brief Sets the current learning mode
       *
       * Learning can be either on or off.  You can set learning on
       *  and off at runtime.  After learning is turned on, chunks will
       *  be created until learning is turned off.  Turning learning off
       *  does not excise chunks already created.  
       *
       * Learning can also be turned on and off from productions by using
       *  the built-in RHS function: learning.  This technique is used to
       *  do learning on a per-problem space basis.
       *
       * @param on   Pass true to turn learning on. Pass false to turn learning
       *               off.
       * @param err  Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       */
      virtual void SetLearning(bool on, Error* err = 0) = 0;

      /**
         @brief Returns the current max-chunks setting for the agent

         @param err Receives error information if non-null
         @returns The max-chunks value
      */
      virtual int GetMaxChunks(Error* err = 0) = 0;

      /**
         @brief Set the current max-chunks setting for the agent

         @param maxChunks The new value
         @param err Receives error information if non-null
      */
      virtual void SetMaxChunks(int maxChunks, Error* err = 0) = 0;

      /**
         @brief Returns the current max-chunks setting for the agent

         @param err Receives error information if non-null
         @returns The max-chunks value
      */
      virtual int GetMaxElaborations(Error* err = 0) = 0;

      /**
         @brief Set the current max-chunks setting for the agent

         @param maxChunks The new value
         @param err Receives error information if non-null
      */
      virtual void SetMaxElaborations(int maxElabs, Error* err = 0) = 0;

      /**
         @brief Returns the current max-nil-output-cycles setting for the agent

         @param err Receives error information if non-null
         @returns The max-nil-output-cycles value
      */
      virtual int GetMaxNilOutputCycles(Error* err = 0) = 0;

      /**
         @brief Set the current max-nil-output-cycles setting for the agent

         @param maxNils The new value
         @param err Receives error information if non-null
      */
      virtual void SetMaxNilOutputCycles(int maxNils, Error* err = 0) = 0;
	  virtual void ResetNilOutputCounter(Error* err = 0) = 0;  // resets the gSKI local counter

      /**
       * @brief An accessor method for querying the current "Wait on state
       *        no change" setting.
       *
       * An accessor method for determining if the agent will wait when
       * a state no change occurs (instead of dropping into a sub-state).
       *
       * @param err Pointer to a client-owned error structure. IF the pointer
       *            is not NULL this structure is filled with extended error
       *            information. If it is null (the default) extended error
       *            information is not returned.
       */
      virtual bool IsWaitingOnStateNoChange(Error* err = 0) = 0;

      /**
       * @brief Sets the current wait on state no change mode
       *
       * Waiting on a state no change can be either on or off at runtime.
       * When on the agent does not drop into a substate during a state
       * no change and instead just does nothing. When off the agent drops
       * into a sub-state during a state no change.
       *
       * @param on Pass true to turn waiting on a state no change on and off
       *           to turn it off.
       * @param err Pointer to a client-owned error structure. IF the pointer
       *            is not NULL this structure is filled with extended error
       *            information. If it is null (the default) extended error
       *            information is not returned.
       */
      virtual void SetWaitOnStateNoChange(bool on, Error* err = 0) = 0;

       /**
       * @brief Returns the o-support mode for this agent
       *
       * The o-support mode determines how o-supported wmes
       *  are managed.  See egSKIOSupportMode for more details
       *  each of the modes.
       *
       * You cannot set o-support mode at runtime.  You have to set it
       *  when you create the agent (see IAgentManager::AddAgent) or when
       *  your initialize it (IAgent::Reinitialize).
       *
       * @param err  Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       * @returns The current o-support mode for this agent.
       */
      /* TODO: O-Support mode investigations */
      virtual egSKIOSupportMode GetOSupportMode(Error* err = 0) = 0;
      virtual void SetOSupportMode(egSKIOSupportMode mode, Error* err = 0) =0;

       /**
         @brief Returns the indifferent selection setting for the agent

         @param err Receives error info
         @returns The indifferent selection setting
      */
      virtual egSKIUserSelectType GetIndifferentSelection(Error* err = 0) = 0;

      /**
         @brief Set the indifferent selection setting for the agent

         @param t The new setting
         @param err Receives error info
      */
      virtual void SetIndifferentSelection(egSKIUserSelectType t, Error* err = 0) = 0;

      virtual int GetDefaultWMEDepth(Error* err = 0) = 0;
      virtual void SetDefaultWMEDepth(int d, Error* err = 0) = 0;

       /**
       * @brief Gets the current phase for this agent
       *
       * Call this method to find out what phase this agent
       *  is currently in.  You may call this method while the agent
       *  is running or while it is stopped.  Agents can be stopped
       *  in the middle of decision cycles, so an agent may remain
       *  in the middle of a decision cycle while not running.
       *
       * @param err  Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       * @returns The decision phase this agent is currently in.
       */
      virtual egSKIPhaseType GetCurrentPhase(Error* err = 0) = 0;

      /**
       * @brief Gets the current count of smallest steps executed
       *
       * Call this method when you need to know the number of smallest steps
       *  (elaboration cycles or phases, depending on the phase) that have
       *  executed since this agent was last initialized.
       *
       * @param err  Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       * @returns The number of smallest steps since the agent was last
       *             initialized.
       */
      virtual unsigned long GetNumSmallestStepsExecuted(Error* err = 0) = 0;

      
      /**
       * @brief Gets the current phase count for this agent
       *
       * Call this method when you need to know the number of phases
       *  that have executed since this agent was last initialized.
       *
       * @param err  Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       * @returns The number of phases since the agent was last
       *             initialized.
       */
      virtual unsigned long GetNumPhasesExecuted(Error* err = 0) = 0;


	 /**
       * @brief Gets the current elaboration count for this agent
       *
       * Call this method when you need to know the number of elaborations
       *  that have executed since this agent was last initialized.
       *
       * @param err  Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       * @returns The number of elaborations since the agent was last
       *             initialized.
       */
      virtual unsigned long GetNumElaborationsExecuted(Error* err = 0) = 0;


      /**
       * @brief Gets the current decision cycle count
       *
       * Call this method when you need to know the number of decision cycles
       *  this agent has executed since it was initialized (or reinitiailzed)
       *
       * @param err  Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       * @returns The number of decision cycles since the agent was last
       *             initialized.
       */
      virtual unsigned long GetNumDecisionCyclesExecuted(Error* err = 0) = 0;

      /**
       * @brief Gets the current output count for this agent
       *
       * Call this method when you need to know the number of decision cycles
       *  in which output has occured sing this agent was last initialized.
       *
       * @param err  Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       * @returns The number of decision cycles with output since the agent was last
       *             initialized.
       */
      virtual unsigned long GetNumOutputsExecuted(Error* err = 0) = 0;

      /**
         @brief Returns the agent's preformance monitor object

      */
      virtual IAgentPerformanceMonitor* GetPerformanceMonitor(Error* err = 0) = 0;

      /**
       * @brief Adds a client-defined rhs function to this agent.
       *
       * Call this method to add a new right hand side (RHS) function to
       *   this agent.  This function will be immediately callable from
       *   any this agent.
       *
       * You will probably want to call this method soon after creating
       *  the agents.  Re-initializing the agent will NOT remove the
       *  rhs functions that were added.
       *
       * Rhs functions  must be uniquely named within an agent.  This is because the
       *  name is used in the soar code to reference it. Duplicate RHS functions
       *  would result in an ambiguity from the soar code.
       *
       * @Note Adding a RHS function through a remote transport layer is not
       *        a good idea.  RHS functions only make sense within the kernel
       *        process (they need to be fast).
       *
       *  Possible Errors:
       *     @li gSKIERR_INVALID_PTR -- If you pass an invalid pointer for a listener.
       *     @li gSKIERR_RHS_FUNCTION_ALREADY_EXISTS -- If a rhs function of the same name
       *                      was already added to this agent.
       *
       * @param rhsFunction Pointer to a RHS function object.  This object has its Execute method 
       *                      called back whenever a soar production fires that calls this function
       *                      in its RHS actions.  This pointer should not be 0 (will generate
       *                      an assertion in debug mode).  In release mode nothing happens
       *                      and false is returned if this parameter is 0.
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @returns true if the RHS function was added.  False if there is already a RHS function
       *             with the given name or if there is another failure.
       */
      virtual bool AddClientRhsFunction(IRhsFunction* rhsFunction, 
                                  Error*        err = 0) = 0;

      /**
       *  @brief Remove a RHS function from this agent
       *
       *  You can only remove client-defined rhs functions.  Built in RHS functions
       *   cannot be removed.
       *  
       *  Possible Errors:
       *     @li gSKIERROR_NO_SUCH_RHS_FUNCTION
       *     @li gSKIERR_RHS_FUNCTION_IN_USE
       *
       *  @param  name Name of the RHS function to remove.
       *  @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       *  @return true if the given RHS function was removed.  false otherwise.  If false is returned
       *               check the value of err for extended error information.
       */
      virtual bool RemoveClientRhsFunction(const char* name, Error* err = 0) = 0;

      /**
       *  @brief Remove all client-defined RHS functions from this agent
       *
       *  Call this method tor remove ALL client-defined RHS functions from this
       *   agent.  
       *  
       *  @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       */
      virtual void RemoveAllClientRhsFunctions(Error* err = 0) = 0;

      /*************************** Listeners *********************************/

      /**
      *  @brief Adds a listener for agent running events
      *
      *  Call this method to register a listener to recieve system events.
      *  Agent run events are:
      *     @li gSKIEVENT_BEFORE_SMALLEST_STEP
      *     @li gSKIEVENT_AFTER_SMALLEST_STEP
      *     @li gSKIEVENT_BEFORE_ELABORATION_CYCLE
      *     @li gSKIEVENT_AFTER_ELABORATION_CYCLE
      *     @li gSKIEVENT_BEFORE_PHASE_EXECUTED
      *     @li gSKIEVENT_AFTER_PHASE_EXECUTED
      *     @li gSKIEVENT_BEFORE_DECISION_CYCLE
      *     @li gSKIEVENT_AFTER_DECISION_CYCLE
      *     @li gSKIEVENT_AFTER_INTERRUPT
      *     @li gSKIEVENT_BEFORE_RUNNING
      *     @li gSKIEVENT_AFTER_RUNNING
      *
      *  If this listener has already been added for the given event, nothing happens
      *
      *  Possible Errors:
      *     @li gSKIERR_INVALID_PTR -- If you pass an invalid pointer for a listener.
      *
      *  @param eventId  One of the valid event ids listed above
      *  @param listener A pointer to a client owned listener that will be called back when
      *                      an event occurs.  Because the listener is client owned, it is not
      *                      cleaned up by the kernel when it shuts down.  The same listener
      *                      can be registered to recieve multiple events.  If this listener
      *                      is 0, no listener is added and an error is recored in err.
      *  @param allowAsynch A flag indicating whether or not it is ok for the listener to be
      *                         notified asynchonously of system operation.  If you specify "true"
      *                         the system may not callback the listener until some time after
      *                         the event occurs. This flag is only a hint, the system may callback
      *                         your listener synchronously.  The main purpose of this flag is to
      *                         allow for efficient out-of-process implementation of event callbacks
      *  @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      */
      virtual void AddRunListener(egSKIRunEventId     eventId, 
                                  IRunListener*       listener, 
                                  bool                allowAsynch = false,
                                  Error*              err         = 0) = 0;

     /**
      *  @brief Removes an agent running event listener
      *
      *  Call this method to remove a previously added event listener.
      *  The system will automatically remove all listeners when the kernel shutsdown;
      *   however, since all listeners are client owned, the client is responsible for
      *   cleaning up memory used by listeners.
      *
      *  If the given listener is not registered to recieve the given event, this
      *     function will do nothing (but a warning is logged).
      *
      *  Agent run events are:
      *     @li gSKIEVENT_BEFORE_SMALLEST_STEP
      *     @li gSKIEVENT_AFTER_SMALLEST_STEP
      *     @li gSKIEVENT_BEFORE_ELABORATION_CYCLE
      *     @li gSKIEVENT_AFTER_ELABORATION_CYCLE
      *     @li gSKIEVENT_BEFORE_PHASE_EXECUTED
      *     @li gSKIEVENT_AFTER_PHASE_EXECUTED
      *     @li gSKIEVENT_BEFORE_DECISION_CYCLE
      *     @li gSKIEVENT_AFTER_DECISION_CYCLE
      *     @li gSKIEVENT_AFTER_INTERRUPT
      *     @li gSKIEVENT_BEFORE_RUNNING
      *     @li gSKIEVENT_AFTER_RUNNING
      *
      *  Possible Errors:
      *     @li gSKIERR_INVALID_PTR -- If you pass an invalid pointer for a listener.
      *
      *  @param eventId  One of the valid event ids listed above
      *  @param listener A pointer to the listener you would like to remove.  Passing a 0
      *                     pointer causes nothing to happen except an error being recorded
      *                     to err.
      *  @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      */
      virtual void RemoveRunListener(egSKIRunEventId      eventId,
                                     IRunListener*        listener,
                                     Error*               err = 0) = 0;

      /**
      *  @brief Adds a listener for Rhs Function change events
      *
      *  Call this method to register a listener to recieve right hand side function change events.
      *  Rhs function change events are:
      *     @li gSKIEVENT_RHS_FUNCTION_ADDED
      *     @li gSKIEVENT_RHS_FUNCTION_REMOVED
      *
      *  If this listener has already been added for the given event, nothing happens
      *
      *  Possible Errors:
      *    @li gSKIERR_INVALID_PTR -- If you pass an invalid pointer for a listener.
      *
      *  @param eventId  One of the valid event ids listed above
      *  @param listener A pointer to a client owned listener that will be called back when
      *                      an event occurs.  Because the listener is client owned, it is not
      *                      cleaned up by the kernel when it shuts down.  The same listener
      *                      can be registered to recieve multiple events.  If this listener
      *                      is 0, no listener is added and an error is recored in err.
      *  @param allowAsynch A flag indicating whether or not it is ok for the listener to be
      *                         notified asynchonously of system operation.  If you specify "true"
      *                         the system may not callback the listener until some time after
      *                         the event occurs. This flag is only a hint, the system may callback
      *                         your listener synchronously.  The main purpose of this flag is to
      *                         allow for efficient out-of-process implementation of event callbacks
      *  @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      */
      virtual void AddRhsFunctionChangeListener(egSKISystemEventId           eventId, 
                                                IRhsFunctionChangeListener*  listener, 
                                                bool                         allowAsynch = false,
                                                Error*                       err         = 0) = 0;

      /**
      *  @brief Removes a Rhs function change event listener
      *
      *  Call this method to remove a previously added event listener.
      *  The system will automatically remove all listeners when the kernel shutsdown;
      *   however, since all listeners are client owned, the client is responsible for
      *   cleaning up memory used by listeners.
      *
      *  If the given listener is not registered to recieve the given event, this
      *     function will do nothing (but a warning is logged).
      *
      *  Rhs function change events are:
      *     @li gSKIEVENT_RHS_FUNCTION_ADDED
      *     @li gSKIEVENT_RHS_FUNCTION_REMOVED
      *
      *  Possible Errors:
      *     @li gSKIERR_INVALID_PTR -- If you pass an invalid pointer for a listener.
      *
      *  @param eventId  One of the valid event ids listed above
      *  @param listener A pointer to the listener you would like to remove.  Passing a 0
      *                     pointer causes nothing to happen except an error being recorded
      *                     to err.
      *  @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      */
      virtual void RemoveRhsFunctionChangeListener(egSKISystemEventId                 eventId,
                                                   IRhsFunctionChangeListener*  listener,
                                                   Error*                       err = 0) = 0;

      /**
      *  @brief Adds a listener for Rhs Function execution events
      *
      *  Call this method to register a listener to recieve right hand side function execution events.
      *  Rhs function execution events are:
      *     @li gSKIEVENT_RHS_FUNCTION_EXECUTED
      *
      *  If this listener has already been added for the given event, nothing happens
      *
      *  Possible Errors:
      *    @li gSKIERR_INVALID_PTR -- If you pass an invalid pointer for a listener.
      *
      *  @param eventId  One of the valid event ids listed above
      *  @param listener A pointer to a client owned listener that will be called back when
      *                      an event occurs.  Because the listener is client owned, it is not
      *                      cleaned up by the kernel when it shuts down.  The same listener
      *                      can be registered to recieve multiple events.  If this listener
      *                      is 0, no listener is added and an error is recored in err.
      *  @param allowAsynch A flag indicating whether or not it is ok for the listener to be
      *                         notified asynchonously of system operation.  If you specify "true"
      *                         the system may not callback the listener until some time after
      *                         the event occurs. This flag is only a hint, the system may callback
      *                         your listener synchronously.  The main purpose of this flag is to
      *                         allow for efficient out-of-process implementation of event callbacks
      *  @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
       */
      virtual void AddRhsFunctionListener(egSKISystemEventId    eventId, 
                                          IRhsFunctionListener* listener, 
                                          bool                  allowAsynch = false,
                                          Error*                err         = 0) = 0;

      /**
      * @brief Adds a name filter to a given RHS function execution event
      *
      * Call this method when you want to recieve RHS function execution event
      *  callbacks for a particular set of functions (possibly only 1).  
      *
      * The given listener will only recieve event notifications for RHS
      *  functions that match the given name pattern (regular expression).
      *
      *  All listener filters are conjuncted or "ANDed" together.  This means that
      *   an event must pass successfully through all of the filters for a listener 
      *   or it will not be sent to that listener.
      *
      *  E.g. Calling this method with szRhsFuncNamePattern = "^max$" results in the
      *    listener only recieving callbacks when the "max" RHS function executes.
      *
      *  E.g. Calling this method with szRhsFuncNamePattern = "max|min" results in the
      *    listener only recieving callbacks for rhs functions named "max" and "min" or
      *    functions that have "min" or "max" somewhere in their name.
      *
      *  Rhs function execution events are:
      *     @li gSKIEVENT_RHS_FUNCTION_EXECUTED
      *
      *  Possible Errors:
      *     @li gSKIERR_INVALID_PTR -- If you pass an invalid pointer for a listener or name pattern.
      *
      *  @param eventId  One of the valid event ids listed above
      *  @param listener The listener for which to filter events. 
      *  @param rhsFuncNamePattern Name pattern (regular expression) for the RHS function(s)
      *                               for which you would like to recieve event callbacks.
      *                               Passing in 0 results in no filter being created.
      *  @param  negate If true, the listener will recieve the callback if the pattern
      *                    does NOT  match.
      *  @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      */
      virtual void AddRhsFunctionListenerNameFilter(egSKISystemEventId    eventId,
                                                    IRhsFunctionListener* listener,
                                                    const char*           rhsFuncNamePattern,
                                                    bool                  negate      = false,
                                                    Error*                err         = 0) = 0;
      
      /**
      *  @brief Removes a Rhs function execution event listener
      *
      *  Call this method to remove a previously added event listener.
      *  The system will automatically remove all listeners when the kernel shutsdown;
      *   however, since all listeners are client owned, the client is responsible for
      *   cleaning up memory used by listeners.
      *
      *  If the given listener is not registered to recieve the given event, this
      *     function will do nothing (but a warning is logged).
      *
      *  Rhs function execution events are:
      *     @li gSKIEVENT_RHS_FUNCTION_EXECUTED
      *
      *  @param eventId  One of the valid system event ids listed above
      *  @param listener A pointer to the listener you would like to remove.  Passing a 0
      *                     pointer causes nothing to happen except an error being recorded
      *                     to err.
      *  @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      */
      virtual void RemoveRhsFunctionListener(egSKISystemEventId     eventId,
                                             IRhsFunctionListener*  listener,
                                             Error*                 err = 0) = 0;

      /**
      *  @brief Removes all filters on the given listener for the given event.
      *
      *  Call this method to remove listener filters on a particular event and listener.
      *   The system will automatically clean up all filters during shutdown, so you only
      *   need to call this method if you are dynamically changing the filters on a listener
      *   at runtime (e.g. in a debugger).
      *
      *  If the given event has no filters on it, nothing happens (but a warning is logged).
      *
      *  Rhs function execution events are:
      *     @li gSKIEVENT_RHS_FUNCTION_EXECUTED
      *
      *  @param eventId  One of the valid system event ids listed above
      *  @param listener A pointer to the listener for which you would like to remove filters.  
      *                     Passing a 0 pointer causes nothing to happen except an error
      *                     being recorded to err.
      *  @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      */
      virtual void RemoveRhsFunctionListenerFilters(egSKISystemEventId    eventId,
                                                    IRhsFunctionListener* listener,
                                                    Error*                err = 0) = 0;


	  /**
      *  @brief Adds a listener for agent XML trace events
      *
      *  Call this method to register a listener to recieve system events.
      *  Agent run events are:
      *     @li gSKIEVENT_XML_TRACE_OUTPUT
      *
      *  If this listener has already been added for the given event, nothing happens
      *
      *  Possible Errors:
      *     @li gSKIERR_INVALID_PTR -- If you pass an invalid pointer for a listener.
      *
      *  @param eventId  One of the valid event ids listed above
      *  @param listener A pointer to a client owned listener that will be called back when
      *                      an event occurs.  Because the listener is client owned, it is not
      *                      cleaned up by the kernel when it shuts down.  The same listener
      *                      can be registered to recieve multiple events.  If this listener
      *                      is 0, no listener is added and an error is recored in err.
      *  @param allowAsynch A flag indicating whether or not it is ok for the listener to be
      *                         notified asynchonously of system operation.  If you specify "true"
      *                         the system may not callback the listener until some time after
      *                         the event occurs. This flag is only a hint, the system may callback
      *                         your listener synchronously.  The main purpose of this flag is to
      *                         allow for efficient out-of-process implementation of event callbacks
      *  @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      */
	  virtual void AddXMLListener(egSKIXMLEventId	 eventId, 
                            IXMLListener*            listener, 
                            bool                     allowAsynch = false,
                            Error*                   err         = 0) = 0;

	  /**
      *  @brief Removes an agent XML trace event listener
      *
      *  Call this method to remove a previously added event listener.
      *  The system will automatically remove all listeners when the kernel shutsdown;
      *   however, since all listeners are client owned, the client is responsible for
      *   cleaning up memory used by listeners.
      *
      *  If the given listener is not registered to recieve the given event, this
      *     function will do nothing (but a warning is logged).
      *
      *  Agent run events are:
      *     @li gSKIEVENT_XML_TRACE_OUTPUT
      *
      *  Possible Errors:
      *     @li gSKIERR_INVALID_PTR -- If you pass an invalid pointer for a listener.
      *
      *  @param eventId  One of the valid event ids listed above
      *  @param listener A pointer to the listener you would like to remove.  Passing a 0
      *                     pointer causes nothing to happen except an error being recorded
      *                     to err.
      *  @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
	  */
      virtual void RemoveXMLListener(egSKIXMLEventId	eventId,
                               IXMLListener*			listener,
                               Error*					err = 0) = 0;

      /** 
       * @brief Print callback support
       *
       * These are deprecated.  Do not use them.
       */ 
      virtual void AddPrintListener(egSKIPrintEventId        eventId, 
                                    IPrintListener*          listener, 
                                    bool                     allowAsynch = false,
                                    Error*                   err         = 0) = 0;

      virtual void RemovePrintListener(egSKIPrintEventId     eventId,
                                       IPrintListener*       listener,
                                       Error*                err = 0) = 0;

	  /** Fire the gSKIEVENT_BEFORE_RUN_STARTS event **/
	  virtual void FireRunStartsEvent() = 0 ;

	  /** Fire the gSKIEVENT_AFTER_RUN_ENDS event **/
	  virtual void FireRunEndsEvent() = 0 ;

	  // Called when a "RunEvent" occurs in the kernel
	  virtual void HandleEvent(egSKIRunEventId eventId, gSKI::IAgent* agentPtr, egSKIPhaseType phase) = 0 ;
	  
	  // Multi-attribute related methods
   
      /**
         Returns a list of all multi-attributes that have been set with the
         SetMultiAttribute method.

         @param pErr Receives error info
         @returns An iterator.
      */
      virtual tIMultiAttributeIterator* GetMultiAttributes(Error* pErr = 0) = 0;

      /**
         Look up a multi-attribute by name.

         The caller MUST Release the returned object.

         @param attribute The attribute name.
         @param pErr Receive error info
         @returns The multi-attribute object or 0 if none exists for the given name.
      */
      virtual IMultiAttribute* GetMultiAttribute(const char* attribute, Error* pErr = 0) = 0;

      /**
         Sets a new multi-attribute or modifies an existing one.

         @param attribute The name of the attribute
         @param priority The RETE matching priority
         @param pErr Receives error info
      */
      virtual void SetMultiAttribute(const char* attribute, 
                                     int priority,
                                     Error* pErr = 0) = 0;

      /**
         Returns the current numeric indifferent mode.

         @param pErr Receives error info
         @returns the current numeric indifferent mode
      */
      virtual egSKINumericIndifferentMode GetNumericIndifferentMode(Error* pErr = 0) = 0;

      /**
         Sets the numeric indifferent mode for the agent

         @param m the mode to set the agent to
         @param pErr Receives error info
      */
      virtual void SetNumericIndifferentMode(egSKINumericIndifferentMode m, Error* pErr = 0) = 0;

	  virtual bool GetOperand2Mode() = 0;
	  virtual void SetOperand2Mode(bool mode) = 0;

	  virtual int GetAttributePreferencesMode(Error* err = 0) = 0;
	  virtual void SetAttributePreferencesMode(int mode, Error* err = 0) = 0;

	  virtual int GetInputPeriod(Error* err = 0) = 0;
	  virtual void SetInputPeriod(int mode, Error* err = 0) = 0;
   };

   //
   //
   // NOTE!  THIS IS A WORKAROUND HACK!!!  THIS SHOULD NOT STAY HERE!!!
   //        IT IS NEEDED FOR THE TSI UNTIL IT CAN GET REWRITTEN!
   //
   //
   extern agent* GetSoarAgentPtr(IAgent* agent);

}


#endif