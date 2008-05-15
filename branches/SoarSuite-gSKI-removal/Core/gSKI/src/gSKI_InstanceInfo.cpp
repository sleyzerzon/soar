#include <portability.h>

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_instanceinfo.cpp 
*********************************************************************
* created:	   6/28/2002   12:07
*
* purpose: 
*********************************************************************/
#include "gSKI_InstanceInfo.h"
#include "gSKI_Error.h"

namespace gSKI
{
   InstanceInfo::InstanceInfo(const char*         instanceName, 
                              const char*         serverName, 
                              egSKIProcessType    processType, 
                              egSKIThreadingModel threadingModel) : m_instanceName(instanceName),
                                                                    m_serverName(serverName),
                                                                    m_threadingModel(threadingModel),
                                                                    m_processType(processType)
   {

   
   }

   InstanceInfo::~InstanceInfo()
   {
   
   }

   const char* InstanceInfo::GetInstanceName(Error* err) const
   {
      ClearError(err);

      return m_instanceName.c_str();
   }

   egSKIProcessType InstanceInfo::GetProcessType(Error* err) const
   {
      ClearError(err);
   
      return m_processType;
   }

   const char* InstanceInfo::GetServerName(Error* err) const
   {
      ClearError(err);
   
      return m_serverName.c_str();
   }

   egSKIThreadingModel InstanceInfo::GetThreadingModel(Error* err) const
   {
      ClearError(err);
      return(m_threadingModel);
   }
}
