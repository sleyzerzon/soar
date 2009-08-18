/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file igski_conditionset.h 
*********************************************************************
* created:	   6/19/2002   9:28
*
* purpose: 
*********************************************************************/
#ifndef IGSKI_CONDITIONSET_H
#define IGSKI_CONDITIONSET_H

#include "IgSKI_Iterator.h"


namespace gSKI {

struct Error;

   /**
    * @brief Handles condition heriarchies.
    *
    * Because a condition can be made up of two different kinds of
    * tests, basic tests, and set tests, we use this class to differentiate
    * them.  The reason that all tests cannot be treated the same is because
    * an entire set may be negated, which means that the condition is only
    * true if _all_ of the sub-conditions of that set are 
    * not true.
    *
    * On top of all of this, a negated condition set can be made up of many
    * subsets of negated conditions, which can be made up of more subsets
    * of negated condions, etc. etc. etc. ad ifinitum.
    *
    * To handle this, at each conditional level of the condition, there
    * are two different lists.  A list of individual elements that are
    * being tested at that level, and a list of negated subsets for the
    * same level.  This will allow you to traverse the condition heirarchy
    * and get the relevant conditions out.  _Remember_, if all of the
    * sets your get back from a call to GetMatchSets is a negated set!!!</i>
    *
    * It is worth noting that all relevant sets are negated.
    */
   class IConditionSet {
   public:
      /**
       * @brief Destructor for the IConditionSet
       *
       * This function insures that the destructor in the most derived
       * class is called when it is destroyed.  This will always be 
       * neccesary because this is a pure virtual base class.
       */
      virtual ~IConditionSet(){};

      /**
       * @brief This returns the conditions that are not within
       *        a negated set.
       *
       * @param  pErr Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       * @returns An iterator to he conditions not in a 
       *          negated set.
       */
      virtual tIConditionIterator *GetConditions(Error *pErr = 0) = 0;

      /**
       * @brief Returns the number of conditions
       *
       * @returns The number of conditions that will be returned by a call
       *          to GetConditions()
       */
      virtual unsigned int GetNumConditions(Error *pErr = 0)const  = 0;

      /**
       * @brief Returns the negated lists.
       *
       * @param  pErr Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       * @returns An Iterator to a list of negated condition
       *          sets at this level of the condition.
       */
      virtual tIConditionSetIterator *GetConditionSets(Error *pErr = 0) const = 0;

      /**
       * @brief Returns the number of condition sets.
       *
       * @param  pErr Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       * @returns The number of condition sets at this "level"
       */
      virtual unsigned int GetNumConditionSets(Error *pErr = 0) const  = 0;

      /**
       * @brief Indiates whether or not this condition set is negated.
       *
       * @param  pErr Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       * With the exception of the very initial set returned by the
       * production, all sets are...by definition negated.  This is 
       * because a non-negated set is really the same as the the same
       * grouping of conditions not in a set.  While single negated
       * conditions can be placed in a set, they are really the same
       * as a non-set negated condition.
       *
       * @returns Negation of this set.
       */
      virtual bool IsNegated(Error *pErr = 0) const = 0;

   };
}

#endif