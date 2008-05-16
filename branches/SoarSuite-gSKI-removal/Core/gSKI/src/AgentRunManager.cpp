#include <portability.h>

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gSKI_AgentRunManager.cpp
*********************************************************************
* created:	   6/27/2002   10:44
*
* purpose: 
*********************************************************************/

#include "AgentRunManager.h"

#include "MegaAssert.h"
#include "gSKI_Agent.h"
#include "gSKI_Error.h"
#include "gSKI_Enumerations.h"
#include "gSKI_Kernel.h"

#include <algorithm>

namespace gSKI
{
   AgentRunManager::AgentRunData::AgentRunData(Agent* _a, unsigned long _steps, unsigned long _maxSteps): 
      a(_a), steps(_steps), maxSteps(_maxSteps) { }


	  void AgentRunManager::AddAgentToRunList(Agent* a)
   {
      // We make sure we don't add it twice
      tAgentListIt it = std::find(m_addedAgents.begin(), m_addedAgents.end(), a);
      if(it == m_addedAgents.end())
      {
         m_addedAgents.push_back(a);

         // If it is in the remove list, we remove it (this way we don't try
         //  to add and remove the same agent)
         it = std::find(m_removedAgents.begin(), m_removedAgents.end(), a);
         if(it != m_removedAgents.end())
            m_removedAgents.erase(it);
      }
   }

   void AgentRunManager::RemoveAgentFromRunList(Agent* a)
   {
      // We make sure we don't remove it twice
      tAgentListIt it = std::find(m_removedAgents.begin(), m_removedAgents.end(), a);
      if(it == m_removedAgents.end())
      {
         m_removedAgents.push_back(a);
      
         // If it is in the add list, we remove it (this way we don't try
         //  to add and remove the same agent)
         it = std::find(m_addedAgents.begin(), m_addedAgents.end(), a);
         if(it != m_addedAgents.end())
            m_addedAgents.erase(it);
      }
   }

   void AgentRunManager::HandleEvent(egSKIAgentEventId eventId, Agent* agentPtr)
   {
      if(eventId == gSKIEVENT_BEFORE_AGENT_DESTROYED)
         m_removedAgents.push_back(agentPtr);
   }

   bool AgentRunManager::isValidAgent(Agent* a)
   {
      if(m_removedAgents.size() > 0)
      {
         tAgentListIt it = std::find(m_removedAgents.begin(), m_removedAgents.end(), a);
         return (it == m_removedAgents.end())? true: false;
      }
      // There are no agents to remove
      return true;
   }

   void AgentRunManager::removeFromRunList(Agent* a)
   {
      // Iterate until we find the agent
      tAgentRunListIt it = m_runningAgents.begin(); 
      while((it != m_runningAgents.end()) && ((*it).a != a))
         ++it;

      // If we found it, remove it
      if(it != m_runningAgents.end())
         m_runningAgents.erase(it);
   }

   Kernel* AgentRunManager::getKernel()
   {
		return m_pKernel ;
   }

}
