/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_event_system_kernel.h 
*********************************************************************
* created:	   6/17/2002   13:16
*
* purpose: 
*********************************************************************/

#ifndef GSKI_EVENT_SYSTEM_KERNEL_H
#define GSKI_EVENT_SYSTEM_KERNEL_H

/* We have to forward declare these to keep from having a circular reference */
struct agent_struct;
union  symbol_union;

/**
 * @brief Very general callback function
 *
 * All callbacks are sent through this generic interface so the kernel can be completely
 *  unlinked from the gSKI implementation.
 *
 * @li The first parameter is the event id (one of those listed above)
 * @li The second parameter is an unsigned char that behaves like a boolean.  Its value is 1 (true)
 *         if the event already occured and 0 (false) if it is about to occur 
 *         (before event and after event flag)
 * @li The third parameter is the gSKI object that will recieve the callback.
 * @li The fourth parameter is a pointer to the internal "kernel" object.  This is 0 until the 
 *           internal kernel object is created.
 * @li The fifth parameter is the good old agent structure (this will be 0 for callbacks where
 *           an agent structure doesn't make sense)
 * @li The sixth parameter is the event specific data.
 *
 */
typedef void (*gSKI_K_CallbackFunctionPtr)(unsigned long         eventId, 
                                           unsigned char         eventOccured,
                                           void*                 object, 
                                           struct agent_struct*  soarAgent, 
                                           void*                 data);

/**
 * @brief Structure used to store callback functions and the object to recieve the callback.
 */
typedef struct gSKI_K_CallbackData_struct {
   
   /** Pointer to callback function.  If this is 0, a callback is not made */
   gSKI_K_CallbackFunctionPtr    function;

   /** Pointer to the gski object that will recieve teh callback */
   void*                         gski_object;

} gSKI_K_CallbackData;

/** ----------------------------- COMPLEXT EVENT DATA --------------------------------*/

/** 
 * @brief Structure for WMObject creation callback
 */
typedef struct gSKI_K_WMObjectCallbackData_struct {

   /** Pointer to the object being created */
   union symbol_union*           wm_new_object;

   /** Attribute used to reference new object. NIL for new state object. */
   union symbol_union*           wm_referencing_attr;

   /** Pointer to the object that is referencing the new object. NIL for new state object. */
   union symbol_union*           wm_referencing_object;

} gSKI_K_WMObjectCallbackData;

#endif
