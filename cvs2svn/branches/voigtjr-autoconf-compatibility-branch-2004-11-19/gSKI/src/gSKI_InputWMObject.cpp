#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/********************************************************************
* @file gski_inputwmobject.cpp 
*********************************************************************
* @remarks Copyright (C) 2002 Soar Technology, All rights reserved. 
* The U.S. government has non-exclusive license to this software 
* for government purposes. 
*********************************************************************
* created:	   7/11/2002   14:54
*
* purpose: 
********************************************************************/

#include "gSKI_InputWMObject.h"
#include "gSKI_InputWme.h"

#include "IgSKI_Iterator.h"
#include "IgSKI_InputProducer.h"

#include "gSKI_InputWorkingMemory.h"
#include "gSKI_Wme.h"
#include "gSKI_Error.h"
#include "gSKI_Agent.h"
#include "gSKI_Symbol.h"

#include "MegaAssert.h"

#include "symtab.h"

//
// Explicit Export for this file.
//#include "MegaUnitTest.h"
//DEF_EXPOSE(gSKI_InputWMObject);

namespace gSKI 
{

   /*
     ===============================

     ===============================
   */

   InputWMObject::InputWMObject(InputWorkingMemory* manager, Symbol* sym):
      m_sym(sym),
      m_gsym(0),
      m_manager(manager),
      m_letter(sym->id.name_letter)
   {
      MegaAssert( m_manager != 0, "Manager for InputWMObject cannot be null!");
      MegaAssert( m_sym != 0, "Symbol for InputWMObject cannot be null!" );
      MegaAssert( m_sym->common.symbol_type == IDENTIFIER_SYMBOL_TYPE,
                  "Specified symbol must be an identifier type!" );

      // Creating the GSKI symbol.  This will add a reference to the symbol.
      // but since I also keep around the raw symbol (and release it) I'll add
      // another refcount here
      m_gsym = new gSymbol(m_manager->GetSoarAgent(), 
                           m_sym,
                           this, false);
      m_manager->registerObjectSymbol(m_gsym);

      symbol_add_ref(m_sym);
   }

   /*
     ===============================

     ===============================
   */
   
   InputWMObject::InputWMObject(InputWorkingMemory* manager, char letter):
      m_sym(0),
      m_gsym(0),
      m_manager(manager),
      m_letter(letter)
   {
      MegaAssert( m_manager != 0, "Manager for InputWMObject cannot be null!");

      // Creating a new identifier symbol for this WMObject
      MegaAssert( m_manager->GetSoarAgent(), 
		  "Manager must belong to non-null agent!");
      m_sym = get_new_io_identifier(m_manager->GetSoarAgent(),
                                    m_letter);
                          
      // Creating the GSKI symbol also
      m_gsym = new gSymbol(m_manager->GetSoarAgent(), m_sym, this, false);
      m_manager->registerObjectSymbol(m_gsym);
   }

   /*
     ===============================

     ===============================
   */

   InputWMObject::~InputWMObject() 
   {
      MegaAssert(m_manager != 0, "Manager for InputWMObject cannot be null!");

      symbol_remove_ref(m_manager->GetSoarAgent(),	m_sym);

      if(m_gsym)
      {
         m_gsym->Release();
      }
   }

   /*
     ===============================

     ===============================
   */
   const ISymbol* InputWMObject::GetId(Error* err) const
   {
      ClearError(err);

      return m_gsym;
   }

   /*
     ===============================

     ===============================
   */

   tIWMObjectIterator* InputWMObject::GetObjectsReferencing(Error* err) const
   {
      ClearError(err);

      MegaAssert(false, "NOT IMPLEMENTED YET!");

      return 0;
   }

   /*
     ===============================

     ===============================
   */

   tIWMObjectIterator* InputWMObject::GetObjectsReferencedBy(Error* err) const
   {
      ClearError(err);

      MegaAssert(false, "NOT IMPLEMENTED YET!");

      return 0;
   }

   /*
     ===============================

     ===============================
   */

   tIWmeIterator* InputWMObject::GetWmesReferencing(Error* err) const
   {
      ClearError(err);

      MegaAssert(false, "NOT IMPLEMENTED YET!");

      return 0;
   }

   /*
     ===============================

     ===============================
   */

   tIWmeIterator* InputWMObject::GetWMEs(const char* attributeName,
                                         egSKISymbolType valueType,
                                         Error* err) const
   {
      ClearError(err);

      std::vector<IWme*> matchingWmes;

      // Iterating over the referenced objects
      for ( std::set<InputWme*>::const_iterator it = m_vwmes.begin();
            it != m_vwmes.end();
            ++it ) {

         InputWme* wme = *it;

		 // The attributeName can be NULL, which should match all attributes.
         if ( (attributeName == 0 || wme->AttributeEquals( attributeName )) 
              && valueType & wme->GetValue()->GetType() ) {
            wme->AddRef();
            matchingWmes.push_back(wme);
         }
      
      }
    
      return new tWmeIter(matchingWmes);
   }

   /*
     ===============================

     ===============================
   */

   egSKIWMObjectType InputWMObject::GetObjectType(Error* err) const
   {
      ClearError(err);

      return gSKI_SIMPLE_OBJECT;
   }

   /*
     ===============================

     ===============================
   */

   IState* InputWMObject::ToState(Error* err) const
   {
      ClearError(err);

      MegaAssert(false, "NOT IMPLEMENTED YET!");

      return 0;
   }

   /*
     ===============================

     ===============================
   */

   bool InputWMObject::HasBeenRemoved(Error* err) const
   {
      ClearError(err);

      MegaAssert(false, "NOT IMPLEMENTED YET!");

      return false;
   }

   /*
     ===============================

     ===============================
   */

   bool InputWMObject::IsEqual(IWMObject* object, Error* err) const
   {
      ClearError(err);

      MegaAssert(false, "NOT IMPLEMENTED YET!");

      return false;
   }

   /*
     ===============================
    
     ===============================
   */
   void InputWMObject::ReInitialize() 
   {
      m_vwmes.clear();
      m_parentmap.clear();
      m_childmap.clear();
   }

   /*
     ===============================
    
     ===============================
   */
   void InputWMObject::AddReferencedWme(InputWme* wme)
   {
      MegaAssert( wme != 0 , "Can't add a null input wme!");
      if ( wme == 0 ) return;

      m_vwmes.insert(wme);
      wme->SetOwningObject(this);

      // If the value of this wme is an identifier then add the object
      // as a referenced object
      if ( wme->GetValue()->GetType() == gSKI_OBJECT ) {

         InputWMObject* iobj = 
          m_manager->GetOrCreateObjectFromInterface(wme->GetValue()->GetObject());
        
         m_childmap.insert(std::pair<InputWme*, InputWMObject*>(wme,iobj));
      }

   }

   /*
     ===============================
    
     ===============================
   */
   void InputWMObject::RemoveReferencedWme(InputWme* wme)
   {
      // Finding the wme to remove
      std::set<InputWme*>::iterator it = m_vwmes.find(wme);
      if ( it != m_vwmes.end() ) {
         // Removing the wme from the internal data structures
         m_vwmes.erase(it);
         
         // Checking in the child map if this wme references an object
         tWmeObjIt it2 = m_childmap.find(wme);
         if ( it2 != m_childmap.end() ) {

            // If it does then remove the entry from the map
            m_childmap.erase(it2);
         }

      }
   }

   /*
     ===============================
    
     ===============================
   */
   void InputWMObject::AddReferencedObject(InputWMObject* obj, InputWme* wme)
   {
      MegaAssert( obj != 0, "Can't add a null input object!");
      MegaAssert( wme != 0, "Can't use a null wme to add a null input object!");
      if ( obj == 0 || wme == 0 ) return;

      m_childmap.insert(std::pair<InputWme*,InputWMObject*>(wme, obj));
   }

   /*
     ===============================
    
     ===============================
   */
   void InputWMObject::RemoveReferencedObject(InputWMObject* obj, InputWme* wme) 
   {
      tWmeObjIt it = m_childmap.find(wme);
      if ( it != m_childmap.end() ) {
         InputWMObject* tobj = it->second;
         
         // If the object and wme are referenced by this object then remove them
         if ( tobj == obj ) {
            m_vwmes.erase(wme);
            m_childmap.erase(it);
         }

      }
   }

   /*
     ===============================
    
     ===============================
   */
   void InputWMObject::AddReferencingObject(InputWMObject* obj, InputWme* wme)
   {
      MegaAssert( obj != 0, "Can't add a null input object!");
      MegaAssert( wme != 0, "Can't use a null wme to add a null input object!");
      if (obj == 0 || wme == 0 ) return;

      m_parentmap.insert(std::pair<InputWme*,InputWMObject*>(wme, obj));
   }

   /*
     ===============================
    
     ===============================
   */
   tIWMObjectIterator* 
   InputWMObject::GetObjectsReferencedByAttribute(const std::string& attr) const
   {
      std::vector<IWMObject*> matchingObjects;

      // Iterating over the referenced objects
      for ( std::map<InputWme*,InputWMObject*>::const_iterator it = m_childmap.begin();
            it != m_childmap.end();
            ++it ) {

         InputWme* wme = it->first;
         if ( wme->AttributeEquals( attr )) {
            MegaAssert(it->second, "Input wme pointing to a null wmo!");
            it->second->AddRef();
            matchingObjects.push_back(it->second);
         }
      
      }
    
      return new tWMObjectIter(matchingObjects);
   }

   /*
     ===============================    
     ===============================
   */
   void InputWMObject::AddInputProducer(IInputProducer* producer)
   {
      m_producerset.insert(producer);
   }

   /*
     ===============================    
     ===============================
   */
   void InputWMObject::RemoveInputProducer(IInputProducer* producer)
   {
      std::set<IInputProducer*>::iterator it = m_producerset.find(producer);

      if ( it != m_producerset.end() ) {
         m_producerset.erase(it);
      }
   }

   /*
     ===============================    
     ===============================
   */
   void InputWMObject::DeleteInputProducers()
   {
      // Commented out by Dave Ray 4/29/2004. Deleting the input producers
      // makes no sense. Since they come from outside of gSKI (through
      // AddInputProducer) gSKI has no way of knowing what the correct
      // way to delete these objects is. Maybe a better approach would
      // be to add a callback to IInputProducer to tell it when it is
      // removed?
      //std::set<IInputProducer*>::iterator it = m_producerset.begin();
      //for ( ; it != m_producerset.end(); ++it ) {
      //   delete *it;
      //}

      // Clearing out the set of now invalid pointers
      m_producerset.clear();
   }

   /*
     ===============================    
     ===============================
   */
  void InputWMObject::Update(std::set<InputWMObject*>& processedObjects)
  {
    // First checking that this object hasn't already been added to the set of 
    // processed objects ( adding it and processing it if it hasn't; returning
    // if it has already been processed )
    std::set<InputWMObject*>::iterator it = processedObjects.find(this);
    if ( it != processedObjects.end() ) {
      return;
    } else {
      processedObjects.insert(this);
    }

    // Now calling all the input producers associated with this input wm object
    // TODO: There can be problems here if you try to add IInputProducers to this object
    // from other IInputProducers (check set iterators to make sure this isn't a problem)
    for ( std::set<IInputProducer*>::iterator it = m_producerset.begin();
	  it != m_producerset.end();
	  ++it ) {

      int setsize = m_producerset.size();
      setsize = setsize;

      // Invoking the input producers
      IInputProducer* producer = (*it);
      if ( producer != 0 ) {
	producer->Update(m_manager, this);
      } else {
	MegaAssert( false, "Null IInputProducer registered with InputWMObject!");
      }

    }

    // Now updating all the input wmes of this object
    for ( std::set<InputWme*>::iterator it = m_vwmes.begin();
	  it != m_vwmes.end();
	  ++it ) {
      
      // Updating these wmes ( which creates raw kernel wmes from gSKI InputWmes
      // if they haven't already been created )
      InputWme* iwme = (*it);
      if ( iwme != 0 ) {
	iwme->Update();
      } else {
	MegaAssert( false, "Null InputWme registered with InputWMObject!" );
      }

    }

    // Now updating all the child objects
    for ( std::map<InputWme*, InputWMObject*>::iterator it = m_childmap.begin();
	  it != m_childmap.end();
	  ++it ) {

      // Updating the child objects (and remembering to pass the set of processed
      // objects to avoid being caught in cycles
      InputWMObject* obj = it->second;
      if ( obj != 0 ) {
	obj->Update(processedObjects);
      } else {
	MegaAssert( false, 
		    "Null InputWMObject registered as child of another InputWMObject!");
      }
    
    }

  }

}
