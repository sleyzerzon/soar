#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/* utilities.cpp */

#include "stl_support.h"
#include "utilities.h"
#include "gdatastructs.h"
#include "wmem.h"

wme **get_augs_of_id(agent* thisAgent, Symbol * id, tc_number tc, int *num_attr)
{
   slot *s;
   wme *w;


   wme **list;                 /* array of WME pointers, AGR 652 */
   int attr;                   /* attribute index, AGR 652 */
   int n;


/* AGR 652  The plan is to go through the list of WMEs and find out how
  many there are.  Then we malloc an array of that many pointers.
  Then we go through the list again and copy all the pointers to that array.
  Then we qsort the array and print it out.  94.12.13 */


   if (id->common.symbol_type != IDENTIFIER_SYMBOL_TYPE)
       return NULL;
   if (id->id.tc_num == tc)
       return NULL;
   id->id.tc_num = tc;


   /* --- first, count all direct augmentations of this id --- */
   n = 0;
   for (w = id->id.impasse_wmes; w != NIL; w = w->next)
       n++;
   for (w = id->id.input_wmes; w != NIL; w = w->next)
       n++;
   for (s = id->id.slots; s != NIL; s = s->next) {
       for (w = s->wmes; w != NIL; w = w->next)
           n++;
       for (w = s->acceptable_preference_wmes; w != NIL; w = w->next)
           n++;
   }


   /* --- next, construct the array of wme pointers and sort them --- */
   list = static_cast<wme**>(allocate_memory(thisAgent, n * sizeof(wme *), MISCELLANEOUS_MEM_USAGE));
   attr = 0;
   for (w = id->id.impasse_wmes; w != NIL; w = w->next)
       list[attr++] = w;
   for (w = id->id.input_wmes; w != NIL; w = w->next)
       list[attr++] = w;
   for (s = id->id.slots; s != NIL; s = s->next) {
       for (w = s->wmes; w != NIL; w = w->next)
           list[attr++] = w;
       for (w = s->acceptable_preference_wmes; w != NIL; w = w->next)
           list[attr++] = w;
   }
   //qsort(list, n, sizeof(wme *), compare_attr);


   *num_attr = n;
   return list;


}

SoarSTLWMEPoolList* get_augs_of_id_STL(agent* thisAgent, Symbol * id, tc_number tc)
{
	// notice how we give the constructor our custom SoarSTLWMEPoolList with the agent and memory type to use
	SoarSTLWMEPoolList* list = new SoarSTLWMEPoolList(SoarMemoryPoolAllocator<wme*>(thisAgent, &thisAgent->wme_pool));

	slot *s;
    wme *w;

    if (id->common.symbol_type != IDENTIFIER_SYMBOL_TYPE)
        return NULL;
    if (id->id.tc_num == tc)
        return NULL;
    id->id.tc_num = tc;

	// build list of wmes
    for (w = id->id.impasse_wmes; w != NIL; w = w->next)
        list->push_back(w);
    for (w = id->id.input_wmes; w != NIL; w = w->next)
        list->push_back(w);
    for (s = id->id.slots; s != NIL; s = s->next) {
        for (w = s->wmes; w != NIL; w = w->next)
            list->push_back(w);
        for (w = s->acceptable_preference_wmes; w != NIL; w = w->next)
            list->push_back(w);
    }

    return list;
}
