#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
//FIXME: #include <portability.h>

/////////////////////////////////////////////////////////////////
// InputListener class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : May 2007
//
// This class's OnKernelEvent method is called when
// the agent's input phase callback is called.
//
/////////////////////////////////////////////////////////////////

#include "sml_InputListener.h"
#include "sml_Connection.h"
#include "sml_AgentSML.h"
#include "sml_KernelSML.h"
#include "KernelHeaders.h"
#include "sock_Check.h"
#include "sock_Debug.h"	// For PrintDebugFormat

using namespace sml ;

// Flag to control printing debug information about the input link
#ifdef _DEBUG
	static bool kDebugInput = false ;
#else
	static bool kDebugInput = false ;
#endif

// A set of helper functions for tracing kernel wmes
static void Symbol2String(Symbol* pSymbol, 	bool refCounts, std::ostringstream& buffer) {
	if (pSymbol->common.symbol_type==IDENTIFIER_SYMBOL_TYPE) {
		buffer << pSymbol->id.name_letter ;
		buffer << pSymbol->id.name_number ;
	}
	else if (pSymbol->common.symbol_type==VARIABLE_SYMBOL_TYPE) {
		buffer << pSymbol->var.name ;
	}
	else if (pSymbol->common.symbol_type==SYM_CONSTANT_SYMBOL_TYPE) {
		buffer << pSymbol->sc.name ;
	}
	else if (pSymbol->common.symbol_type==INT_CONSTANT_SYMBOL_TYPE) {
		buffer << pSymbol->ic.value ;
	}
	else if (pSymbol->common.symbol_type==FLOAT_CONSTANT_SYMBOL_TYPE) {
		buffer << pSymbol->fc.value ;
	}

	if (refCounts)
		buffer << "[" << pSymbol->common.reference_count << "]" ;
}

static std::string Wme2String(wme* pWME, bool refCounts) {
	std::ostringstream buffer ;

	buffer << pWME->timetag << ":" ;

	Symbol2String(pWME->id, refCounts, buffer) ;
	buffer << " ^" ;
	Symbol2String(pWME->attr, refCounts, buffer) ;
	buffer << " " ;
	Symbol2String(pWME->value, refCounts, buffer) ;

	return buffer.str() ;
}

static void PrintDebugWme(char const* pMsg, wme* pWME, bool refCounts = false) {
	std::string str = Wme2String(pWME, refCounts) ;
	PrintDebugFormat("%s %s", pMsg, str.c_str()) ;
}

static void PrintDebugSymbol(Symbol* pSymbol, bool refCounts = false) {
	std::ostringstream buffer ;
	Symbol2String(pSymbol, refCounts, buffer) ;
	std::string str = buffer.str() ;

	PrintDebugFormat("%s", str.c_str()) ;
}

void InputListener::Init(KernelSML* pKernelSML, AgentSML* pAgentSML)
{
	m_KernelSML = pKernelSML ;
	SetAgentSML(pAgentSML) ;
}

// Called when an event occurs in the kernel
void InputListener::OnKernelEvent(int eventID, AgentSML* pAgentSML, void* pCallData)
{
    int callbacktype = (int)reinterpret_cast<long long>(pCallData);

    switch(callbacktype) {
    case TOP_STATE_JUST_CREATED:
	  ProcessPendingInput(pAgentSML, callbacktype) ;
      break;
    case NORMAL_INPUT_CYCLE:
	  ProcessPendingInput(pAgentSML, callbacktype) ;
      break;
    case TOP_STATE_JUST_REMOVED:
      break;
	}
}

void InputListener::ProcessPendingInput(AgentSML* pAgentSML, int callbacktype)
{
	PendingInputList* pPending = pAgentSML->GetPendingInputList() ;

	bool ok = true ;

	for (PendingInputListIter iter = pPending->begin() ; iter != pPending->end() ; iter++) {
		ElementXML* pInputMsg = *iter ;

		// Analyze the message and find important tags
		AnalyzeXML msg ;
		msg.Analyze(pInputMsg) ;

		// Get the "name" attribute from the <command> tag
		char const* pCommandName = msg.GetCommandName() ;

		// Only input commands should be stored in the pending input list
		assert(!strcmp(pCommandName, "input")) ;

		ElementXML const* pCommand = msg.GetCommandTag() ;

		int nChildren = pCommand->GetNumberChildren() ;

		ElementXML wmeXML(NULL) ;
		ElementXML* pWmeXML = &wmeXML ;

		if (kDebugInput)
			PrintDebugFormat("--------- %s starting input ----------", pAgentSML->GetName()) ;

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
				char const* pID			= pWmeXML->GetAttribute(sml_Names::kWME_Id) ;	// May be a client side id value (e.g. "o3" not "O3")
				char const* pAttribute  = pWmeXML->GetAttribute(sml_Names::kWME_Attribute) ;
				char const* pValue		= pWmeXML->GetAttribute(sml_Names::kWME_Value) ;
				char const* pType		= pWmeXML->GetAttribute(sml_Names::kWME_ValueType) ;	// Can be NULL (=> string)
				char const* pTimeTag	= pWmeXML->GetAttribute(sml_Names::kWME_TimeTag) ;	// May be a client side time tag (e.g. -3 not +3)

				// Set the default value
				if (!pType)
					pType = sml_Names::kTypeString ;

				// Check we got everything we need
				if (!pID || !pAttribute || !pValue || !pTimeTag)
					continue ;

				// Map the ID from client side to kernel side (if the id is already a kernel side id it's returned unchanged)
				std::string id ;
				pAgentSML->ConvertID(pID, &id) ;

				if (kDebugInput)
				{
					PrintDebugFormat("%s Add %s ^%s %s (type %s tag %s)", pAgentSML->GetName(), id.c_str(), pAttribute, pValue, pType, pTimeTag) ;
				}

				// Add the wme
				ok = AddInputWME(pAgentSML, id.c_str(), pAttribute, pValue, pType, pTimeTag) && ok ;
			}
			else if (remove)
			{
				char const* pTimeTag = pWmeXML->GetAttribute(sml_Names::kWME_TimeTag) ;	// May be (will be?) a client side time tag (e.g. -3 not +3)

				if (kDebugInput)
				{
					PrintDebugFormat("%s Remove tag %s", pAgentSML->GetName(), pTimeTag) ;
				}

				// Remove the wme
				ok = RemoveInputWME(pAgentSML, pTimeTag) && ok ;
			}
		}

		delete pInputMsg ;
	}

	pPending->clear() ;
}

bool InputListener::AddInputWME(AgentSML* pAgentSML, char const* pID, char const* pAttribute, char const* pValue, char const* pType, char const* pTimeTag)
{
	Symbol* pValueSymbol = 0 ;
	agent*  pAgent = pAgentSML->GetAgent() ;

	CHECK_RET_FALSE(strlen(pID) >= 2) ;
	char idLetter = pID[0] ;
	int idNumber = atoi(&pID[1]) ;

	if (IsStringEqual(sml_Names::kTypeString, pType)) {
		// Creating a wme with a string constant value
		pValueSymbol = get_io_sym_constant(pAgent, pValue) ;
	} else if (IsStringEqual(sml_Names::kTypeInt, pType)) {
		// Creating a WME with an int value
		int value = atoi(pValue) ;
		pValueSymbol = get_io_int_constant(pAgent, value) ;
	} else if (IsStringEqual(sml_Names::kTypeDouble, pType)) {
		// Creating a WME with a float value
		float value = (float)atof(pValue) ;
		pValueSymbol = get_io_float_constant(pAgent, value) ;
	} else if (IsStringEqual(sml_Names::kTypeID, pType)) {
		// Creating a WME with an identifier value

		CHECK_RET_FALSE(strlen(pValue)) ;		// Identifier must be format Xdddd for some letter "X" and at least one digit "d"
		char letter = pValue[0]	;				// Kernel identifiers always use upper case identifier letters

		bool haveKernelID = true ;
		std::string value = pValue ;
		if (letter >= 'a' || letter <= 'z')
		{
			// Convert the value (itself an identifier) from client to kernel
			bool found = pAgentSML->ConvertID(pValue, &value) ;

			if (!found)
				haveKernelID = false ;
		}

		// Find the identifier number
		char const* pNumber = value.c_str() ;
		pNumber++ ;
		int number = atoi(pNumber) ;

		// If we reach here without having a mapping to a kernel side identifier
		// then we want to force the kernel to create a new id for us.
		// We'll do that by setting the number to 0 -- which will never match an existing
		// identifier.
		if (!haveKernelID)
		{
			letter = (char)toupper(letter) ;
			number = 0 ;
		}

		// See if this identifier already exists
		pValueSymbol = get_io_identifier(pAgent, letter, number) ;

		// If we just created this symbol record it in our mapping table
		if (pValueSymbol->common.reference_count == 1)
		{
			// We need to record the id that the kernel assigned to this object and match it against the id the
			// client is using, so that in future we can map the client's id to the kernel's.
			std::ostringstream buffer;
			buffer << pValueSymbol->id.name_letter ;
			buffer << pValueSymbol->id.name_number ;
			std::string newid = buffer.str() ;
			pAgentSML->RecordIDMapping(pValue, newid.c_str()) ;

			if (kDebugInput)
			{
				PrintDebugFormat("Recording id mapping of %s to %s", pValue, newid.c_str()) ;
			}
		}
	}

	// Now create the wme

	Symbol* pIDSymbol   = get_io_identifier(pAgent, idLetter, idNumber) ;
	Symbol* pAttrSymbol = get_io_sym_constant(pAgent, pAttribute) ;

	CHECK_RET_FALSE(pIDSymbol) ;
	CHECK_RET_FALSE(pAttrSymbol) ;

	wme* pNewInputWme = add_input_wme(pAgent, pIDSymbol, pAttrSymbol, pValueSymbol) ;
	long timeTag = pNewInputWme->timetag ;

	CHECK_RET_FALSE(pNewInputWme) ;

	if (kDebugInput)
		PrintDebugWme("Adding wme ", pNewInputWme, true) ;

	// The kernel doesn't support direct lookup of wme from timetag so we'll
	// store these values in a hashtable.  Perhaps later we'll see about adding
	// this support directly into the kernel.
	pAgentSML->RecordTime(pTimeTag, timeTag) ;
//	pAgentSML->RecordKernelTimeTag(pTimeTag, pNewInputWme) ;

	unsigned long refCount1 = release_io_symbol(pAgentSML->GetAgent(), pNewInputWme->id) ;
	unsigned long refCount2 = release_io_symbol(pAgentSML->GetAgent(), pNewInputWme->attr) ;
	unsigned long refCount3 = release_io_symbol(pAgentSML->GetAgent(), pNewInputWme->value) ;

	if (kDebugInput)
		PrintDebugWme("Adding wme ", pNewInputWme, true) ;

	return true ;
}

bool InputListener::RemoveInputWME(AgentSML* pAgentSML, char const* pTimeTag)
{
	// Get the wme that matches this time tag
	long timetag = pAgentSML->ConvertTime(pTimeTag) ;
	wme* pWME = find_input_wme_by_timetag(pAgentSML->GetAgent(), timetag) ;

//	wme* pWME = pAgentSML->ConvertKernelTimeTag(pTimeTag) ;

	std::string printInput1 = pAgentSML->ExecuteCommandLine("print --internal --depth 2 I2") ;

	if (kDebugInput)
		PrintDebugFormat("%s\nLooking for %ld", printInput1.c_str(), timetag) ;

	// The wme is already gone so no work to do
	if (!pWME)
	{
		return false ;
	}

//	wme* pWME = pAgentSML->ConvertKernelTimeTag(pTimeTag) ;

	if (kDebugInput)
		PrintDebugWme("Removing input wme ", pWME, true) ;

	CHECK_RET_FALSE(pWME) ;

	if (pWME->value->common.symbol_type==IDENTIFIER_SYMBOL_TYPE) {
		std::ostringstream buffer;
		buffer << pWME->value->id.name_letter ;
		buffer << pWME->value->id.name_number ;
		std::string newid = buffer.str() ;

		// This extra release of the identifier value seems to be required
		// but I don't understand why.
		Symbol* pIDSymbol = pWME->value ;
//		release_io_symbol(pAgentSML->GetAgent(), pIDSymbol) ;
//		release_io_symbol(pAgentSML->GetAgent(), pWME->id) ;
//		release_io_symbol(pAgentSML->GetAgent(), pWME->attr) ;

		// Remove this id from the id mapping table
		// BUGBUG: This seems to be assuming that there's only a single use of this ID in the kernel, but what if it's shared?
		// I think we might not want to do this and just leave the mapping forever once it has been used.  If the client
		// re-uses this value we'll continue to map it.
		//pAgentSML->RemoveID(id.c_str()) ;
	}

	Bool ok = remove_input_wme(pAgentSML->GetAgent(), pWME) ;

	/*
	unsigned long refCount1 = release_io_symbol(pAgentSML->GetAgent(), pWME->id) ;
	unsigned long refCount2 = release_io_symbol(pAgentSML->GetAgent(), pWME->attr) ;
	unsigned long refCount3 = release_io_symbol(pAgentSML->GetAgent(), pWME->value) ;
	*/

	// Remove the object from the time tag table because
	// we no longer own it.
	pAgentSML->RemoveKernelTimeTag(pTimeTag) ;

	CHECK_RET_FALSE(ok) ;

	return (ok != 0) ;
}

// Register for the events that KernelSML itself needs to know about in order to work correctly.
void InputListener::RegisterForKernelSMLEvents()
{
	// Listen for input callback events so we can submit pending input to the kernel
	this->RegisterWithKernel(smlEVENT_INPUT_PHASE_CALLBACK) ;
}

void InputListener::UnRegisterForKernelSMLEvents()
{
	this->UnregisterWithKernel(smlEVENT_INPUT_PHASE_CALLBACK) ;
}

