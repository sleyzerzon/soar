#include <portability.h>

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_match.cpp
*********************************************************************
* created:	   6/27/2002   10:44
*
* purpose: 
*********************************************************************/
#include "gSKI_Match.h"

namespace gSKI
{

   Match::Match(agent *a, condition *cond)
   {

   }

   Match::~Match()
   {
   
   }

   Condition *Match::GetCondition(Error *e) const
   {

      return 0;
   }

   IWme *Match::GetWme(Error *e) const
   {

      return 0;
   }

}
