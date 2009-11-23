#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/********************************************************************
* @file gski_match.cpp
*********************************************************************
* @remarks Copyright (C) 2002 Soar Technology, All rights reserved. 
* The U.S. government has non-exclusive license to this software 
* for government purposes. 
*********************************************************************
* created:	   6/27/2002   10:44
*
* purpose: 
*********************************************************************/
#include "gSKI_Match.h"

namespace gSKI
{

   /*
   ===============================
 __  __       _       _
|  \/  | __ _| |_ ___| |__
| |\/| |/ _` | __/ __| '_ \
| |  | | (_| | || (__| | | |
|_|  |_|\__,_|\__\___|_| |_|
   ===============================
   */
   Match::Match(agent *a, condition *cond)
   {

   }


   /*
   ===============================
 /\/|_  __       _       _
|/\/  \/  | __ _| |_ ___| |__
   | |\/| |/ _` | __/ __| '_ \
   | |  | | (_| | || (__| | | |
   |_|  |_|\__,_|\__\___|_| |_|
   ===============================
   */
   Match::~Match()
   {
   
   }

   /*
   ===============================
  ____      _    ____                _ _ _   _
 / ___| ___| |_ / ___|___  _ __   __| (_) |_(_) ___  _ __
| |  _ / _ \ __| |   / _ \| '_ \ / _` | | __| |/ _ \| '_ \
| |_| |  __/ |_| |__| (_) | | | | (_| | | |_| | (_) | | | |
 \____|\___|\__|\____\___/|_| |_|\__,_|_|\__|_|\___/|_| |_|
   ===============================
   */

   ICondition *Match::GetCondition(Error *e) const
   {

      return 0;
   }

   /*
   ===============================
  ____      _ __        __
 / ___| ___| |\ \      / / __ ___   ___
| |  _ / _ \ __\ \ /\ / / '_ ` _ \ / _ \
| |_| |  __/ |_ \ V  V /| | | | | |  __/
 \____|\___|\__| \_/\_/ |_| |_| |_|\___|
   ===============================
   */

   IWme *Match::GetWme(Error *e) const
   {

      return 0;
   }

}