/////////////////////////////////////////////////////////////////
// WMElement class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : Sept 2004
//
// This is the base class for all working memory elements.
//
/////////////////////////////////////////////////////////////////

#include "sml_ClientWMElement.h"
#include "sml_ClientAgent.h"
#include "sml_ClientIdentifier.h"

#include "sml_EmbeddedConnection.h"	// For direct methods
#include "sml_ClientDirect.h"

using namespace sml ;

WMElement::WMElement(Agent* pAgent, Identifier* pID, char const* pAttributeName, long timeTag)
{
	m_TimeTag = timeTag ;

	// Record the agent that owns this wme.
	m_Agent			= pAgent ;

	// id and attribute name can both be NULL if this is at the top of the tree.
	if (pAttributeName)
		m_AttributeName = pAttributeName ;

	m_ID = NULL ;

	if (pID)
		m_ID = pID->GetSymbol() ;

#ifdef SML_DIRECT
	m_WME = 0 ;
#endif
}

WMElement::~WMElement(void)
{
#ifdef SML_DIRECT
	// If we're using the direct connection methods, we need to release the gSKI object
	// that we own.
	if (m_WME && GetAgent()->GetConnection()->IsDirectConnection())
	{
		((EmbeddedConnection*)GetAgent()->GetConnection())->DirectReleaseWME(m_WME) ;
	}
#endif
}

void WMElement::GenerateNewTimeTag()
{
	// Generate a new time tag for this wme
	m_TimeTag = GetAgent()->GetWM()->GenerateTimeTag() ;
}
