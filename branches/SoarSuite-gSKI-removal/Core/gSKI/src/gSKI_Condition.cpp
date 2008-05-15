#include <portability.h>

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_condition.cpp
*********************************************************************
* created:	   6/27/2002   10:44
*
* purpose: 
*********************************************************************/
#include "gSKI_Condition.h"
#include "gSKI_Enumerations.h"
#include "gSKI_Test.h"
#include "gSKI_TestSet.h"
#include "gSKI_EnumRemapping.h"

#include "gdatastructs.h"
#include "production.h"
#include "print.h"
#include "MegaAssert.h"

namespace gSKI 
{


   Condition::Condition(condition *cond, agent* a) : m_condition(cond),
                                                     m_agent(a),
                                                     m_idTest(0),
                                                     m_attrTest(0),
                                                     m_valTest(0)
   {
     Bool removed_goal_test;
     Bool removed_impasse_test;
   	
	   test id_test = copy_test_removing_goal_impasse_tests(m_agent, 
                                                           m_condition->data.tests.id_test, 
	                                                        &removed_goal_test, 
	                                                        &removed_impasse_test);
	   m_idTest   = new TestSet(id_test, m_agent);
   	
	   m_attrTest = new TestSet(m_condition->data.tests.attr_test, m_agent);
   	
	   m_valTest = new TestSet(m_condition->data.tests.value_test, m_agent);
   }

   Condition::~Condition()
   {
	   if(m_idTest   != 0) delete m_idTest;
	   if(m_attrTest != 0) delete m_attrTest;
	   if(m_valTest  != 0) delete m_valTest;
   }

   const char* Condition::GetText(Error* err)
   {

      return 0;
   }

   bool Condition::IsNegated(Error* err)
   {
      return m_condition->type == NEGATIVE_CONDITION;
   }

   bool Condition::IsImpasseCondition(Error* err)
   {
      return false;
   }

   bool Condition::IsStateCondition(Error* err)
   {
      return false;
   }

   TestSet* Condition::GetIdTest(Error* err)
   {
      return m_idTest;
   }

   TestSet* Condition::GetAttrTest(Error* err)
   {
      return m_attrTest;
   }

   TestSet* Condition::GetValTest(Error* err)
   {
      return m_valTest;
   }
}
