/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_errorids.h 
*********************************************************************
* created:	   6/19/2002   11:38
*
* purpose: 
*********************************************************************/
#ifndef GSKI_ERRORIDS_H
#define GSKI_ERRORIDS_H

#include "gSKI_Structures.h"

namespace gSKI {

   class ErrorClass : private gSKI::Error
   {
   public:
      tgSKIErrId   GetgSKIErrorId() const { return Id; }
      const char*  GetErrorMsg()    const { return Text; }
      const char*  GetExtendedMsg() const { return ExtendedMsg; }
   };

}
#endif
