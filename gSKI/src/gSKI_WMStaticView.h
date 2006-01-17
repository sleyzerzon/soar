/********************************************************************
* @file gski_wmstaticview.h 
*********************************************************************
* @remarks Copyright (C) 2002 Soar Technology, All rights reserved. 
* The U.S. government has non-exclusive license to this software 
* for government purposes. 
*********************************************************************
* created:	   7/11/2002   16:59
*
* purpose: 
*********************************************************************/

#ifndef GSKI_WMSTATICVIEW_H
#define GSKI_WMSTATICVIEW_H

#include "IgSKI_WMStaticView.h"

namespace gSKI {

   /**
    * @brief This interface represents a static snap shot of working memory
    *         data.
    *
    * This interface provides a static snapshot of working memory data and
    * provides methods for searching over memory data elements and creating
    * static subviews.
    */
   class WMStaticView: public IWMStaticView {
   public:

      /** 
       * @brief Virtual Destructor 
       *
       * Including adestructor for the usual C++ safety reasons. 
       * This should not be called be client programers. Use the Release()
       * method of the IRelease interface to clean up the resources for 
       * objects of this type.
       */
     ~WMStaticView();
   };

}

#endif
