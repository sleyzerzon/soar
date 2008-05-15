#include <portability.h>

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_production.cpp
*********************************************************************
* created:	   6/27/2002   10:44
*
* purpose: 
*********************************************************************/
#include "gSKI_Production.h"
#include "gSKI_Error.h"
#include "gSKI_Iterator.h"
#include "gSKI_ConditionSet.h"
#include "gSKI_Condition.h"
#include "gSKI_Iterator.h"
#include "gSKI_EnumRemapping.h"

#include <string>

//
// Soar includes.
#include "production.h"
#include "symtab.h"
#include "rete.h"
#include "print.h"
#include "gdatastructs.h"
#include "rhsfun.h"

namespace gSKI
{
   // If includeConditions is false, the conditions and actions are not loaded (i.e. creating gSKI copies of the kernel conditions)
   // and only the name etc. are valid in the production.  This turns out to be
   // all that we are interested in 95% of the time -- boosting performance substantially.
   // It also allows us to work around some deep bugs in gSKI's handling of conditions which in turn
   // lead to memory leaks.  If we ever solve those problems this optional loading is still helpful
   // due to the performance benefits.
   Production::Production(production *prod, bool includeConditions, agent* agent):
							m_soarProduction(prod), m_agent(agent), m_conditionSet(0)  
   {
      condition* top;
      condition* bottom;
      action*    rhs;

	  if (includeConditions)
	  {
		m_conditionSet = new ConditionSet(m_agent);
	      
		// This will extract the structures for the lhs and the rhs
		p_node_to_conditions_and_nots (m_agent, m_soarProduction->p_node, 0, 0, &top, &bottom, 0,&rhs);

		// We tranlate the structures to the gSKI structures
		loadConditions(top, *m_conditionSet);
		loadActions(rhs, m_actions, m_functions);
	  }
   }

   /** 
    * @brief Copy operator ensuring a correct deep copy.
    */
   Production&  Production::operator=(const Production& rhs)
   {
      if(this != &rhs)
      {
         // Cleanup all old memory
         cleanup();

         m_soarProduction = rhs.m_soarProduction;
         m_agent          = rhs.m_agent;
         m_conditionSet   = rhs.m_conditionSet;
        
         // Copy the actions and rhs functions
         for(tRhsActionVecCIt ita = rhs.m_actions.begin(); 
             ita != rhs.m_actions.end(); ++ita)
         {
            m_actions.push_back(new RhsAction(*(*ita)));
         }

         for(tRhsFunctionVecCIt itf = rhs.m_functions.begin(); 
             itf != rhs.m_functions.end(); ++itf)
         {
            m_functions.push_back(new RhsFunctionAction(*(*itf)));
         }
      }
      return *this;
   }

   Production::~Production() 
   {
      cleanup();
   }

   void Production::cleanup()
   {
      if(m_conditionSet)
      {
         delete m_conditionSet;
         m_conditionSet = 0;
      }

      // Delete all rhs actions
      for(tRhsActionVecIt ita = m_actions.begin(); ita != m_actions.end(); ++ita)
         delete (*ita);

      // Delete all rhs functions (standalone)
      for(tRhsFunctionVecIt itf = m_functions.begin(); itf != m_functions.end(); ++itf)
         delete (*itf);

      // Clear out the vectors
      m_actions.clear();
      m_functions.clear();
   }

   unsigned long Production::GetFiringCount(Error *err) const
   {
      return m_soarProduction->firing_count;
   }

   const char* Production::GetName(Error *err) const
   {
      MegaAssert(m_soarProduction != 0, "Uninitialized PRoduction.");

      static std::string s("");

      if(s.length() == 0)
      {
         MegaAssert(m_soarProduction->name->sc.name != 0, "Empty Production Name");
         return m_soarProduction->name->sc.name;
      }
      return 0;
   }

   egSKIProdType Production::GetType(Error *pErr) const
   {
      MegaAssert(m_soarProduction != 0, "Uninitialized PRoduction.");

      return EnumRemappings::ReMapProductionType(m_soarProduction->type);
   }

   const char* Production::GetDocs(Error *err) const
   {

      return 0;
   }

   const char* Production::GetSource(Error *err) const
   {

      return 0;
   }

   void dumpConditions(ConditionSet* cs)
   {
      tIConditionSetIterator* CondSets = cs->GetConditionSets();
      while(CondSets->IsValid());
   }

   const char* Production::GetText(Error *err) const 
   {
      return 0;
   }

   ConditionSet* Production::GetConditions(Error *err) const
   {
      // If this is NULL, you should ask to includeConditions when building the production object.
      assert(m_conditionSet) ;

      return m_conditionSet;
   }

   tIRhsActionIterator* Production::GetActions(Error *err)
   {
      // If this is NULL, you should ask to includeConditions (and actions) when building the production object.
      assert(m_conditionSet) ;

	  ClearError(err);
      return new Iterator<RhsAction*, tRhsActionVec>(m_actions);
   }

   tIRhsFunctionActionIterator* Production::GetStandAloneFunctions(Error* err)
   {
      // If this is NULL, you should ask to includeConditions when building the production object.
      assert(m_conditionSet) ;

      ClearError(err);
      return new Iterator<RhsFunctionAction*, tRhsFunctionVec>(m_functions);
   }

   void Production::Excise(Error *pErr )
   {
      excise_production(m_agent, m_soarProduction, false);
   }

   unsigned long Production::CountReteTokens(Error * pErr /*= 0*/)
   {
      ClearError(pErr);
      
      return count_rete_tokens_for_production(m_agent, m_soarProduction);
   }

   void Production::loadProductionFromKernel(void) const
   {
      
   }

   void Production::loadConditions(condition *conds, ConditionSet &condSet) const
   {
      if(conds == 0) 
         return;
      
      for(condition *c = conds; c != 0; c = c->next)
      {
         //
         // If we have a negated set.
         if (c->type == CONJUNCTIVE_NEGATION_CONDITION)
         {
            loadConditions((c)->data.ncc.top, *(condSet.AddConditionSet()));
         }
         else
         {
            condSet.AddCondition(c);
         }
      }
   }

   void Production::loadActions(action* actions, tRhsActionVec& actionVec, tRhsFunctionVec& funcVec)
   {
      if(actions == 0)
         return;

      for(action* a = actions; a != 0; a = a->next)
      {
         if(a->type == MAKE_ACTION)
         {
            actionVec.push_back(new RhsAction(m_agent, a));
         }
         else if(a->type == FUNCALL_ACTION)
         {
            MegaAssert(rhs_value_is_funcall(a->value), "Rhs function is supposed to be a standalone function.");
            if(rhs_value_is_funcall(a->value))
               funcVec.push_back(new RhsFunctionAction(m_agent, rhs_value_to_funcall_list(a->value)));
         }
         else
         {
            MegaAssert(false, "Unknown type of action for a production.");
         }
      }
   }

   bool Production::IsWatched() 
   {
	   return m_soarProduction->trace_firings ? true : false;
   }
}
