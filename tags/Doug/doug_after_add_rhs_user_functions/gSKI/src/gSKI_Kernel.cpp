#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_kernel.cpp 
*********************************************************************
* created:	   6/27/2002   10:44
*
* purpose: 
*********************************************************************/
#include "gSKI_Kernel.h"
#include "gSKI_Error.h"
#include "gSKI_InstanceInfo.h"
#include "gSKI_AgentManager.h"
#include "gSKI_EnumRemapping.h"
#include "kernel_struct.h"
#include "MegaAssert.h"
#include "Log.h"

//#include "MegaUnitTest.h"
//DEF_EXPOSE(gSKI_Kernel);

//
// HACK!
#include "gSKI_DoNotTouch.h"

namespace gSKI
{

   /*
   =========================
 _  __                    _
| |/ /___ _ __ _ __   ___| |
| ' // _ \ '__| '_ \ / _ \ |
| . \  __/ |  | | | |  __/ |
|_|\_\___|_|  |_| |_|\___|_|
   =========================
   */
   Kernel::Kernel(const IKernelFactory* kf) : m_log(0), m_kF(kf)
   {
      m_soarKernel   = create_kernel();
      init_soar( m_soarKernel );
      m_instanceInfo = new InstanceInfo("test1", "test2", gSKI_IN_PROCESS, gSKI_SINGLE_THREAD);
      m_agentMgr     = new AgentManager(this);
   }

   /*
   ==================================
 /\/|  __                    _
|/\/ |/ /___ _ __ _ __   ___| |
   | ' // _ \ '__| '_ \ / _ \ |
   | . \  __/ |  | | | |  __/ |
   |_|\_\___|_|  |_| |_|\___|_|
   ==================================
   */
   Kernel::~Kernel()
   {
      delete m_log; m_log = 0;
      delete m_agentMgr; m_agentMgr = 0;
      delete m_instanceInfo; m_instanceInfo = 0;
      destroy_kernel(m_soarKernel);
      m_soarKernel = 0;
   }

   /*
   =========================
  ____      _      _                    _   __  __
 / ___| ___| |_   / \   __ _  ___ _ __ | |_|  \/  | __ _ _ __   __ _  __ _  ___ _ __
| |  _ / _ \ __| / _ \ / _` |/ _ \ '_ \| __| |\/| |/ _` | '_ \ / _` |/ _` |/ _ \ '__|
| |_| |  __/ |_ / ___ \ (_| |  __/ | | | |_| |  | | (_| | | | | (_| | (_| |  __/ |
 \____|\___|\__/_/   \_\__, |\___|_| |_|\__|_|  |_|\__,_|_| |_|\__,_|\__, |\___|_|
                       |___/                                         |___/
   =========================
   */
   IAgentManager* Kernel::GetAgentManager(Error* err)
   {
      ClearError(err);

      return(m_agentMgr); 
   }


   /*
   =========================
  ____      _   _  __                    _ _____          _
 / ___| ___| |_| |/ /___ _ __ _ __   ___| |  ___|_ _  ___| |_ ___  _ __ _   _
| |  _ / _ \ __| ' // _ \ '__| '_ \ / _ \ | |_ / _` |/ __| __/ _ \| '__| | | |
| |_| |  __/ |_| . \  __/ |  | | | |  __/ |  _| (_| | (__| || (_) | |  | |_| |
 \____|\___|\__|_|\_\___|_|  |_| |_|\___|_|_|  \__,_|\___|\__\___/|_|   \__, |
                                                                        |___/
   =========================
   */
   const IKernelFactory* Kernel::GetKernelFactory(Error* err) const
   {
      return m_kF;
   }


   /*
   =========================
  ____      _   ____            __
 / ___| ___| |_|  _ \ ___ _ __ / _| ___  _ __ _ __ ___   __ _ _ __   ___ ___
| |  _ / _ \ __| |_) / _ \ '__| |_ / _ \| '__| '_ ` _ \ / _` | '_ \ / __/ _ \
| |_| |  __/ |_|  __/  __/ |  |  _| (_) | |  | | | | | | (_| | | | | (_|  __/
 \____|\___|\__|_|  _\___|_|  |_|  \___/|_|  |_| |_| |_|\__,_|_| |_|\___\___|
|  \/  | ___  _ __ (_) |_ ___  _ __
| |\/| |/ _ \| '_ \| | __/ _ \| '__|
| |  | | (_) | | | | | || (_) | |
|_|  |_|\___/|_| |_|_|\__\___/|_|
   =========================
   */
   IPerformanceMonitor* Kernel::GetPerformanceMonitor(Error* err)
   {
      ClearError(err);
   
      return 0;
   }

   /*
   =========================
  ____      _   ___           _
 / ___| ___| |_|_ _|_ __  ___| |_ __ _ _ __   ___ ___
| |  _ / _ \ __|| || '_ \/ __| __/ _` | '_ \ / __/ _ \
| |_| |  __/ |_ | || | | \__ \ || (_| | | | | (_|  __/
 \____|\___|\__|___|_| |_|___/\__\__,_|_| |_|\___\___|
|_ _|_ __  / _| ___  _ __ _ __ ___   __ _| |_(_) ___  _ __
 | || '_ \| |_ / _ \| '__| '_ ` _ \ / _` | __| |/ _ \| '_ \
 | || | | |  _| (_) | |  | | | | | | (_| | |_| | (_) | | | |
|___|_| |_|_|  \___/|_|  |_| |_| |_|\__,_|\__|_|\___/|_| |_|
      =========================
   */
   IInstanceInfo* Kernel::GetInstanceInformation(Error* err)
   {
      ClearError(err);
   
      return m_instanceInfo;
   }




   /*
   =========================
  ____      _   ____       _                 _
 / ___| ___| |_|  _ \  ___| |__  _   _  __ _| |    ___   __ _
| |  _ / _ \ __| | | |/ _ \ '_ \| | | |/ _` | |   / _ \ / _` |
| |_| |  __/ |_| |_| |  __/ |_) | |_| | (_| | |__| (_) | (_| |
 \____|\___|\__|____/ \___|_.__/ \__,_|\__, |_____\___/ \__, |
| |    ___   ___ __ _| |_(_) ___  _ __ |___/            |___/
| |   / _ \ / __/ _` | __| |/ _ \| '_ \
| |__| (_) | (_| (_| | |_| | (_) | | | |
|_____\___/ \___\__,_|\__|_|\___/|_| |_|
   =========================
   */
   const char* Kernel::GetLogLocation(Error* err) const
   {
      ClearError(err);

      return 0;
   }
   
   /*
   =========================
 ____       _   ____       _                 _
/ ___|  ___| |_|  _ \  ___| |__  _   _  __ _| |    ___   __ _
\___ \ / _ \ __| | | |/ _ \ '_ \| | | |/ _` | |   / _ \ / _` |
 ___) |  __/ |_| |_| |  __/ |_) | |_| | (_| | |__| (_) | (_| |
|____/ \___|\__|____/ \___|_.__/ \__,_|\__, |_____\___/ \__, |
    _        _   _       _ _         _ |___/            |___/
   / \   ___| |_(_)_   _(_) |_ _   _| |    _____   _____| |
  / _ \ / __| __| \ \ / / | __| | | | |   / _ \ \ / / _ \ |
 / ___ \ (__| |_| |\ V /| | |_| |_| | |__|  __/\ V /  __/ |
/_/   \_\___|\__|_| \_/ |_|\__|\__, |_____\___| \_/ \___|_|
                               |___/
   =========================
   */
   void Kernel::SetLogActivityLevel(egSKILogActivityLevel ALevel,
                                         Error*                err)
   {
      ClearError(err);
      
      delete m_log; m_log = 0; // destroy old-log if any.

      m_logLevel = ALevel;

      unsigned char flags = 0;
      switch(ALevel)
      {
      case gSKI_LOG_NONE:
         flags = 0;
         break;
      case gSKI_LOG_ALL_EXCEPT_DEBUG:
         flags = ILog::ERR_LOG + ILog::INFO_LOG + ILog::WARN_LOG;
         break;
      case gSKI_LOG_ERRORS:
         flags = ILog::ERR_LOG;
         break;
      case gSKI_LOG_ALL:
         flags = ILog::ERR_LOG + ILog::INFO_LOG + ILog::WARN_LOG + ILog::DEBUG_LOG;
         break;
      }
      m_log = new Log("gSKI_Logs", flags, this );
   }

   /*
   =========================
  ____      _   ____       _                 _
 / ___| ___| |_|  _ \  ___| |__  _   _  __ _| |    ___   __ _
| |  _ / _ \ __| | | |/ _ \ '_ \| | | |/ _` | |   / _ \ / _` |
| |_| |  __/ |_| |_| |  __/ |_) | |_| | (_| | |__| (_) | (_| |
 \____|\___|\__|____/ \___|_.__/ \__,_|\__, |_____\___/ \__, |
    _        _   _       _ _         _ |___/            |___/
   / \   ___| |_(_)_   _(_) |_ _   _| |    _____   _____| |
  / _ \ / __| __| \ \ / / | __| | | | |   / _ \ \ / / _ \ |
 / ___ \ (__| |_| |\ V /| | |_| |_| | |__|  __/\ V /  __/ |
/_/   \_\___|\__|_| \_/ |_|\__|\__, |_____\___| \_/ \___|_|
                               |___/
   =========================
   */
   egSKILogActivityLevel Kernel::GetLogActivityLevel(Error* err) const
   {
      ClearError(err);


      return m_logLevel;
   }


   /*
   =========================
    _       _     _ ____            _                 _     _     _
   / \   __| | __| / ___| _   _ ___| |_ ___ _ __ ___ | |   (_)___| |_ ___ _ __   ___ _ __
  / _ \ / _` |/ _` \___ \| | | / __| __/ _ \ '_ ` _ \| |   | / __| __/ _ \ '_ \ / _ \ '__|
 / ___ \ (_| | (_| |___) | |_| \__ \ ||  __/ | | | | | |___| \__ \ ||  __/ | | |  __/ |
/_/   \_\__,_|\__,_|____/ \__, |___/\__\___|_| |_| |_|_____|_|___/\__\___|_| |_|\___|_|
                          |___/
      =========================
      */
      void Kernel::AddSystemListener(egSKIEventId        nEventId, 
                             ISystemListener*    pListener, 
                             bool                bAllowAsynch,
                             Error*              err)
      {
         ClearError(err);
         //m_systemListeners.AddListener(nEventId, pListener);
      }


   /*
   =========================
 ____                               ____            _
|  _ \ ___ _ __ ___   _____   _____/ ___| _   _ ___| |_ ___ _ __ ___
| |_) / _ \ '_ ` _ \ / _ \ \ / / _ \___ \| | | / __| __/ _ \ '_ ` _ \
|  _ <  __/ | | | | | (_) \ V /  __/___) | |_| \__ \ ||  __/ | | | | |
|_| \_\___|_|_|_| |_|\___/ \_/ \___|____/ \__, |___/\__\___|_| |_| |_|
| |   (_)___| |_ ___ _ __   ___ _ __      |___/
| |   | / __| __/ _ \ '_ \ / _ \ '__|
| |___| \__ \ ||  __/ | | |  __/ |
|_____|_|___/\__\___|_| |_|\___|_|
      =========================
      */
      void Kernel::RemoveSystemListener(egSKIEventId         nEventId,
                                ISystemListener*     pListener,
                                Error*               err)
      {
         ClearError(err);
         //m_systemListeners.RemoveListener(nEventId, pListener);
      }

	  /**************************************************
	   *
	   * Listen for rhs user function firings
	   *
	   **************************************************/
	  void Kernel::AddRhsListener(egSKIEventId   nEventId, 
                             IRhsListener*       pListener, 
                             bool                bAllowAsynch,
                             Error*              err)
      {
         ClearError(err);
         m_rhsListeners.AddListener(nEventId, pListener);
      }

	  /**************************************************
	   *
	   * Stop listening for rhs user function firings
	   *
	   **************************************************/
      void Kernel::RemoveRhsListener(egSKIEventId    nEventId,
                                IRhsListener*        pListener,
                                Error*               err)
      {
         ClearError(err);
         m_rhsListeners.RemoveListener(nEventId, pListener);
      }

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
	  bool Kernel::FireRhsNotification(IAgent* pAgent, bool commandLine, char const* pFunctionName, char const* pArgument,
									   int maxReturnValueLength, char* pReturnValue)
	  {
			RhsNotifier rhs(pAgent, commandLine, pFunctionName, pArgument, maxReturnValueLength, pReturnValue) ;
			bool result = m_rhsListeners.NotifyGetResult(gSKIEVENT_RHS_USER_FUNCTION, rhs) ;

			return result ;
	  }

   /*
   =========================
    _       _     _ _                _     _     _
   / \   __| | __| | |    ___   __ _| |   (_)___| |_ ___ _ __   ___ _ __
  / _ \ / _` |/ _` | |   / _ \ / _` | |   | / __| __/ _ \ '_ \ / _ \ '__|
 / ___ \ (_| | (_| | |__| (_) | (_| | |___| \__ \ ||  __/ | | |  __/ |
/_/   \_\__,_|\__,_|_____\___/ \__, |_____|_|___/\__\___|_| |_|\___|_|
                               |___/
      =========================
      */
      void Kernel::AddLogListener(egSKIEventId              nEventId, 
                                  ILogListener*             pListener, 
                                  bool                      bAllowAsynch,
                                  Error*                    err)
      {
         ClearError(err);

         m_logListeners.AddListener(nEventId, pListener);
         if(m_log != 0)
         {
            m_log->SetCallback(&m_logListeners);
         } else {
            SetErrorExtended(err, gSKIERR_INVALID_PTR, "Not Currently Logging");
         }
      }

   /*
   =========================
 ____                               _                _     _     _
|  _ \ ___ _ __ ___   _____   _____| |    ___   __ _| |   (_)___| |_ ___ _ __   ___ _ __
| |_) / _ \ '_ ` _ \ / _ \ \ / / _ \ |   / _ \ / _` | |   | / __| __/ _ \ '_ \ / _ \ '__|
|  _ <  __/ | | | | | (_) \ V /  __/ |__| (_) | (_| | |___| \__ \ ||  __/ | | |  __/ |
|_| \_\___|_| |_| |_|\___/ \_/ \___|_____\___/ \__, |_____|_|___/\__\___|_| |_|\___|_|
                                               |___/
      =========================
      */
   void Kernel::RemoveLogListener(egSKIEventId        nEventId,
                                    ILogListener*       pListener,
                                    Error*              err)
   {
      ClearError(err);
      m_logListeners.RemoveListener(nEventId, pListener);
   }




   /*
   =========================
    _       _     _  ____                            _   _
   / \   __| | __| |/ ___|___  _ __  _ __   ___  ___| |_(_) ___  _ __
  / _ \ / _` |/ _` | |   / _ \| '_ \| '_ \ / _ \/ __| __| |/ _ \| '_ \
 / ___ \ (_| | (_| | |__| (_) | | | | | | |  __/ (__| |_| | (_) | | | |
/_/   \_\__,_|\__,_|\____\___/|_| |_|_| |_|\___|\___|\__|_|\___/|_| |_|
| |    ___  ___| |_| |   (_)___| |_ ___ _ __   ___ _ __
| |   / _ \/ __| __| |   | / __| __/ _ \ '_ \ / _ \ '__|
| |__| (_) \__ \ |_| |___| \__ \ ||  __/ | | |  __/ |
|_____\___/|___/\__|_____|_|___/\__\___|_| |_|\___|_|
      =========================
      */
      void Kernel::AddConnectionLostListener(egSKIEventId             nEventId, 
                                             IConnectionLostListener* pListener, 
                                             bool                     bAllowAsynch,
                                             Error*                   err)
      {
         ClearError(err);
      
      }


   /*
   =========================
 ____                                ____                            _   _
|  _ \ ___ _ __ ___   _____   _____ / ___|___  _ __  _ __   ___  ___| |_(_) ___  _ __
| |_) / _ \ '_ ` _ \ / _ \ \ / / _ \ |   / _ \| '_ \| '_ \ / _ \/ __| __| |/ _ \| '_ \
|  _ <  __/ | | | | | (_) \ V /  __/ |__| (_) | | | | | | |  __/ (__| |_| | (_) | | | |
|_| \_\___|_| |_| |_|\___/_\_/ \___|\____\___/|_| |_|_| |_|\___|\___|\__|_|\___/|_| |_|
| |    ___  ___| |_| |   (_)___| |_ ___ _ __   ___ _ __
| |   / _ \/ __| __| |   | / __| __/ _ \ '_ \ / _ \ '__|
| |__| (_) \__ \ |_| |___| \__ \ ||  __/ | | |  __/ |
|_____\___/|___/\__|_____|_|___/\__\___|_| |_|\___|_|
      =========================
      */
      void Kernel::RemoveConnectionLostListener(egSKIEventId             nEventId,
                                                IConnectionLostListener* pListener,
                                                Error*                   err)
      {
         ClearError(err);
      
      }

   /*
   ==================================
 ____      _
|  _ \ ___| | ___  __ _ ___  ___
| |_) / _ \ |/ _ \/ _` / __|/ _ \
|  _ <  __/ |  __/ (_| \__ \  __/
|_| \_\___|_|\___|\__,_|___/\___|
   ==================================
   */
   void Kernel::Release(Error* err)
   {
      delete(this);
   }

   // TODO: Properly implement this function
   bool Kernel::IsClientOwned(Error* err) const
   { 
      MegaAssert(false, "Properly implement this function!");
      return false;
   }

   /*
   ==================================
   
   ==================================
   */
  EvilBackDoor::ITgDWorkArounds* Kernel::getWorkaroundObject()
  {
    static  EvilBackDoor::TgDWorkArounds evilBackDoor;
    return &evilBackDoor;
  }


}