#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gSKI_InputWme.cpp          
*********************************************************************
* created:	   7/22/2002   16:06
*
* purpose: 
*********************************************************************/

#include "gSKI_InputWme.h"

#include <vector>

#include "io.h"

#include "gSKI_InputWMObject.h"
#include "gSKI_InputWorkingMemory.h"
#include "IgSKI_Production.h"
#include "gSKI_Agent.h"
#include "gSKI_Symbol.h"
#include "gSKI_Error.h"
#include "MegaAssert.h"

//
// Explicit Export for this file.
//#include "MegaUnitTest.h"


//DEF_EXPOSE(gSKI_InputWme);

namespace gSKI
{
   InputWme::InputWme(InputWorkingMemory* manager, wme* wme):
     m_manager(manager),
     m_owningobject(0),
     m_attribute(0),
     m_value(0),
     m_rawwme(wme),
     m_removeWme(false)
   {
      MegaAssert( false, "Not implemented yet!");
   }

   InputWme::InputWme( InputWorkingMemory* manager, 
                       InputWMObject* iobj,
                       gSymbol* attribute,
                       gSymbol* value ):
     m_manager(manager),
     m_owningobject(iobj),
     m_attribute(attribute),
     m_value(value),
     m_rawwme(0),
     m_removeWme(false)
   {

     MegaAssert( m_manager != 0, "The manager of this input wme cannot be null!");
     MegaAssert( m_owningobject != 0, "The owning object of this wme cannot be null!");
     MegaAssert( attribute != 0, "The attribute of this wme cannot be null!");
     MegaAssert( value != 0, "The value of this wme cannot be null!");

     gSymbol::ConvertSymbol(m_attribute)->AddRef();
     gSymbol::ConvertSymbol(m_value)->AddRef();

     // No reference is added to the owning object to avoid a circular reference.

     Update();
   }

  InputWme::~InputWme() 
  {
      m_attribute->Release();
      m_value->Release();
  }

  IWMObject* InputWme::GetOwningObject(Error* err) const
  {
    ClearError(err);
    
    if ( m_owningobject != 0 ) {
       return m_owningobject;
    } else {
       // TODO: Look for it in working memory (should this ever happen)
       return 0;
    }
  }

  const ISymbol* InputWme::GetAttribute(Error* err) const
  {
    ClearError(err);
    return m_attribute;
  }

  const ISymbol* InputWme::GetValue(Error* err) const
  {
    ClearError(err);
    return m_value;
  }

  // TODO: This method should probably return an unsigned long
  long InputWme::GetTimeTag(Error* err) const
  {
    ClearError(err);
      return m_rawwme ? m_rawwme->timetag : 0;
    }

  egSKISupportType InputWme::GetSupportType(Error * err) const
  {
    ClearError(err);

    return gSKI_I_SUPPORT;
  }

  tIProductionIterator* InputWme::GetSupportProductions(Error* err) const
  {
    ClearError(err);

    // There are no supporting productions for input wmes so return an empty iterator
    std::vector<IProduction*> temp;
    return new tProductionIter(temp);
  }

  bool InputWme::HasBeenRemoved(Error* err) const
  {
    ClearError(err);

      return m_rawwme == 0;
  }

  bool InputWme::IsEqual(IWme* wme, Error* err) const
  {
     ClearError(err);
     MegaAssert(false, "NOT IMPLEMENTED YET!");
     return false;
  }

  void InputWme::SetOwningObject(InputWMObject* obj)
  {
      m_owningobject = obj;
    } 

  bool InputWme::AttributeEquals( const std::string& attr ) const
  {
    if ( m_attribute == 0 ) return false;
    if ( m_attribute->GetType() != gSKI_STRING ) 
      return false;
    
    return ( m_attribute->GetString() == attr);
    
  }

  void InputWme::Remove() 
  {
    m_removeWme = true;
    Update();
  }

  void InputWme::Update()
  {
     // Adding the WME directly to working memory if were in the input or output phase
     egSKIPhaseType curphase = m_manager->GetAgent()->GetCurrentPhase();
      if ( curphase == gSKI_INPUT_PHASE ) 
      {
       // If there is no raw wme for this than make one 
       // (unless it has already been removed)
         if ( m_rawwme == 0 && !m_removeWme ) 
         {
	 agent* a = m_manager->GetSoarAgent();
	 Symbol* id = m_owningobject->GetSoarSymbol();
	 Symbol* attr = m_attribute->GetSoarSymbol();
	 Symbol* val = m_value->GetSoarSymbol();
            m_rawwme = add_input_wme( a, id, attr, val );

	 MegaAssert( m_rawwme != 0, "Trouble adding an input wme!");
       }

       // Removing any wme's scheduled for removal
         if ( m_removeWme && m_rawwme != 0 ) 
         {
            Bool retvalue =  remove_input_wme(m_manager->GetSoarAgent(), m_rawwme);
	 MegaAssert( retvalue, "Trouble removing an input wme!");
            m_rawwme = 0;

	 // Detaching this object from the other input objects
            MegaAssert( m_owningobject != 0, "Invalid owning object for InputWme." );
            if(m_owningobject != 0 )
            {
               m_owningobject->RemoveReferencedWme(this);
            }
	//	 m_owningobject->RemoveReferencedWme(this);
		SetOwningObject(0);

		// After the wme is really removed, we release ourselves.  We do it this way
		// because a client calling "RemoveWme()" has no way to know when the gSKI Wme object
		// can be released, because the removal of the kernel wme won't occur until the next
		// input phase after RemoveWme is called.  Thus "RemoveWme" now includes a reference decrement
		// as part of its actions, providing the client a good way to do clean up.
		this->Release() ;
       }
     }
  }

}
