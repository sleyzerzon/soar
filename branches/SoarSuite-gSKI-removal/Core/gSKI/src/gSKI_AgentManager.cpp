#include <portability.h>

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gSKI_AgentManager.cpp
*********************************************************************
* created:	   6/27/2002   10:44
*
* purpose: 
*********************************************************************/
#include "gSKI_AgentManager.h"
#include "gSKI_Error.h"
#include "gSKI_Agent.h"
#include "gSKI_Enumerations.h"
#include "gSKI_Kernel.h"
#include "gSKI_ProductionManager.h"

#include "MegaAssert.h"

#include "init_soar.h"
#include "agent.h"
#include "kernel_struct.h"

namespace gSKI 
{

   AgentManager::AgentManager(Kernel* krnl) : m_kernel(krnl), m_runManager(krnl)
   {
      m_runCompleteListener.SetAgentManager(this);

      // Let the run manager know when agents are destroyed.
      AddAgentListener(gSKIEVENT_BEFORE_AGENT_DESTROYED, &m_runManager);
   }

   AgentManager::~AgentManager() 
   {
      for(tAgentMap::It it = m_agents.begin(); it != m_agents.end(); ++it)
      {
         // TODO: anything special if the agent is running?
         // maybe just require that the agents be stopped..
         delete it->second;
      }
   }

   void AgentManager::AddAgentToRunList(Agent* agentToAdd, Error* err)
   {
      ClearError(err);
      m_runManager.AddAgentToRunList(agentToAdd);
   }

   void AgentManager::RemoveAgentFromRunList(Agent* agentToAdd, Error* err)
   {
      ClearError(err);
      m_runManager.RemoveAgentFromRunList(agentToAdd);
   }

   void AgentManager::AddAllAgentsToRunList(Error* err)
   {
      ClearError(err);
      for(tAgentMap::It it = m_agents.begin(); it != m_agents.end(); ++it)
         m_runManager.AddAgentToRunList((*it).second);
   }

   void AgentManager::RemoveAllAgentsFromRunList(Error* err)
   {
      ClearError(err);
      for(tAgentMap::It it = m_agents.begin(); it != m_agents.end(); ++it)
         m_runManager.RemoveAgentFromRunList((*it).second);
   }


   bool AgentManager::InterruptAll(egSKIStopLocation    stopLoc, 
                                   Error*               err)
   {
      Agent* a;

      ClearError(err);

      // Iterate over each of the agents and call interrupt on each of
      //  them, checking first to see if they are actually running.
      for(tAgentMap::It aIt = m_agents.begin() ; aIt != m_agents.end(); ++aIt)
      {
         a = (*aIt).second;

         // This will clear the error if there is no error
         if(a->Interrupt(stopLoc, gSKI_STOP_BY_RETURNING, err) == false)
            return false;
      }

      // We succeeded
      return true;
   }

   void AgentManager::ClearAllInterrupts(Error* err)
   {
      Agent* a;

      // Just in case there are no agents
      ClearError(err);

      // Iterate over each of the agents and if they are interrupted,
      //  call continue on them
      for(tAgentMap::It aIt = m_agents.begin() ; aIt != m_agents.end(); ++aIt)
      {
         a = (*aIt).second;

         // This will clear the error if there is no error
         if(a->GetRunState() == gSKI_RUNSTATE_INTERRUPTED)
            a->ClearInterrupts(err);
      }
   }

   void AgentManager::HaltAll(Error* err)
   {
      Agent* a;

      ClearError(err);

      // Iterate over each of the agents and if they are interrupted,
      //  call continue on them
      for(tAgentMap::It aIt = m_agents.begin() ; aIt != m_agents.end(); ++aIt)
      {
         a = (*aIt).second;

         // Halt the system and return the error value if there is an error
         a->Halt(err);
         if(err->Id != gSKIERR_NONE)
            return;
      }
   }

   tIAgentIterator* AgentManager::GetAgentIterator(Error* err)
   {
		ClearError(err);

		// I don't see a way to build a gSKI iterator from a map
		// so copying the agents into a vector and creating an iterator for that.
		std::vector<Agent*> agents ;

		for(tAgentMap::It iter = m_agents.begin() ; iter != m_agents.end(); ++iter)
		{
			Agent* pAgent = iter->second ;
			agents.push_back(pAgent) ;
		}

		// Create an iterator for our vector.  I assume the vector can now go out of scope...I hope that's right.
		tIAgentIterator* pAgentIter = new tAgentIter(agents) ;

		return pAgentIter;
   }

   Agent* AgentManager::GetAgent(const char* name, Error* err)
   {
      ClearError(err);

      if(IsInvalidPtr(name, err))
         return 0;

      //
      // Search the map for this agent (by name) and
      // return the Agent pointer if it is in the list.
      //
      tAgentMap::It agentIt = m_agents.find(name);
      if(agentIt == m_agents.end())
      {
         SetError(err, gSKIERR_AGENT_DOES_NOT_EXIST);
         return 0;
      }

      return agentIt->second;
   }

   void AgentManager::RemoveAgentByName(const char *name, Error* err)
   {
      ClearError(err);

      if(IsInvalidPtr(name, err))
         return;

      //
      // Find the agent and delete it if you find it.
      // Otherwise, return an error.
      //
      tAgentMap::It agentIt = m_agents.find(name);
      if(agentIt != m_agents.end())
      {
         Agent* _agent = (*agentIt).second;

         // If it is running, we listen for it to stop running and
         //  delete it after it is stopped.
         if(_agent->GetRunState() == gSKI_RUNSTATE_RUNNING)
         {
			 assert(false) ;	// Not supported

            // Set up to listen for this agent being done running.  This will
            //  call RemoveAgentByName after the agent completes its run
//            _agent->AddRunListener(gSKIEVENT_AFTER_RUN_ENDS, &m_runCompleteListener);

            // Tell it to halt
            _agent->Halt();
         }
         else
         {
            // It is ok to remove it here

            // Notify everyone that the agent is going to be destroyed
            AgentNotifier nf((*agentIt).second);
            m_agentListeners.Notify(gSKIEVENT_BEFORE_AGENT_DESTROYED, nf);

            // Destroy it
            delete((*agentIt).second);
            m_agents.erase(agentIt);
         }

      } 
      else 
      {
         SetError(err, gSKIERR_AGENT_DOES_NOT_EXIST);
      }
   }

   void AgentManager::RemoveAgent(Agent* agent, Error* err)
   {
      ClearError(err);

      if(IsInvalidPtr(agent, err))
         return;

      tAgentMap::It agentIt = m_agents.begin();

      //
      // Find the agent in the map as a 'second' value.
      for(; agentIt != m_agents.end(); ++agentIt)
      {
         //
         // Is this one the agent we are looking for?
         if(agentIt->second == agent)
         {
            //
            // Remove the agent and leave the method.
            RemoveAgentByName(agentIt->second->GetName());
            return;
         }
      }
      SetError(err, gSKIERR_AGENT_DOES_NOT_EXIST);
   }

   Agent* AgentManager::AddAgent(agent* pSoarAgent, 
                                  const char*       prodFileName, 
                                  bool              learningOn,
                                  egSKIOSupportMode oSupportMode,
                                  Error*            err)
   {
      ClearError(err);
	  //MegaAssert(false, "Stop here for debugging!");

      Agent* _agent = new Agent(pSoarAgent, m_kernel);

	  std::string tmpStr(pSoarAgent->name);
      m_agents.insert(std::pair<std::string, Agent *>(tmpStr, _agent));


      return _agent;
   }

   void AgentManager::FireAgentCreated(Agent* pAgent)
   {
      // Notify everyone that the agent was created
      AgentNotifier nf(pAgent);
      m_agentListeners.Notify(gSKIEVENT_AFTER_AGENT_CREATED, nf);
   }

   void AgentManager::AddAgentListener(egSKIAgentEventId eventId, 
                                     IAgentListener*     listener, 
                                     bool                allowAsynch,
                                     Error*              err)
   {
      AddListenerToManager(m_agentListeners, eventId, listener, err);
   }

   void AgentManager::RemoveAgentListener(egSKIAgentEventId  eventId,
                                        IAgentListener*      listener,
                                        Error*               err)
   {
      RemoveListenerFromManager(m_agentListeners, eventId, listener, err);
   }

   void AgentManager::AgentRunCompletedListener::HandleEvent(egSKIRunEventId eventId, Agent* agentPtr, egSKIPhaseType phase)
   {
      MegaAssert(eventId == gSKIEVENT_AFTER_RUN_ENDS, "Getting an unexpected event in the agent removal listener.");
      m_am->RemoveAgentByName(agentPtr->GetName());
   }

}



