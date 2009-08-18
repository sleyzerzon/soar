#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/********************************************************************
* @file gski_matchset.cpp
*********************************************************************
* @remarks Copyright (C) 2002 Soar Technology, All rights reserved. 
* The U.S. government has non-exclusive license to this software 
* for government purposes. 
*********************************************************************
* created:	   6/27/2002   10:44
*
* purpose: 
*********************************************************************/

#include "gSKI_MatchSet.h"

namespace gSKI
{
   /*
   ===============================
 /\/|_  __       _       _     ____       _
|/\/  \/  | __ _| |_ ___| |__ / ___|  ___| |_
   | |\/| |/ _` | __/ __| '_ \\___ \ / _ \ __|
   | |  | | (_| | || (__| | | |___) |  __/ |_
   |_|  |_|\__,_|\__\___|_| |_|____/ \___|\__|
   ===============================
   */
   MatchSet::~MatchSet(){}

   /*
   ===============================
  ____      _   __  __       _       _
 / ___| ___| |_|  \/  | __ _| |_ ___| |__   ___ ___
| |  _ / _ \ __| |\/| |/ _` | __/ __| '_ \ / _ Y __|
| |_| |  __/ |_| |  | | (_| | || (__| | | |  __|__ \
 \____|\___|\__|_|  |_|\__,_|\__\___|_| |_|\___|___/
   ===============================
   */
   tIMatchIterator *MatchSet::GetMatches(Error* err)
   {

      return 0;
   }

   /*
   ===============================
  ____      _   __  __       _       _     ____       _
 / ___| ___| |_|  \/  | __ _| |_ ___| |__ / ___|  ___| |_ ___
| |  _ / _ \ __| |\/| |/ _` | __/ __| '_ \\___ \ / _ \ __/ __|
| |_| |  __/ |_| |  | | (_| | || (__| | | |___) |  __/ |_\__ \
 \____|\___|\__|_|  |_|\__,_|\__\___|_| |_|____/ \___|\__|___/
   ===============================
   */
   tIMatchSetIterator *MatchSet::GetMatchSets(Error* err)
   {

      return 0;
   }

   /*
   ===============================
  ____      _    ____                _ _ _   _             ____       _
 / ___| ___| |_ / ___|___  _ __   __| (_) |_(_) ___  _ __ / ___|  ___| |_
| |  _ / _ \ __| |   / _ \| '_ \ / _` | | __| |/ _ \| '_ \\___ \ / _ \ __|
| |_| |  __/ |_| |__| (_) | | | | (_| | | |_| | (_) | | | |___) |  __/ |_
 \____|\___|\__|\____\___/|_| |_|\__,_|_|\__|_|\___/|_| |_|____/ \___|\__|
   ===============================
   */
   IConditionSet *MatchSet::GetConditionSet(Error* err)
   {
       return 0;
   }

}