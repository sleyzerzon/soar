/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_kernel.h 
*********************************************************************
* created:	   6/27/2002   10:38
*
* purpose: 
*********************************************************************/
#ifndef GSKI_KERNEL_H
#define GSKI_KERNEL_H

#include "IgSKI_Kernel.h"
#include "gSKI_Events.h"
#include "EventManagementTemplates.h"

typedef struct kernel_struct kernel;

namespace gSKI 
{
   class InstanceInfo;
   class AgentManager;
   class Log;
   class IKernelFactory;

   /**
    * @brief: The implementation of the kernel interface.  This class is used
    *         to allow access to kernel level functionality, and there will
    *         normally be only one of these per system, but the is not 
    *         required.
    */
   class Kernel : public IKernel
   {
    public:
       /**
        * @brief: Constructor of Kernel.
        */
       Kernel(const IKernelFactory* kf);

      /**
       * @brief Destructor for the kernel.
       * 
       * The destructor for the kernel makes sure everything held by the kernel is
       *  cleaned up (including agents).  It will not clean up client owned elements
       *  like cloned WMEs and KernelFactories
       */
       ~Kernel();

      /**
        * @brief Returns a pointer to the Kernel Factory that created it.
        *
        * @returns The pointer to the creating Kernel Factory.
        */
       const IKernelFactory* GetKernelFactory(Error* err = 0) const ;

      /**
       * @brief  Returns the agent manager for this kernel.
       *
       * Use the agent manager to add and remove agents as well as 
       *  set up the agents to run, etc.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @return A pointer to the agent manager.  The pointer may be 0 if there
       *          is a failure.
       */
      IAgentManager* GetAgentManager(Error* err = 0);

      /**
       * @brief Returns the performance monitor for this kernel
       *
       * Use the performance monitor to look at timing and memory usage by 
       *  the kernel and its agents.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @returns A pointer to the performance monitor for this kernel.  This pointer
       *           may be 0 if there is a failure.
       */
      /* TODO: This interface needs to be defined */
      IPerformanceMonitor* GetPerformanceMonitor(Error* err = 0);

      /**
       * @brief Returns the instance information for this instance of the kernel
       *
       * Use the instance information to discover the threading model, process model
       *  and other instance information for this instance of the kernel.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @returns A pointer to an InstanceInfo object containing information on 
       *           this kernel instance.  This pointer may be 0 if there is
       *           a failure.
       */
      IInstanceInfo* GetInstanceInformation(Error* err = 0);

      /**
       * @brief    Get the current log location
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @returns  A c-style string with the path where debug logs are being kept.
       *            The format of the path follows the format of file paths on the server
       *            machine.  If there are no debug logs or an unrecoverable
       *            error occurs, 0 is returned.
       */
      const char* GetLogLocation(Error* err = 0) const;

      /**
       * @brief    Sets the activity level of the debug log output
       *
       * The following are all possible debug log activity settings:
       *
       *  @li gSKI_LOG_NONE     : No logs are generated
       *  @li gSKI_LOG_ERRORS   : Only errors are logged
       *  @li gSKI_LOG_ALL      : Everything is logged (errors, warnings, info, debug)
       *  @li gSKI_LOG_ALL_EXCEPT_DEBUG : Everything except debug-only messages are logged
       *
       * The debug logs files are named as follows:
       *     @li gSKIError.txt   : for errors (critical problems)
       *     @li gSKIWarning.txt : for warnings (non-critical problems)
       *     @li gSKIInfo.txt    : for general information
       *     @li gSKIDebug.txt   : for debug information
       *
       * @param eALevel The desired debug log activity level.  This setting will effect
       *                  both the output file logs and the debug log messages routed
       *                  through the event system.
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       */
      void SetLogActivityLevel(egSKILogActivityLevel eALevel,
                                    Error*                err = 0);

      /**
       * @brief Gets the current debug log activity level
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @returns The current debug log activity level.  If there is a connection
       *            failure, the return value is undefined.
       */
      egSKILogActivityLevel GetLogActivityLevel(Error* err) const;

      /********************** LISTENERS ***********************************/

      /**
      *  @brief Adds a listener for system events
      *
      *  Call this method to register a listener to recieve system events.
      *  System events are:
      *     @li gSKIEVENT_SHUTDOWN
      *     @li gSKIEVENT_RESTART
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
      void AddSystemListener(egSKIEventId        eventId, 
                                     ISystemListener*    listener, 
                                     bool                allowAsynch = false,
                                     Error*              err         = 0);

      /**
      *  @brief Removes a system event listener
      *
      *  Call this method to remove a previously added event listener.
      *  The system will automatically remove all listeners when the kernel shutsdown;
      *   however, since all listeners are client owned, the client is responsible for
      *   cleaning up memory used by listeners.
      *
      *  If the given listener is not registered to recieve the given event, this
      *     function will do nothing (but a warning is logged).
      *
      *  System events are:
      *     @li gSKIEVENT_SHUTDOWN
      *     @li gSKIEVENT_RESTART
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
      void RemoveSystemListener(egSKIEventId         eventId,
                                        ISystemListener*     listener,
                                        Error*               err = 0);

      /**
      *  @brief Adds a listener for rhs (right hand side) user functions
      *
      *  Call this method to register a listener to receive rhs user functions.
	  *
	  *  This event is unusual in that the listener is providing the implementation of
	  *  the function.  The arguments are passed in a single string and the return value
	  *  is also a string.  If multiple listeners register, the first to return a value will
	  *  stop others from being called.
	  *
      *  RHS user events are:
      *     @li gSKIEVENT_RHS_USER_FUNCTION
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
      virtual void AddRhsListener(egSKIEventId        eventId, 
                                  IRhsListener*		  listener, 
                                  bool                allowAsynch = false,
                                  Error*              err         = 0) ;

	  /**************************************************
	   *
	   * Notify listeners about a RHS user function firing.
	   * The listeners can provide the return value for this function.
	   *
	   * If this function returns true, pReturnValue should be filled in with the return value.
	   * maxReturnValueLength indicates the size of the pReturnValue buffer (which is allocated by the
	   * caller of this function not the listener who responds).
	   *
	   * If "commandLine" is true then we will execute this with the command line processor
	   * rather than a generic user function that the user provides.
	   *
	   **************************************************/
	  virtual bool FireRhsNotification(IAgent* pAgent, bool commandLine, char const* pFunctionName, char const* pArgument,
									   int maxReturnValueLength, char* pReturnValue) ;

      /**
      *  @brief Removes a rhs listener
      *
      *  Call this method to remove a previously added event listener.
      *  The system will automatically remove all listeners when the kernel shutsdown;
      *   however, since all listeners are client owned, the client is responsible for
      *   cleaning up memory used by listeners.
      *
      *  If the given listener is not registered to receive the given event, this
      *     function will do nothing (but a warning is logged).
      *
      *  System events are:
      *     @li gSKIEVENT_RHS_USER_FUNCTION
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
      virtual void RemoveRhsListener(egSKIEventId      eventId,
                                     IRhsListener*     listener,
                                     Error*            err = 0) ;

      /**
      *  @brief Adds a listener for debug log events
      *
      *  Call this method to register a listener to recieve debug log events.
      *  Debug Log events are:
      *     @li gSKIEVENT_LOG_ERROR
      *     @li gSKIEVENT_LOG_WARNING
      *     @li gSKIEVENT_LOG_INFO
      *     @li gSKIEVENT_LOG_DEBUG
      *
      *  If this listener has already been added for the given event, nothing happens
      *
      *  Possible Errors:
      *    @li gSKIERR_INVALID_PTR -- If you pass an invalid pointer for a listener.
      *
      *  @param eventId  One of the valid system ids listed above
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
      void AddLogListener(egSKIEventId   eventId, 
                          ILogListener*  listener, 
                          bool           allowAsynch = false,
                          Error*         err         = 0);
   
      /**
      *  @brief Removes a debug log event listener
      *
      *  Call this method to remove a previously added event listener.
      *  The system will automatically remove all listeners when the kernel shutsdown;
      *   however, since all listeners are client owned, the client is responsible for
      *   cleaning up memory used by listeners.
      *
      *  If the given listener is not registered to recieve the given event, this
      *     function will do nothing (but a warning is logged).
      *
      *  Debug Log events are:
      *     @li gSKIEVENT_LOG_ERROR
      *     @li gSKIEVENT_LOG_WARNING
      *     @li gSKIEVENT_LOG_INFO
      *     @li gSKIEVENT_LOG_DEBUG
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
      void RemoveLogListener(egSKIEventId   eventId,
                             ILogListener*  listener,
                             Error*         err = 0);

      /**
      *  @brief Adds a listener for connection lost events
      *
      *  Call this method to register a listener to recieve connection lost events.
      *  Connection lost events are:
      *     @li gSKIEVENT_CONNECTION_LOST
      *
      *  Connections can only be lost with out-of-process communications.  You can use
      *   this callback to do whatever you need to do in the client to recover from
      *   a lost connection.
      *
      *  If this listener has already been added for the given event, nothing happens
      *
      *  Possible Errors:
      *    @li gSKIERR_INVALID_PTR -- If you pass an invalid pointer for a listener.
      *
      *  @param eventId  One of the valid system ids listed above
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
      void AddConnectionLostListener(egSKIEventId             eventId, 
                                             IConnectionLostListener* listener, 
                                             bool                     allowAsynch = false,
                                             Error*                   err         = 0);

   
      /**
      *  @brief Removes a connection lost event listener
      *
      *  Call this method to remove a previously added event listener.
      *  The system will automatically remove all listeners when the kernel shutsdown;
      *   however, since all listeners are client owned, the client is responsible for
      *   cleaning up memory used by listeners.
      *
      *  If the given listener is not registered to recieve the given event, this
      *     function will do nothing (but a warning is logged).
      *
      *  Connection lost events are:
      *     @li gSKIEVENT_CONNECTION_LOST
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
      void RemoveConnectionLostListener(egSKIEventId             eventId,
                                        IConnectionLostListener* listener,
                                        Error*                   err = 0);
      public:

         /**
          * @brief: Fetch the logger.
          */
         Log* GetLogger()
         {
            return m_log;
         }

         /** 
         * @brief Event notifier for print callback
         */
         class LogNotifier {
         public:
            LogNotifier(IKernel* kernel, const char* msg): m_msg(msg), m_kernel(kernel){}

            void operator()(egSKIEventId eventId, 
                            ILogListener* listener) const
            {
               listener->HandleEvent(eventId, m_kernel, m_msg);
            }
         private:
            const char *      m_msg;
            IKernel*          m_kernel;
         };

		 // When using this should call NotifyGetResult() not just Notify().
		 class RhsNotifier {
         public:
            RhsNotifier(IAgent* agent, bool commandLine, char const* pFunctionName, char const* pArgument,
					    int maxLengthReturnValue, char* pReturnValue):
						m_Agent(agent), m_CommandLine(commandLine), m_FunctionName(pFunctionName), m_Argument(pArgument),
						m_MaxLength(maxLengthReturnValue), m_ReturnValue(pReturnValue)
						{}

			bool operator()(egSKIEventId eventId, 
                            IRhsListener* listener) const
            {
				bool haveResult = listener->HandleEvent(eventId, m_Agent, m_CommandLine, m_FunctionName, m_Argument, m_MaxLength, m_ReturnValue) ;
				return haveResult ;
            }
         private:
			IAgent*			m_Agent ;
			bool			m_CommandLine ;
			char const*		m_FunctionName ;
			char const*		m_Argument ;
			int				m_MaxLength ;
			char*			m_ReturnValue ;
         };

         /** 
          * @brief Listener manager definitions 
          */
         typedef ListenerManager<ILogListener, LogNotifier>   tLogListenerManager;
         typedef ListenerManager<IRhsListener, RhsNotifier>   tRhsListenerManager;

         /**
          * @brief Retrieves the Soar Kernel Pointer
          */
         kernel* GetSoarKernel() { return m_soarKernel; };

         /**
         * @brief Release this object
         * 
         * Call this method to remove this object from memory.  The pointer used to call Release
         *  will no longer be valid after the Release method returns.
         */
         void Release(Error* err = 0);

         // TODO: Properly implement this function
         bool IsClientOwned(Error* err = 0) const;

      private: 

         /** */
         InstanceInfo*               m_instanceInfo;
         
         /** */
         AgentManager*               m_agentMgr;

         /** */
         tLogListenerManager         m_logListeners;

		 /** */
         tRhsListenerManager         m_rhsListeners;

         /** */
         Log*                        m_log;            /**< The log file nanager */
         
         /** */
	      egSKILogActivityLevel       m_logLevel;       /**< The level of current debugging. */

         /** */
         kernel*                     m_soarKernel;

         /** */
         const IKernelFactory*       m_kF;

      private:
         EvilBackDoor::ITgDWorkArounds* getWorkaroundObject();

   };
}
#endif





