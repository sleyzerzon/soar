/////////////////////////////////////////////////////////////////
// WorkingMemory class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : Sept 2004
//
// This class is used to represent Soar's working memory.
// We maintain a copy of this on the client so we can just
// send changes over to the kernel.
//
// Basic method is that working memory is stored as a tree
// of Element objects.
// When the client makes a change to the tree, we modify the tree
// and add the change to the list of changes to make to WM.
// At some point, we actually send that list of changes over.
// We should be able to be clever about collapsing changes together
// in the list of deltas (e.g. change value A->B->C can remove the
// A->B change (since it's overwritten by B->C).
//
/////////////////////////////////////////////////////////////////

#include "sml_ClientWorkingMemory.h"
#include "sml_ClientIdentifier.h"
#include "sml_ClientAgent.h"
#include "sml_ClientKernel.h"
#include "sml_Connection.h"
#include "sml_ClientStringElement.h"
#include "sml_ClientIntElement.h"
#include "sml_ClientFloatElement.h"
#include "sml_TagWme.h"
#include "sml_StringOps.h"

using namespace sml ;

WorkingMemory::WorkingMemory()
{
	m_InputLink  = NULL ;
	m_OutputLink = NULL ;
	m_Agent		 = NULL ;
}

WorkingMemory::~WorkingMemory()
{
	delete m_OutputLink ;
	delete m_InputLink ;
}

Connection* WorkingMemory::GetConnection() const
{
	return GetAgent()->GetConnection() ;
}

char const* WorkingMemory::GetAgentName() const
{
	return GetAgent()->GetName() ;
}

// Searches for an identifier object that matches this id.
Identifier*	WorkingMemory::FindIdentifier(char const* pID, bool searchInput, bool searchOutput)
{
	// BADBAD: For better speed we could keep a map of identifiers in use and just look this up.
	Identifier* pMatch = NULL ;

	if (searchInput)
	{
		if (m_InputLink)
			pMatch = m_InputLink->FindIdentifier(pID) ;
	}

	if (searchOutput && !pMatch)
	{
		if (m_OutputLink)
			pMatch = m_OutputLink->FindIdentifier(pID) ;
	}

	return pMatch ;
}

// Create a new WME of the appropriate type based on this information.
WMElement* WorkingMemory::CreateWME(Identifier* pParent, char const* pAttribute, char const* pValue, char const* pType, long timeTag)
{
	// Value is an identifier
	if (strcmp(pType, sml_Names::kTypeID) == 0)
		return new Identifier(GetAgent(), pParent, pAttribute, pValue, timeTag) ;

	// Value is a string
	if (strcmp(pType, sml_Names::kTypeString) == 0)
		return new StringElement(GetAgent(), pParent, pAttribute, pValue, timeTag) ;

	// Value is an int
	if (strcmp(pType, sml_Names::kTypeInt) == 0)
	{
		int value = atoi(pValue) ;
		return new IntElement(GetAgent(), pParent, pAttribute, value, timeTag) ;
	}

	// Value is a float
	if (strcmp(pType, sml_Names::kTypeDouble) == 0)
	{
		double value = atof(pValue) ;
		return new FloatElement(GetAgent(), pParent, pAttribute, value, timeTag) ;
	}

	return NULL ;
}

void WorkingMemory::RecordAddition(WMElement* pWME)
{
	// For additions, the delta list does not take ownership of the wme.
	m_OutputDeltaList.AddWME(pWME) ;

	// Mark this wme as having just been added (in case the client would prefer
	// to walk the tree).
	pWME->SetJustAdded(true) ;
}

void WorkingMemory::RecordDeletion(WMElement* pWME)
{
	// This list takes ownership of the deleted wme.
	// When the item is removed from the delta list it will be deleted.
	m_OutputDeltaList.RemoveWME(pWME) ;
}

/*************************************************************
* @brief This function is called when output is received
*		 from the Soar kernel.
*
* @param pIncoming	The output command (list of wmes added/removed from output link)
* @param pResponse	The reply (no real need to fill anything in here currently)
*************************************************************/
bool WorkingMemory::ReceivedOutput(AnalyzeXML* pIncoming, ElementXML* pResponse)
{
#ifdef _DEBUG
	char * pMsg = pIncoming->GetCommandTag()->GenerateXMLString(true) ;
	ElementXML::DeleteString(pMsg) ;
#endif

	// Get the command tag which contains the list of wmes
	ElementXML const* pCommand = pIncoming->GetCommandTag() ;

	int nChildren = pCommand->GetNumberChildren() ;

	ElementXML wmeXML(NULL) ;
	ElementXML* pWmeXML = &wmeXML ;

	bool ok = true ;

	for (int i = 0 ; i < nChildren ; i++)
	{
		pCommand->GetChild(&wmeXML, i) ;

		// Ignore tags that aren't wmes.
		if (!pWmeXML->IsTag(sml_Names::kTagWME))
			continue ;

		// Find out if this is an add or a remove
		char const* pAction = pWmeXML->GetAttribute(sml_Names::kWME_Action) ;

		if (!pAction)
			continue ;

		bool add = IsStringEqual(pAction, sml_Names::kValueAdd) ;
		bool remove = IsStringEqual(pAction, sml_Names::kValueRemove) ;

		if (add)
		{
			// We're adding structure to the output link
			char const* pID			= pWmeXML->GetAttribute(sml_Names::kWME_Id) ;	// These IDs will be kernel side ids (e.g. "I3" not "i3")
			char const* pAttribute  = pWmeXML->GetAttribute(sml_Names::kWME_Attribute) ;
			char const* pValue		= pWmeXML->GetAttribute(sml_Names::kWME_Value) ;
			char const* pType		= pWmeXML->GetAttribute(sml_Names::kWME_ValueType) ;	// Can be NULL (=> string)
			char const* pTimeTag	= pWmeXML->GetAttribute(sml_Names::kWME_TimeTag) ;	// These will be kernel side time tags (e.g. +5 not -7)

			// Set the default value
			if (!pType)
				pType = sml_Names::kTypeString ;

			// Check we got everything we need
			if (!pID || !pAttribute || !pValue || !pTimeTag)
				continue ;

			long timeTag = atoi(pTimeTag) ;

			// Find the parent wme that we're adding this new wme to
			Identifier* pParent = FindIdentifier(pID, false, true) ;

			if (pParent)
			{
				// Create a client side wme object to match the output wme and add it to
				// our tree of objects.
				WMElement* pAddWme = CreateWME(pParent, pAttribute, pValue, pType, timeTag) ;
				if (pAddWme)
				{
					pParent->AddChild(pAddWme) ;

					// Make a record that this wme was added so we can alert the client to this change.
					RecordAddition(pAddWme) ;
				}
			}
			else
			{
				// We should only fail to find the parent when we're adding the output link itself.
				if (strcmpi(pAttribute, sml_Names::kOutputLinkName) == 0)
				{
					m_OutputLink = new Identifier(GetAgent(), pValue, timeTag) ;
				}
			}
		}
		else if (remove)
		{
			// We're removing structure from the output link
			char const* pTimeTag = pWmeXML->GetAttribute(sml_Names::kWME_TimeTag) ;	// These will usually be kernel side time tags (e.g. +5 not -7)

			long timeTag = atoi(pTimeTag) ;

			// If we have no output link we can't delete things from it.
			if (!m_OutputLink)
				continue ;

			// Find the WME which matches this tag.
			// This may fail as we may have removed the parent of this WME already in the series of remove commands.
			WMElement* pWME = m_OutputLink->FindTimeTag(timeTag) ;

			// Delete the WME
			if (pWME && pWME->GetParent())
			{
				pWME->GetParent()->RemoveChild(pWME) ;

				// Make a record that this wme was removed, so we can tell the client about it.
				// This recording will also involve deleting the wme.
				RecordDeletion(pWME) ;
			}
		}
	}

	// Returns false if any of the adds/removes fails
	return ok ;
}

/*************************************************************
* @brief Returns the id object for the input link.
*		 The agent retains ownership of this object.
*************************************************************/
Identifier* WorkingMemory::GetInputLink()
{
	if (!m_InputLink)
	{
		AnalyzeXML response ;

		if (GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_GetInputLink, GetAgentName()))
		{
			m_InputLink = new Identifier(GetAgent(), response.GetResultString(), GenerateTimeTag()) ;
		}
	}

	return m_InputLink ;
}

/*************************************************************
* @brief Returns the id object for the output link.
*		 The agent retains ownership of this object.
*
* The value will be NULL until the first output phase has executed
* in the Soar kernel and we've received the information from the kernel.
*************************************************************/
Identifier* WorkingMemory::GetOutputLink()
{
	return m_OutputLink ;
}

/*************************************************************
* @brief Builds a new WME that has a string value and schedules
*		 it for addition to Soar's input link.
*
*		 The agent retains ownership of this object.
*		 The returned object is valid until the caller
*		 deletes the parent identifier.
*		 The WME is not added to Soar's input link until the
*		 client calls "Commit".
*************************************************************/
StringElement* WorkingMemory::CreateStringWME(Identifier* parent, char const* pAttribute, char const* pValue)
{
	StringElement* pWME = new StringElement(GetAgent(), parent, pAttribute, pValue, GenerateTimeTag()) ;

	// Record that the identifer owns this new WME
	parent->AddChild(pWME) ;

	// Add it to our list of changes that need to be sent to Soar.
	m_DeltaList.AddWME(pWME) ;

	return pWME ;
}

/*************************************************************
* @brief Same as CreateStringWME but for a new WME that has
*		 an int as its value.
*************************************************************/
IntElement* WorkingMemory::CreateIntWME(Identifier* parent, char const* pAttribute, int value)
{
	IntElement* pWME = new IntElement(GetAgent(), parent, pAttribute, value, GenerateTimeTag()) ;

	// Record that the identifer owns this new WME
	parent->AddChild(pWME) ;

	// Add it to our list of changes that need to be sent to Soar.
	m_DeltaList.AddWME(pWME) ;

	return pWME ;
}

/*************************************************************
* @brief Same as CreateStringWME but for a new WME that has
*		 a floating point value.
*************************************************************/
FloatElement* WorkingMemory::CreateFloatWME(Identifier* parent, char const* pAttribute, double value)
{
	FloatElement* pWME = new FloatElement(GetAgent(), parent, pAttribute, value, GenerateTimeTag()) ;

	// Record that the identifer owns this new WME
	parent->AddChild(pWME) ;

	// Add it to our list of changes that need to be sent to Soar.
	m_DeltaList.AddWME(pWME) ;

	return pWME ;
}

/*************************************************************
* @brief Update the value of an existing WME.
*		 The value is not actually sent to the kernel
*		 until "Commit" is called.
*************************************************************/
void WorkingMemory::UpdateString(StringElement* pWME, char const* pValue)
{
	if (!pWME || !pValue)
		return ;

	// If the value hasn't changed, don't do anything
	if (strcmp(pValue, pWME->GetValueAsString()) == 0)
		return ;

	// Changing the value logically is a remove and then an add

	// Get the tag of the value to remove
	long removeTimeTag = pWME->GetTimeTag() ;

	// Change the value and the time tag (this is equivalent to us deleting the old object
	// and then creating a new one).
	pWME->SetValue(pValue) ;
	pWME->GenerateNewTimeTag() ;

	// Add it to the list of changes that need to be sent to Soar.
	m_DeltaList.UpdateWME(removeTimeTag, pWME) ;
}

void WorkingMemory::UpdateInt(IntElement* pWME, int value)
{
	if (!pWME)
		return ;

	// If the value hasn't changed, don't do anything
	if (pWME->GetValue() == value)
		return ;

	// Changing the value logically is a remove and then an add

	// Get the tag of the value to remove
	long removeTimeTag = pWME->GetTimeTag() ;

	// Change the value and the time tag (this is equivalent to us deleting the old object
	// and then creating a new one).
	pWME->SetValue(value) ;
	pWME->GenerateNewTimeTag() ;

	// Add it to the list of changes that need to be sent to Soar.
	m_DeltaList.UpdateWME(removeTimeTag, pWME) ;
}

void WorkingMemory::UpdateFloat(FloatElement* pWME, double value)
{
	if (!pWME)
		return ;

	// If the value hasn't changed, don't do anything
	if (pWME->GetValue() == value)
		return ;

	// Changing the value logically is a remove and then an add

	// Get the tag of the value to remove
	long removeTimeTag = pWME->GetTimeTag() ;

	// Change the value and the time tag (this is equivalent to us deleting the old object
	// and then creating a new one).
	pWME->SetValue(value) ;
	pWME->GenerateNewTimeTag() ;

	// Add it to the list of changes that need to be sent to Soar.
	m_DeltaList.UpdateWME(removeTimeTag, pWME) ;
}

/*************************************************************
* @brief Create a new ID for use by the client.
*		 The kernel will assign its own ids when the WME
*		 is really added, so it needs to know to map back and forth.
*		 To make that mapping clear, we generate IDs that use
*		 a lower case first letter, ensuring they aren't mistaken
*		 for IDs the kernel creates (although this should just help
*		 humans keep track of what's going on--shouldn't matter to
*		 the system).
*************************************************************/
void WorkingMemory::GenerateNewID(char const* pAttribute, std::string* pID)
{
	int id = GetAgent()->GetKernel()->GenerateNextID() ;

	char buffer[kMinBufferSize] ;
	Int2String(id, buffer, sizeof(buffer)) ;

	// we'll start our ids with lower case so we can distinguish them
	// from soar id's.  We'll take the first letter of the attribute,
	// much as soar does, but always add a unique number to the back,
	// so the choice of initial letter really isn't important.
	char letter = pAttribute[0] ;

	// Convert to lowercase
	if (letter >= 'A' || letter <= 'Z')
		letter = letter - 'A' + 'a' ;

	// Make sure we got a letter here (just in case)
	if (letter < 'a' || letter > 'z')
		letter = 'a' ;

	// Return the result
	*pID = letter ;
	pID->append(buffer) ;
}

/*************************************************************
* @brief Same as CreateStringWME but for a new WME that has
*		 an identifier as its value.
*************************************************************/
Identifier* WorkingMemory::CreateIdWME(Identifier* parent, char const* pAttribute)
{
	// Create a new, unique id (e.g. "i3").  This id will be mapped to a different id
	// in the kernel.
	std::string id ;
	GenerateNewID(pAttribute, &id) ;

	Identifier* pWME = new Identifier(GetAgent(), parent, pAttribute, id.c_str(), GenerateTimeTag()) ;

	// Record that the identifer owns this new WME
	parent->AddChild(pWME) ;

	// Add it to our list of changes that need to be sent to Soar.
	m_DeltaList.AddWME(pWME) ;

	return pWME ;
}

/*************************************************************
* @brief Schedules a WME from deletion from the input link and removes
*		 it from the client's model of working memory.
*
*		 The caller should not access this WME after calling
*		 DestroyWME().
*		 The WME is not removed from the input link until
*		 the client calls "Commit"
*************************************************************/
bool WorkingMemory::DestroyWME(WMElement* pWME)
{
	Identifier* parent = pWME->GetIdentifier() ;

	// We can't delete top level WMEs (e.g. the WME that represents the input-link's ID)
	// Those are architecturally created.
	if (parent == NULL)
		return false ;

	// The parent identifier no longer owns this WME
	parent->RemoveChild(pWME) ;

	// Add it to the list of changes to send to Soar.
	m_DeltaList.RemoveWME(pWME->GetTimeTag()) ;

	// Now we can delete it
	delete pWME ;

	return true ;
}

/*************************************************************
* @brief Generate a unique integer id (a time tag)
*************************************************************/
long WorkingMemory::GenerateTimeTag()
{
	// We use negative tags on the client, so we don't mistake them
	// for ones from the real kernel.
	int tag = GetAgent()->GetKernel()->GenerateNextTimeTag() ;

	return tag ;
}

/*************************************************************
* @brief Send the most recent list of changes to working memory
*		 over to the kernel.
*************************************************************/
bool WorkingMemory::Commit()
{
	int deltas = m_DeltaList.GetSize() ;

	// If nothing has changed, we have no work to do.
	if (deltas == 0)
		return true ;

	// Build the SML message we're doing to send.
	ElementXML* pMsg = GetConnection()->CreateSMLCommand(sml_Names::kCommand_Input) ;

	// Add the agent parameter and as a side-effect, get a pointer to the <command> tag.  This is an optimization.
	ElementXML_Handle hCommand = GetConnection()->AddParameterToSMLCommand(pMsg, sml_Names::kParamAgent, GetAgentName()) ;
	ElementXML command(hCommand) ;

	// Build the list of WME changes
	for (int i = 0 ; i < deltas ; i++)
	{
		// Get the next change
		TagWme* pDelta = m_DeltaList.GetDelta(i) ;

		// Add it as a child of the command tag
		// (the command takes ownership of the delta)
		command.AddChild(pDelta) ;
	}

	// This is important.  We are working with a subpart of pMsg.
	// If we retain ownership of the handle and delete the object
	// it will release the handle...deleting part of our message.
	command.Detach() ;

	// We have transfered the list of deltas over to pMsg
	// so we need to clear the list, but not delete the tags that it contains.
	m_DeltaList.Clear(false) ;

#ifdef _DEBUG
	// Generate a text form of the XML so we can look at it in the debugger.
	char* pStr = pMsg->GenerateXMLString(true) ;
	pMsg->DeleteString(pStr) ;
#endif

	// Send the message
	AnalyzeXML response ;
	bool ok = GetConnection()->SendMessageGetResponse(&response, pMsg) ;

	// Clean up
	delete pMsg ;

	return ok ;
}
