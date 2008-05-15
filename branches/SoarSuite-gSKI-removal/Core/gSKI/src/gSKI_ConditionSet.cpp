#include <portability.h>

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_conditionset.cpp
*********************************************************************
* created:	   6/27/2002   10:44
*
* purpose: 
*********************************************************************/
#include "gSKI_ConditionSet.h"
#include "gSKI_Condition.h"
#include "gSKI_Test.h"
#include "gSKI_Iterator.h"
#include "gSKI_TestSet.h"
#include "gSKI_Test.h"

#include <vector>
#include <iostream>

#include "gdatastructs.h"
#include "production.h"

namespace gSKI 
{
   ConditionSet::ConditionSet(agent* a) : m_agent(a)
   {

   }

   ConditionSet::~ConditionSet()
   {
      std::vector<Condition*>::iterator condIt = m_conditions.begin();
      for( ; condIt != m_conditions.end(); ++condIt)
      {
         Condition* c = *condIt;
         delete(c);
      }
   }

   tIConditionIterator* ConditionSet::GetConditions(Error *pErr) 
   {
      //return new tConditionIterator(m_conditions);
      return new Iterator<Condition *, tConditionVec>(m_conditions);
   }

   unsigned int ConditionSet::GetNumConditions(Error *pErr)const
   {
      return 0;
   }

   tIConditionSetIterator *ConditionSet::GetConditionSets(Error *pErr) const
   {
      //return new Iterator<tConditionSet::V *, tConditionSet::t>(m_conditionSets);
      return new tConditionSetIter(m_conditionSets);
   }

   unsigned int ConditionSet::GetNumConditionSets(Error *pErr) const
   {
      return static_cast<unsigned int>(m_conditionSets.size());
   }

   bool ConditionSet::IsNegated(Error *pErr) const
   {
      return true;
   }

   void ConditionSet::AddCondition(condition *cond)
   {
      m_conditions.push_back(new Condition(cond, m_agent));
   }

   ConditionSet* ConditionSet::AddConditionSet()
   {
      ConditionSet* newConditionSet = new ConditionSet(m_agent);
      m_conditionSets.push_back(newConditionSet);
      return newConditionSet;
   }
}
