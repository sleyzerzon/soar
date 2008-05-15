#include <portability.h>

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_kernelfactory.cpp 
*********************************************************************
* created:	   6/26/2002   16:00
*
* purpose: 
*********************************************************************/
#include "gSKI_KernelFactory.h"
#include "gSKI_Error.h"
#include "gSKI_Kernel.h"
#include "gSKI_Iterator.h"

#include "kernel.h"

using namespace std;

namespace gSKI {

   KernelFactory::tInstanceInfoVec::t KernelFactory::m_instances;
   KernelFactory::tKernelVec::t       KernelFactory::m_kernels;

   KernelFactory::KernelFactory()
   {

   }

   Version KernelFactory::GetGSKIVersion(Error* err)const 
   {
      ClearError(err);
      Version v = {gSKI::MajorVersionNumber, gSKI::MinorVersionNumber, gSKI::MicroVersionNumber };
      return v;
   }

   Version KernelFactory::GetKernelVersion(Error* err) const 
   {
      ClearError(err); 
      Version v = {MAJOR_VERSION_NUMBER, MINOR_VERSION_NUMBER, MICRO_VERSION_NUMBER };
      return (v);
   }

   bool KernelFactory::CanCreateInProcess(Error* err) const 
   {
      ClearError(err);
   
      //
      // Because this base API always need to be in-process,
      // this should always return true.
      return true;
   }

   bool KernelFactory::CanAttachLocalOutOfProcess(Error* err) const 
   {
      ClearError(err);
   
      //
      // Because this base API always need to be in-process,
      // this should always return false.
      return false;
   }

   bool KernelFactory::CanCreateLocalOutOfProcess(Error* err) const 
   {
      ClearError(err);

      //
      // Because this base API always need to be in-process,
      // this should always return false.
      return false;
   }

   bool KernelFactory::CanAttachRemote(Error* err) const 
   {
      ClearError(err);
   
      //
      // Because this base API always need to be in-process,
      // this should always return false.
      return false;
   }

   bool KernelFactory::CanCreateRemote(Error* err) const 
   {
      ClearError(err);
   
      //
      // Because this base API always need to be in-process,
      // this should always return false.
      return false;
   }

   bool KernelFactory::CanCreateMultipleInstances(Error* err) const 
   {
      ClearError(err);
      return true;
   }

   tIInstanceInfoIterator* KernelFactory::GetInstanceIterator(Error* err) const
   {
      ClearError(err);

      tInstanceInfoIter *InstIter = new tInstanceInfoIter(m_instances);
   
      return InstIter;
   }

   Kernel* KernelFactory::Create(const char*           szInstanceName,
                                  egSKIThreadingModel   eTModel, 
                                  egSKIProcessType      ePType, 
                                  const char*           szLocation, 
                                  const char*           szLogLocation,
                                  Error*                err) const
   {
      ClearError(err);

      Kernel* newKernel = new Kernel(this);

      m_kernels.push_back(const_cast<const Kernel *>(newKernel));
      m_instances.push_back(newKernel->GetInstanceInformation());

      return newKernel;
   }

   void KernelFactory::DestroyKernel(Kernel *krnl, Error *err)
   {
      m_instances.erase(std::find(m_instances.begin(), m_instances.end(), krnl->GetInstanceInformation()));
      m_kernels.erase(std::find(m_kernels.begin(), m_kernels.end(), krnl));

	  // These two vectors are statics so they tend to leak memory on shutdown
	  // even if we erased all objects because the vectors are still reversing extra space.
	  // An explicit clear call helps make sure they aren't reported as leaking.
	  if (m_instances.size() == 0)
		  m_instances.clear() ;
	  if (m_kernels.size() == 0)
		  m_kernels.clear() ;

      delete(krnl);
   }

   Kernel* KernelFactory::Attach(const InstanceInfo* pInstanceInfo, 
                                  Error* err) const 
   {
      ClearError(err);
   
      return 0;
   }

   bool KernelFactory::Release(Error* err)
   {
      delete(this);
	  return true ;
   }
}
