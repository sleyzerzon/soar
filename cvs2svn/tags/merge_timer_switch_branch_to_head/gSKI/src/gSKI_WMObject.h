/********************************************************************
* @file igski_wmobject.h 
*********************************************************************
* @remarks Copyright (C) 2002 Soar Technology, All rights reserved. 
* The U.S. government has non-exclusive license to this software 
* for government purposes. 
*********************************************************************
* created:	   6/13/2002   15:00
*
* purpose: 
*********************************************************************/
#ifndef GSKI_WMOBJECT_H
#define GSKI_WMOBJECT_H

#include "IgSKI_Iterator.h"
#include "IgSKI_WMObject.h"
#include "IterUtils.h"
#include "gSKI_Iterator.h"

#include "symtab.h"

#include <map>
#include <vector>
#include <string>

namespace gSKI {

   class WorkingMemory;
   class gSymbol;
   class Wme;

   /**
    *	@brief This class represents an object in working memory.
    *
    * An object is a collection of wmes that share the same identifier.
    *  A WME object is similar to an object in an object-oriented language.
    *  @li It has aaddress or handle (its id)
    *  @li It has properties (its wmes)
    *  @li It can contain arbitrary nested structures (wmes point to other objects)
    *  @li You can move them around by reference or copy.
    *
    * They differ from objects in object oriented systems mainly in that they:
    *  @li Are not strongly typed
    *  @li Their list of properties can change at run time.
    *
    * Objects are always copies of what is really in working memory.  The client
    *  always owns individual objects and collections of objects are owned by
    *  the iterator pointing at them.  Releasing a WMObject does not remove
    *  the object from agent working memory or in any way modify agent working
    *  memory.
    *
    * Accessing properties of an object (wmes) causes the return of object wmes that
    *  existed at the time of the call to retrieve the wmes. The set of wmes
    *  present within an object changes as the agent deliberates.  Even the object
    *  can go away in agent working memory.  However, since the client owns
    *  WMObjects, the WMObject does not go away until it is released (through
    *  the Release method or through the iterator referencing it).
    *
    * Objects are implemented as very lightweight wrappers, so storing them
    *  should not cause memory bloat.
    *
    */
   class WMObject: public IWMObject {
   public:
      
      /**
       * @brief Constructors
       *
       */
      WMObject(WorkingMemory* manager, Symbol* sym);
	 
      /** 
       * @brief Destructor
       *
       * The destructor does not release any wmes.  It simply 
       *  frees up the small amount of memory it uses.
       */
       virtual ~WMObject();

      /** 
       * @brief Gets the identifier for this object
       *
       * Each working memory object has a uniqueaddress identifying it.
       *  This method returns that identifier.
       *
       * @param err Pointer to client-owned error structure.  If the pointer
       *          is not 0 this structure is filled with extended error
       *          information.  If it is 0 (the default) extended error
       *          information is not returned.
       *
       * @return A constant symbol representing the ID of the object. This symbol
       *          is owned by the object and should NOT be Released.
       */
     const ISymbol* GetId(Error* err = 0) const;

      /** 
       * @brief Returns all of the objects currently referencing this object.
       *
       * This method returns an iterator into a list of all of the objects
       *  that refer to this object at the instance in time when this method
       *  is called.
       *
       * @param err Pointer to client-owned error structure.  If the pointer
       *          is not 0 this structure is filled with extended error
       *          information.  If it is 0 (the default) extended error
       *          information is not returned.
       *
       * @return An iterator into the list of all objects currently referencing
       *           this object.  This iterator may point to an empty list if
       *           this object has been removed from working memory. 
       *           The pointer returned is never 0.
       */
      /* TODO: Make sure we DONT need the reference type (attribute or value) */
     tIWMObjectIterator* GetObjectsReferencing(Error* err = 0) const;

      /** 
       * @brief Returns all of the objects this object references
       *
       * This method returns an iterator into a list of all of the objects
       *  this object is referencing (through its wmes) at the time this
       *  method is called.
       *
       * @param err Pointer to client-owned error structure.  If the pointer
       *          is not 0 this structure is filled with extended error
       *          information.  If it is 0 (the default) extended error
       *          information is not returned.
       *
       * @return An iterator into the list of all objects referenced by this
       *          object.  This list may be empty if this object is not currently
       *          referencing any other objects. The pointer returned is
       *          never 0.
       */
     tIWMObjectIterator* GetObjectsReferencedBy(Error* err = 0) const;

      /** 
       * @brief Returns all of the wmes currently referencing this object
       *
       * This method returns an iterator into a list of all of the wmes 
       *  referencing this object at the time this method was called.
       *
       * @param err Pointer to client-owned error structure.  If the pointer
       *          is not 0 this structure is filled with extended error
       *          information.  If it is 0 (the default) extended error
       *          information is not returned.
       *
       * @return An iterator into the list of all attributes referencing this
       *          object.  This list may be empty if this object is not referenced
       *          by any wmes (essentially this means that the object does not
       *          exist in working memory any more). The pointer returned is
       *          never 0.
       */
     tIWmeIterator* GetWmesReferencing(Error* err = 0) const;

      /** 
       * @brief Get subset of WMEs owned by this object 
       * 
       * Call this method when you want to retrieve a subset of the WMEs
       *  owned by this object.  For example, to retrieve all non-object
       *  WME's owned by this object pass 0 as the attributeName and
       *  gSKI_DOUBLE | gSKI_INT | gSKI_STRING as the valueType parameter.  
       *  To retrieve only numeric WMEs, pass 0 as a the attributeName
       *  and gSKI_DOUBLE | gSKI_INT as the valueType.  To get any WME
       *  with the name "radar" pass the string "radar" in the attributeName
       *  parameter and gSKI_ANY_SYMBOL as the second parameter.
       *
       * @see IWMObject::GetAllWmes
       *
       * @param attributeName C-style string used to match against the WME's 
       *           attribute value.  If the WME attribute is not a string,
       *           the following conversions occur: 
       *           @li If the attribute is a number, attributeName is converted
       *                to a number and then matched.
       *           @li If the attribute is a is converted to a string and matched.  This
       *           pointer can be 0 to match against any name.
       * @param valueType One or more types of symbol types (bitwise ORed together)
       *                    specifying a filter of WMEs to retrieve. (see
       *                    egSKISymbolType).  Pass gSKI_ANY_SYMBOL to choose
       *                    wmes with any type of value.
       * @param err Pointer to client-owned error structure.  If the pointer
       *          is not 0 this structure is filled with extended error
       *          information.  If it is 0 (the default) extended error
       *          information is not returned.
       *
       * @return A pointer to an iterator containing all wmes owned by this
       *            object that meet the filter criteria given by the
       *            first two parameters.  This pointer will never be 0.
       */
     tIWmeIterator* GetWMEs(const char*           attributeName = 0,
                            egSKISymbolType       valueType     = gSKI_ANY_SYMBOL,
                            Error*                err           = 0) const;

      /** 
       * @brief  Gets the type of this object
       *
       * There are two special objects in Soar.  The first is an operator
       *  the second is a state.  Each of these objects has a special
       *  advanced representation in the gSKI API.  You can use this
       *  method to help in "downcasting" a general object to one of
       *  the special objects.  Call this method to determine what type
       *  of object this is, then call one of the ToXXX methods to
       *  cast it to the more derived type.
       *
       * @see IWMObject::ToState
       *
       * @param err Pointer to client-owned error structure.  If the pointer
       *          is not 0 this structure is filled with extended error
       *          information.  If it is 0 (the default) extended error
       *          information is not returned.
       *
       * @return One of the egSKIWMObjectType enumerated values indicating
       *          what type of object this is.
       */
     egSKIWMObjectType GetObjectType(Error* err = 0) const;

      /** 
       * @brief Downcasts this IWMObject to a derived IState object.
       *
       * Call this method when you have a pointer to an IWMObject that represents
       *  a state, and you need the additional functionality of the IState 
       *  interface.  This could happen if you are iterating over some subset
       *  of working memory and need to do special processing when you reach
       *  a state.
       *
       * @note Prefer to use the IAgent::GetTopState and IAgent::GetBottomState
       *         to obtain state pointers over downcasting as it will be more
       *         efficient and less error prone than iterating over memory
       *         and downcasting.
       *
       * Possible Errors:
       *  @li gSKIERR_WRONG_TYPE - if this working memory object is not a state
       *              object
       *
       * @see IWMObject::GetObjectType
       *
       * @param err Pointer to client-owned error structure.  If the pointer
       *          is not 0 this structure is filled with extended error
       *          information.  If it is 0 (the default) extended error
       *          information is not returned.
       *
       * @return A pointer to this same IWMObject downcasted to the more
       *          derived IState object pointer.  If the object is not
       *          really a state object, 0 is returned and err will contain
       *          extended error information.
       */
     IState* ToState(Error* err = 0) const;

      /**
       * @brief Returns true if the Wme has been removed from the agent's working memory
       *
       * This method returns true if the WMObject has been removed from the agent's
       *  working memory.  This function is necessary due to the dynamic nature of
       *  Soar's working memory. A WMObject obtained at one point in the decision
       *  cycle may no longer exist in the agent's working memory at a later time.
       *
       * @note This method applies to the memory unit that owns this WMObject.
       *        If this object is part of main working memory, this method will
       *        tell you if the WMObject still exists in main working memory.
       *        If it is part of the input link or the ouput link, it tells you
       *        whether or not it exists in the input or output link.  Nothing
       *        can be removed from a static view, so this method will always
       *        return true for WMObjects owned by static views.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @return true if the WMObject has been removed from working memory, false if
       *          it still exists in the agent's current working memory.
       */
     bool HasBeenRemoved(Error* err = 0) const;

      /**
       * @brief Compares two WMObjects for equality.
       * 
       * This method compares two WMObjects for equality. WMObjects are equal
       *   if all of their attributes are equal.  Two WMObjects with the
       *   same id are always equal if they are owned by the same working
       *   memory view.
       *
       * @param object The WMObject with which to compare this WMObject
       * @param err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @return true if the WMObject's are equal and false if they are not
       */
     bool IsEqual(IWMObject* object, Error* err = 0) const;


     /**
      * @brief
      */
     void AddReferencedWme(Wme* wme);
     void AddReferencedObject(WMObject* obj, Wme* wme);
     void AddReferencingObject(WMObject* obj, Wme* wme);
     void ReInitialize();
     tIWMObjectIterator* GetObjectsReferencedByAttribute(const std::string& attr) const;

     // TODO: Document these methods
     void Release(Error* err=0);
     bool IsClientOwned(Error* err=0) const;
     
   private:
     gSymbol* m_gsym;
     WorkingMemory* m_manager;

     std::vector<Wme*> m_vwmes;
     std::map<Wme*,WMObject*> m_parentmap;
     std::map<Wme*,WMObject*> m_childmap;

     typedef std::map<Wme*,WMObject*>::iterator tWmeObjIt;

     typedef FwdContainerType< std::vector<IWMObject *> >  tWMObjectVec;
     typedef IteratorWithRelease<tWMObjectVec::V, tWMObjectVec::t>  tWMObjectIter;
  };

}


#endif
