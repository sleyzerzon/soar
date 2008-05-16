#include <portability.h>

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
#include "gSKI_EnumRemapping.h"
#include "kernel_struct.h"
#include "MegaAssert.h"

namespace gSKI
{

   Kernel::Kernel(const KernelFactory* kf) : m_kF(kf)
   {
      m_soarKernel   = create_kernel();
	  m_InterruptCheckRate = 10 ;
	  m_stopPoint    = gSKI_INPUT_PHASE;
      init_soar( m_soarKernel );
      m_instanceInfo = new InstanceInfo("test1", "test2", gSKI_IN_PROCESS, gSKI_SINGLE_THREAD);
   }

   Kernel::~Kernel()
   {
      delete m_instanceInfo; m_instanceInfo = 0;
      destroy_kernel(m_soarKernel);
      m_soarKernel = 0;
   }

   const KernelFactory* Kernel::GetKernelFactory(Error* err) const
   {
      return m_kF;
   }

   IPerformanceMonitor* Kernel::GetPerformanceMonitor(Error* err)
   {
      ClearError(err);
   
      return 0;
   }

   InstanceInfo* Kernel::GetInstanceInformation(Error* err)
   {
      ClearError(err);
   
      return m_instanceInfo;
   }

      void Kernel::AddConnectionLostListener(egSKISystemEventId       nEventId, 
                                             IConnectionLostListener* pListener, 
                                             bool                     bAllowAsynch,
                                             Error*                   err)
      {
         ClearError(err);
      
      }

      void Kernel::RemoveConnectionLostListener(egSKISystemEventId       nEventId,
                                                IConnectionLostListener* pListener,
                                                Error*                   err)
      {
         ClearError(err);
      
      }

   void Kernel::Release(Error* err)
   {
      delete(this);
   }

  int  Kernel::GetInterruptCheckRate() const			{ return m_InterruptCheckRate ; }
  void Kernel::SetInterruptCheckRate(int newRate)	{ if (newRate >= 1) m_InterruptCheckRate = newRate ; }

  unsigned long Kernel::GetStopPoint()                 {return m_stopPoint ; }
  void Kernel::SetStopPoint(bool forever, egSKIRunType runStepSize, egSKIPhaseType m_StopBeforePhase)
  {
	  if ((gSKI_RUN_DECISION_CYCLE == runStepSize) || forever) {
		  m_stopPoint = m_StopBeforePhase ;
	  } else  {
		  m_stopPoint = gSKI_INPUT_PHASE ;
	  }
  }
}
