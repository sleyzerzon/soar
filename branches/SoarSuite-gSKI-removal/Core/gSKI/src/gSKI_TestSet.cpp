#include <portability.h>

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_testset.cpp
*********************************************************************
* created:	   6/27/2002   10:44
*
* purpose: 
*********************************************************************/
#include "gSKI_TestSet.h"
#include "gSKI_Test.h"
#include "gSKI_Iterator.h"

#include "gdatastructs.h"
#include "mem.h"

namespace gSKI 
{
   TestSet::TestSet(test thisTest, agent* a) : m_agent(a)
   {
      if(Test::IsSimpleTest(thisTest))
      {
         AddTest(thisTest);
      }
      else
      {
         AddTestSet(thisTest);
      }
   }

   TestSet::~TestSet()
   {
      std::vector<Test *>::iterator testIt = m_tests.begin();
      for( ; testIt != m_tests.end() ; ++testIt)
      {
         delete(*testIt);
      }
      m_tests.clear();


      std::vector<TestSet *>::iterator djIt = m_testSets.begin();
      for( ; djIt != m_testSets.end() ; ++djIt)
      {
         delete(*djIt);
      }
      m_testSets.clear();
   }

   tITestIterator *TestSet::GetTests(Error * /*err*/) const
   {
      return new Iterator<Test *, tTestVec>(m_tests);
   }

   tITestSetIterator *TestSet::GetTestSets(Error * /*err*/) const
   {
      return new Iterator<TestSet *, tTestSetVec>(m_testSets);
   }

   void TestSet::AddTest(const test t)
   {
      m_tests.push_back(new Test(t, m_agent));
   }

   void TestSet::AddTestSet(const test ts)
   {
      //
      // We know we have either a conjunction or a disjunction at this point.
      // We will check which one, then traverse the list adding each test in
      // the conjunction and disjunction to this TestSet.
      complex_test *ct = complex_test_from_test(ts);

      list *testList;
      if(ct->type == DISJUNCTION_TEST)
         testList = ct->data.disjunction_list;
      else
         testList = ct->data.conjunct_list;

      test t;
      for(;testList != 0; testList = testList->rest)
      {
         t = (test)(testList->first);
         if(Test::IsSimpleTest(t))
         {
            m_tests.push_back(new Test(t, m_agent));
         }
         else
         {
            m_testSets.push_back(new TestSet(t, m_agent));
         }
      }
   }

   unsigned int TestSet::GetNumTests(Error * /*err*/) const
   {
      return static_cast<unsigned int>(m_tests.size());
   }

   unsigned int TestSet::GetNumTestSets(Error * /*err*/) const
   {
      return static_cast<unsigned int>(m_testSets.size());
   }


}

