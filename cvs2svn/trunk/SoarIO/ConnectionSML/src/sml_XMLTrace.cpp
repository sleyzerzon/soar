#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/////////////////////////////////////////////////////////////////
// XMLTrace class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : April 2005
//
// Represents a piece of structured trace information generated
// during the course of a run.
//
// This class is related to the ClientTraceXML class.  This class
// is optimized for creating trace objects while the client class
// supports reading the trace information.
//
/////////////////////////////////////////////////////////////////

#include "sml_XMLTrace.h"
#include "assert.h"

using namespace sml ;

XMLTrace::XMLTrace()
{
	m_XML = new ElementXML() ;
	m_XML->SetTagName(sml_Names::kTagTrace) ;

	m_pCurrentTag = NULL ;
}

XMLTrace::~XMLTrace()
{
	delete m_pCurrentTag ;
	m_pCurrentTag = NULL ;

	delete m_XML ;
	m_XML = NULL ;
}

/*************************************************************
* @brief	Reinitialize this XML trace object so we can
*			re-use it.
*************************************************************/
void XMLTrace::Reset()
{
	delete m_pCurrentTag ;
	m_pCurrentTag = NULL ;

	delete m_XML ;
	m_XML = NULL ;

	m_XML = new ElementXML() ;
	m_XML->SetTagName(sml_Names::kTagTrace) ;
}

/*************************************************************
* @brief	Start a new tag.
*
* Subsequent calls to AddAttribute() will work with this tag
* until EndTag() is called.
*
* NOTE: The tag name must remain in scope forever (i.e. it should be
* a constant).  This allows us to save time by not copying the string.
*************************************************************/
void XMLTrace::BeginTag(char const* pTagName)
{
	ElementXML* pChild = new ElementXML() ;
	pChild->SetTagNameFast(pTagName) ;

	ElementXML_Handle hChild = NULL ;

	// The new tag is created as a child of the current tag (if we have one)
	// or the root if we don't.  Adding the child delete the pChild object
	// normally, so we have to create a second one.  Perhaps we should design AddChild?
	if (m_pCurrentTag == NULL)
		hChild = m_XML->AddChild(pChild) ;
	else
		hChild = m_pCurrentTag->AddChild(pChild) ;

	delete m_pCurrentTag ;
	m_pCurrentTag = new ElementXML(hChild) ;
	m_pCurrentTag->AddRefOnHandle() ;
}

/*************************************************************
* @brief	Terminate the current tag.
*
* The tag name is just used for error checking to make sure
* everything is properly balanced.
*************************************************************/
void XMLTrace::EndTag(char const* pTagName)
{
	assert(m_pCurrentTag) ;

	if (!m_pCurrentTag)
		return ;

#ifdef _DEBUG
	// Make sure we're closing the tag we expect
	assert (m_pCurrentTag->IsTag(pTagName)) ;
#endif

	// Update the current tag to be its parent
	bool hasParent = m_pCurrentTag->GetParent(m_pCurrentTag) ;

	if (!hasParent)
	{
		delete m_pCurrentTag ;
		m_pCurrentTag = NULL ;
	}
}

/*************************************************************
* @brief	Adds an attribute to the current tag.
*
* NOTE: The attribute name must remain in scope forever (i.e. it should be
* a constant).  This allows us to save time by not copying the string.
* Naturally, the value doesn't have this restriction.
*************************************************************/
void XMLTrace::AddAttribute(char const* pAttributeName, char const* pValue)
{
	assert(m_pCurrentTag) ;

	if (!m_pCurrentTag)
		return ;

	m_pCurrentTag->AddAttributeFast(pAttributeName, pValue) ;
}

/*************************************************************
* @brief Releases ownership of the underlying XML object.
*
*		 The caller must call releaseRef() on this handle
*		 when it is done with it.
*
*		This should only be called when the message is actually
*		being sent.  It shouldn't be used when building up the message.
*
*		NOTE: After doing this the XMLTrace object should either be
*		deleted or Reset() should be called on it.
*************************************************************/
ElementXML_Handle XMLTrace::Detach()
{
	ElementXML_Handle hXML = m_XML->Detach() ;

	delete m_pCurrentTag ;
	m_pCurrentTag = NULL ;

	delete m_XML ;
	m_XML = NULL ;

	return hXML ;
}