#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/////////////////////////////////////////////////////////////////
// Agent class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : Sept 2004
//
// This class is used by a client app (e.g. an environment) to represent
// a Soar agent and to send commands and I/O to and from that agent.
//
/////////////////////////////////////////////////////////////////

#include "sml_ClientAgent.h"
#include "sml_ClientKernel.h"
#include "sml_Connection.h"
#include "sml_ClientIdentifier.h"
#include "sml_OutputDeltaList.h"
#include "sml_StringOps.h"
#include "sml_Events.h"
#include "sml_ClientXML.h"
#include "sml_ClientTraceXML.h"

#include "sml_ClientDirect.h"
#include "sml_EmbeddedConnection.h"	// For access to direct methods

#include <iostream>     
#include <sstream>     
#include <iomanip>

#include <cassert>
#include <string>

using namespace sml;

Agent::Agent(Kernel* pKernel, char const* pName)
{
	m_Kernel = pKernel ;
	m_Name	 = pName ;
	m_WorkingMemory.SetAgent(this) ;
	m_CallbackIDCounter = 0 ;
	m_XMLCallback = -1 ;
}

Agent::~Agent()
{
}

Connection* Agent::GetConnection() const
{
	return m_Kernel->GetConnection() ;
}

/*************************************************************
* @brief This function is called when output is received
*		 from the Soar kernel.
*
* @param pIncoming	The output command (list of wmes added/removed from output link)
* @param pResponse	The reply (no real need to fill anything in here currently)
*************************************************************/
void Agent::ReceivedOutput(AnalyzeXML* pIncoming, ElementXML* pResponse)
{
	GetWM()->ReceivedOutput(pIncoming, pResponse) ;
}

void Agent::ReceivedEvent(AnalyzeXML* pIncoming, ElementXML* pResponse)
{
	char const* pEventName = pIncoming->GetArgValue(sml_Names::kParamEventID) ;

	// This event had no event id field
	if (!pEventName)
	{
		return ;
	}

	// Go from the string form of the event back to the integer ID
	int id = GetKernel()->m_pEventMap->ConvertToEvent(pEventName) ;

	if (IsRunEventID(id))
	{
		ReceivedRunEvent((smlRunEventId)id, pIncoming, pResponse) ;
	} else if (IsProductionEventID(id))
	{
		ReceivedProductionEvent((smlProductionEventId)id, pIncoming, pResponse) ;
	} else if (IsPrintEventID(id))
	{
		ReceivedPrintEvent((smlPrintEventId)id, pIncoming, pResponse) ;
	}
}

/*************************************************************
* @brief This function is called when an event is received
*		 from the Soar kernel.
*
* @param pIncoming	The event command
* @param pResponse	The reply (no real need to fill anything in here currently)
*************************************************************/
void Agent::ReceivedRunEvent(smlRunEventId id, AnalyzeXML* pIncoming, ElementXML* pResponse)
{
	unused(pResponse) ;

	smlPhase phase = (smlPhase)pIncoming->GetArgInt(sml_Names::kParamPhase, -1) ;

	// Look up the handler(s) from the map
	RunEventMap::ValueList* pHandlers = m_RunEventMap.getList(id) ;

	if (!pHandlers)
		return ;

	// Go through the list of event handlers calling each in turn
	for (RunEventMap::ValueListIter iter = pHandlers->begin() ; iter != pHandlers->end() ; iter++)
	{
		RunEventHandlerPlusData handlerWithData = *iter ;

		RunEventHandler handler = handlerWithData.m_Handler ;
		void* pUserData = handlerWithData.m_UserData ;

		// Call the handler
		handler(id, pUserData, this, phase) ;
	}
}

/*************************************************************
* @brief This function is called when an event is received
*		 from the Soar kernel.
*
* @param pIncoming	The event command
* @param pResponse	The reply (no real need to fill anything in here currently)
*************************************************************/
void Agent::ReceivedPrintEvent(smlPrintEventId id, AnalyzeXML* pIncoming, ElementXML* pResponse)
{
	unused(pResponse) ;

	char const* pMessage = pIncoming->GetArgValue(sml_Names::kParamMessage) ;

	// Look up the handler(s) from the map
	PrintEventMap::ValueList* pHandlers = m_PrintEventMap.getList(id) ;

	if (!pHandlers)
		return ;

	// Go through the list of event handlers calling each in turn
	for (PrintEventMap::ValueListIter iter = pHandlers->begin() ; iter != pHandlers->end() ; iter++)
	{
		PrintEventHandlerPlusData handlerPlus = *iter ;
		PrintEventHandler handler = handlerPlus.m_Handler ;

		void* pUserData = handlerPlus.m_UserData ;

		// Call the handler
		handler(id, pUserData, this, pMessage) ;
	}
}

/*************************************************************
* @brief This function is called when an event is received
*		 from the Soar kernel.
*
* @param pIncoming	The event command
* @param pResponse	The reply (no real need to fill anything in here currently)
*************************************************************/
void Agent::ReceivedProductionEvent(smlProductionEventId id, AnalyzeXML* pIncoming, ElementXML* pResponse)
{
	unused(pResponse) ;

	char const* pProductionName = pIncoming->GetArgValue(sml_Names::kParamName) ;
	char const* pInstance = 0 ;	// gSKI defines this but doesn't support it yet.

	// Look up the handler(s) from the map
	ProductionEventMap::ValueList* pHandlers = m_ProductionEventMap.getList(id) ;

	if (!pHandlers)
		return ;

	// Go through the list of event handlers calling each in turn
	for (ProductionEventMap::ValueListIter iter = pHandlers->begin() ; iter != pHandlers->end() ; iter++)
	{
		ProductionEventHandlerPlusData handlerPlus = *iter ;

		ProductionEventHandler handler = handlerPlus.m_Handler ;
		void* pUserData = handlerPlus.m_UserData ;

		// Call the handler
		handler(id, pUserData, this, pProductionName, pInstance) ;
	}
}

/*************************************************************
* @brief Load a set of productions from a file.
*
* The file must currently be on a filesystem that the kernel can
* access (i.e. can't send to a remote PC unless that PC can load
* this file).
*************************************************************/
bool Agent::LoadProductions(char const* pFilename)
{
	// gSKI's LoadProductions command doesn't support all of the command line commands we need,
	// so we'll send this through the command line processor instead by creating a "source" command.
	std::string cmd = "source ";
	if (strlen(pFilename) && (pFilename[0] == '\"') && (pFilename[strlen(pFilename) - 1] == '\"'))
	{
		cmd += pFilename ;
	} else {
		cmd += '\"';
		cmd += pFilename ;
		cmd += '\"';
	}

	// Execute the source command
	char const* pResult = ExecuteCommandLine(cmd.c_str()) ;

	bool ok = GetLastCommandLineResult() ;

	if (ok)
		ClearError() ;
	else
		SetDetailedError(Error::kDetailedError, pResult) ;

	return ok ;
}

// These are little utility classes we define to help with searching the event maps
class Agent::TestRunCallback : public RunEventMap::ValueTest
{
private:
	int m_ID ;
public:
	TestRunCallback(int id) { m_ID = id ; }

	bool isEqual(RunEventHandlerPlusData handler)
	{
		return handler.m_CallbackID == m_ID ;
	}
} ;

class Agent::TestRunCallbackFull : public RunEventMap::ValueTest
{
private:
	int				m_EventID ;
	RunEventHandler m_Handler ;
	void*			m_UserData ;

public:
	TestRunCallbackFull(int id, RunEventHandler handler, void* pUserData)
	{ m_EventID = id ; m_Handler = handler ; m_UserData = pUserData ; }

	bool isEqual(RunEventHandlerPlusData handlerPlus)
	{
		return handlerPlus.m_EventID == m_EventID &&
			   handlerPlus.m_Handler == m_Handler &&
			   handlerPlus.m_UserData == m_UserData ;
	}
} ;

class Agent::TestProductionCallback : public ProductionEventMap::ValueTest
{
private:
	int m_ID ;
public:
	TestProductionCallback(int id) { m_ID = id ; }

	bool isEqual(ProductionEventHandlerPlusData handler)
	{
		return handler.m_CallbackID == m_ID ;
	}
} ;

class Agent::TestProductionCallbackFull : public ProductionEventMap::ValueTest
{
private:
	int				m_EventID ;
	ProductionEventHandler m_Handler ;
	void*			m_UserData ;

public:
	TestProductionCallbackFull(int id, ProductionEventHandler handler, void* pUserData)
	{ m_EventID = id ; m_Handler = handler ; m_UserData = pUserData ; }

	bool isEqual(ProductionEventHandlerPlusData handlerPlus)
	{
		return handlerPlus.m_EventID == m_EventID &&
			   handlerPlus.m_Handler == m_Handler &&
			   handlerPlus.m_UserData == m_UserData ;
	}
} ;

class Agent::TestPrintCallback : public PrintEventMap::ValueTest
{
private:
	int m_ID ;
public:
	TestPrintCallback(int id) { m_ID = id ; }

	bool isEqual(PrintEventHandlerPlusData handler)
	{
		return handler.m_CallbackID == m_ID ;
	}
} ;

class Agent::TestPrintCallbackFull : public PrintEventMap::ValueTest
{
private:
	int				m_EventID ;
	PrintEventHandler m_Handler ;
	void*			m_UserData ;

public:
	TestPrintCallbackFull(int id, PrintEventHandler handler, void* pUserData)
	{ m_EventID = id ; m_Handler = handler ; m_UserData = pUserData ; }

	bool isEqual(PrintEventHandlerPlusData handlerPlus)
	{
		return handlerPlus.m_EventID == m_EventID &&
			   handlerPlus.m_Handler == m_Handler &&
			   handlerPlus.m_UserData == m_UserData ;
	}
} ;

class Agent::TestXMLCallback : public XMLEventMap::ValueTest
{
private:
	int m_ID ;
public:
	TestXMLCallback(int id) { m_ID = id ; }

	bool isEqual(XMLEventHandlerPlusData handler)
	{
		return handler.m_CallbackID == m_ID ;
	}
} ;

class Agent::TestXMLCallbackFull : public XMLEventMap::ValueTest
{
private:
	int				m_EventID ;
	XMLEventHandler m_Handler ;
	void*			m_UserData ;

public:
	TestXMLCallbackFull(int id, XMLEventHandler handler, void* pUserData)
	{ m_EventID = id ; m_Handler = handler ; m_UserData = pUserData ; }

	bool isEqual(XMLEventHandlerPlusData handlerPlus)
	{
		return handlerPlus.m_EventID == m_EventID &&
			   handlerPlus.m_Handler == m_Handler &&
			   handlerPlus.m_UserData == m_UserData ;
	}
} ;

/*************************************************************
* @brief Register for a "RunEvent".
*		 Multiple handlers can be registered for the same event.
* @param smlEventId		The event we're interested in (see the list below for valid values)
* @param handler		A function that will be called when the event happens
* @param pUserData		Arbitrary data that will be passed back to the handler function when the event happens.
* @param addToBack		If true add this handler is called after existing handlers.  If false, called before existing handlers.
* 
* Current set is:
* smlEVENT_BEFORE_SMALLEST_STEP,
* smlEVENT_AFTER_SMALLEST_STEP,
* smlEVENT_BEFORE_ELABORATION_CYCLE,
* smlEVENT_AFTER_ELABORATION_CYCLE,
* smlEVENT_BEFORE_PHASE_EXECUTED,
* smlEVENT_AFTER_PHASE_EXECUTED,
* smlEVENT_BEFORE_DECISION_CYCLE,
* smlEVENT_AFTER_DECISION_CYCLE,
* smlEVENT_AFTER_INTERRUPT,
* smlEVENT_BEFORE_RUNNING,
* smlEVENT_AFTER_RUNNING,
*
* @returns A unique ID for this callback (used to unregister the callback later) 
*************************************************************/
int Agent::RegisterForRunEvent(smlRunEventId id, RunEventHandler handler, void* pUserData, bool addToBack)
{
	// Start by checking if this id, handler, pUSerData combination has already been registered
	TestRunCallbackFull test(id, handler, pUserData) ;

	// See if this handler is already registered
	RunEventHandlerPlusData plus(0,0,0,0) ;
	bool found = m_RunEventMap.findFirstValueByTest(&test, &plus) ;

	if (found && plus.m_Handler != 0)
		return plus.getCallbackID() ;

	// If we have no handlers registered with the kernel, then we need
	// to register for this event.  No need to do this multiple times.
	if (m_RunEventMap.getListSize(id) == 0)
	{
		GetKernel()->RegisterForEventWithKernel(id, GetAgentName()) ;
	}

	// Record the handler
	m_CallbackIDCounter++ ;

	// We use a struct rather than a pointer to a struct, so there's no need to new/delete
	// everything as the objects are added and deleted.
	RunEventHandlerPlusData handlerPlus(id, handler, pUserData, m_CallbackIDCounter) ;
	m_RunEventMap.add(id, handlerPlus, addToBack) ;

	// Return the ID.  We use this later to unregister the callback
	return m_CallbackIDCounter ;
}

/*************************************************************
* @brief Unregister for a particular event
*************************************************************/
bool Agent::UnregisterForRunEvent(int callbackID)
{
	// Build a test object for the callbackID we're interested in
	TestRunCallback test(callbackID) ;

	// Find the event ID for this callbackID
	smlRunEventId id = m_RunEventMap.findFirstKeyByTest(&test, (smlRunEventId)-1) ;

	if (id == -1)
		return false ;

	// Remove the handler from our map
	m_RunEventMap.removeAllByTest(&test) ;

	// If we just removed the last handler, then unregister from the kernel for this event
	if (m_RunEventMap.getListSize(id) == 0)
	{
		GetKernel()->UnregisterForEventWithKernel(id, GetAgentName()) ;
	}

	return true ;
}

/*************************************************************
* @brief Register for a "ProductionEvent".
*		 Multiple handlers can be registered for the same event.
* @param smlEventId		The event we're interested in (see the list below for valid values)
* @param handler		A function that will be called when the event happens
* @param pUserData		Arbitrary data that will be passed back to the handler function when the event happens.
* @param addToBack		If true add this handler is called after existing handlers.  If false, called before existing handlers.
*
* Current set is:
* Production Manager
* smlEVENT_AFTER_PRODUCTION_ADDED,
* smlEVENT_BEFORE_PRODUCTION_REMOVED,
* smlEVENT_AFTER_PRODUCTION_FIRED,
* smlEVENT_BEFORE_PRODUCTION_RETRACTED,
*
* @returns A unique ID for this callback (used to unregister the callback later) 
*************************************************************/
int Agent::RegisterForProductionEvent(smlProductionEventId id, ProductionEventHandler handler, void* pUserData, bool addToBack)
{
	// Start by checking if this id, handler, pUSerData combination has already been registered
	TestProductionCallbackFull test(id, handler, pUserData) ;

	// See if this handler is already registered
	ProductionEventHandlerPlusData plus(0,0,0,0) ;
	bool found = m_ProductionEventMap.findFirstValueByTest(&test, &plus) ;

	if (found && plus.m_Handler != 0)
		return plus.getCallbackID() ;

	// If we have no handlers registered with the kernel, then we need
	// to register for this event.  No need to do this multiple times.
	if (m_ProductionEventMap.getListSize(id) == 0)
	{
		GetKernel()->RegisterForEventWithKernel(id, GetAgentName()) ;
	}

	// Record the handler
	m_CallbackIDCounter++ ;
	ProductionEventHandlerPlusData handlerPlus(id, handler, pUserData, m_CallbackIDCounter) ;
	m_ProductionEventMap.add(id, handlerPlus, addToBack) ;

	// Return the ID.  We use this later to unregister the callback
	return m_CallbackIDCounter ;
}

/*************************************************************
* @brief Unregister for a particular event
*************************************************************/
bool Agent::UnregisterForProductionEvent(int callbackID)
{
	// Build a test object for the callbackID we're interested in
	TestProductionCallback test(callbackID) ;

	// Find the event ID for this callbackID
	smlProductionEventId id = m_ProductionEventMap.findFirstKeyByTest(&test, (smlProductionEventId)-1) ;

	if (id == -1)
		return false ;

	// Remove the handler from our map
	m_ProductionEventMap.removeAllByTest(&test) ;

	// If we just removed the last handler, then unregister from the kernel for this event
	if (m_ProductionEventMap.getListSize(id) == 0)
	{
		GetKernel()->UnregisterForEventWithKernel(id, GetAgentName()) ;
	}

	return true ;
}

/*************************************************************
* @brief Register for an "PrintEvent".
*		 Multiple handlers can be registered for the same event.
* @param smlEventId		The event we're interested in (see the list below for valid values)
* @param handler		A function that will be called when the event happens
* @param pUserData		Arbitrary data that will be passed back to the handler function when the event happens.
* @param addToBack		If true add this handler is called after existing handlers.  If false, called before existing handlers.
*
* Current set is:
* // Agent manager
* smlEVENT_PRINT
*
* @returns A unique ID for this callback (used to unregister the callback later) 
*************************************************************/
int Agent::RegisterForPrintEvent(smlPrintEventId id, PrintEventHandler handler, void* pUserData, bool addToBack)
{
	// Start by checking if this id, handler, pUSerData combination has already been registered
	TestPrintCallbackFull test(id, handler, pUserData) ;

	// See if this handler is already registered
	PrintEventHandlerPlusData plus(0,0,0,0) ;
	bool found = m_PrintEventMap.findFirstValueByTest(&test, &plus) ;

	if (found && plus.m_Handler != 0)
		return plus.getCallbackID() ;

	// If we have no handlers registered with the kernel, then we need
	// to register for this event.  No need to do this multiple times.
	if (m_PrintEventMap.getListSize(id) == 0)
	{
		GetKernel()->RegisterForEventWithKernel(id, GetAgentName()) ;
	}

	// Record the handler
	m_CallbackIDCounter++ ;

	PrintEventHandlerPlusData handlerPlus(id, handler, pUserData, m_CallbackIDCounter) ;
	m_PrintEventMap.add(id, handlerPlus, addToBack) ;

	// Return the ID.  We use this later to unregister the callback
	return m_CallbackIDCounter ;
}

/*************************************************************
* @brief Unregister for a particular event
*************************************************************/
bool Agent::UnregisterForPrintEvent(int callbackID)
{
	// Build a test object for the callbackID we're interested in
	TestPrintCallback test(callbackID) ;

	// Find the event ID for this callbackID
	smlPrintEventId id = m_PrintEventMap.findFirstKeyByTest(&test, (smlPrintEventId)-1) ;

	if (id == -1)
		return false ;

	// Remove the handler from our map
	m_PrintEventMap.removeAllByTest(&test) ;

	// If we just removed the last handler, then unregister from the kernel for this event
	if (m_PrintEventMap.getListSize(id) == 0)
	{
		GetKernel()->UnregisterForEventWithKernel(id, GetAgentName()) ;
	}

	return true ;
}

/** @brief This handler is called with structured output print data.  Used to implement XML events */
static void XMLEventImplementationHandler(smlPrintEventId id, void* pUserData, Agent* pAgent, char const* pMessage)
{
	unused(id);
	unused(pUserData) ;

	// Some basic error checking
	if (!pMessage || !pAgent)
		return ;

	// We only use this handler to convert from XML as a string to XML as an object
	assert(id == smlEVENT_STRUCTURED_OUTPUT) ;

	// We may have been passed a series of XML messages (objects), not one single object
	size_t startPos = 0 ;
	size_t endPos   = 0 ;
	size_t length = strlen(pMessage) ;

	// We get sent a series of discrete XML documents in pMessage.
	// We'll collect those into a single, macro XML object and pass that to the client
	// so that we get one update event with lots of data rather than many update events.
	// It allows the client to be more efficient.
	ElementXML* pTraceXML = new ElementXML() ;
	pTraceXML->SetTagName(sml_Names::kTagTrace) ;

	while (startPos < length)
	{
		ElementXML* pXML = ElementXML::ParseXMLFromStringSequence(pMessage, startPos, &endPos) ;

		// If there was an error we're done
		if (!pXML)
			break ;

		// Collect smaller pieces into one parent message.
		pTraceXML->AddChild(pXML) ;
		
		// Start the next parse from the end of this parse.
		startPos = endPos ;
	}

	// If we can't parse pMessage into anything valid, we're done.
	if (pTraceXML->GetNumberChildren() == 0)
	{
		delete pTraceXML ;
		return ;
	}

#ifdef _DEBUG
	// It's helpful to see the XML
	char* pStr = pTraceXML->GenerateXMLString(true) ;
#endif

	// Take message and call each handler in turn
	// NOTE: pTraceXML is deleted by SendXMLEvent when it's done.
	pAgent->SendXMLEvent(smlEVENT_XML_TRACE_OUTPUT, pTraceXML) ;

#ifdef _DEBUG
	ElementXML::DeleteString(pStr) ;
#endif
}

/*************************************************************
* @brief This function is called when an XML event needs to be
*		 sent out.  It's route is a little unusual as it's called
*		 from a Print event handler, not directly in response to
*		 an event from the kernel (as are other events).
*
* @param pXMLMessage	The XML message we should distribute to our listeners.
*						NOTE: We must delete this message when we're done.
*************************************************************/
void Agent::SendXMLEvent(smlXMLEventId id, ElementXML* pXMLMessage)
{
	// NOTE: This object needs to stay in scope for as long as we're calling clients
	// and then when it is deleted it will delete the pXMLMessage.
	ClientXML clientXML(pXMLMessage) ;

	// Look up the handler(s) from the map
	XMLEventMap::ValueList* pHandlers = m_XMLEventMap.getList(id) ;

	if (!pHandlers)
		return ;

	// Go through the list of event handlers calling each in turn
	for (XMLEventMap::ValueListIter iter = pHandlers->begin() ; iter != pHandlers->end() ; iter++)
	{
		XMLEventHandlerPlusData handlerPlus = *iter ;
		XMLEventHandler handler = handlerPlus.m_Handler ;

		void* pUserData = handlerPlus.m_UserData ;

		// Call the handler
		handler(id, pUserData, this, &clientXML) ;
	}
}

int Agent::RegisterForXMLEvent(smlXMLEventId id, XMLEventHandler handler, void* pUserData, bool addToBack)
{
	// Start by checking if this id, handler, pUSerData combination has already been registered
	TestXMLCallbackFull test(id, handler, pUserData) ;

	// See if this handler is already registered
	XMLEventHandlerPlusData plus(0,0,0,0) ;
	bool found = m_XMLEventMap.findFirstValueByTest(&test, &plus) ;

	if (found && plus.m_Handler != 0)
		return plus.getCallbackID() ;

	// So far we only support the XML TRACE command and this is implemented
	// by registering for the EVENT_STRUCTURED_OUTPUT print event, capturing
	// that output, parsing it and passing that parsed data back to the caller.
	// We use this implementation because currently the kernel is generating
	// raw XML strings.  Later we'll hopefully convert over to the kernel
	// creating ElementXML objects which will just be passed directly over to the client
	// without the string-format & parsing step.
	assert(id == smlEVENT_XML_TRACE_OUTPUT) ;

	// If we have no handlers registered with the kernel, then we need
	// to register for the print event (which we'll then parse to create XML objects).
	// No need to do this multiple times.
	if (m_XMLEventMap.getListSize(id) == 0)
	{
		m_XMLCallback = RegisterForPrintEvent(smlEVENT_STRUCTURED_OUTPUT, &XMLEventImplementationHandler, NULL) ;
	}

	// Record the handler
	m_CallbackIDCounter++ ;

	XMLEventHandlerPlusData handlerPlus(id, handler, pUserData, m_CallbackIDCounter) ;
	m_XMLEventMap.add(id, handlerPlus, addToBack) ;

	// Return the ID.  We use this later to unregister the callback
	return m_CallbackIDCounter ;
}

bool Agent::UnregisterForXMLEvent(int callbackID)
{
	// Build a test object for the callbackID we're interested in
	TestXMLCallback test(callbackID) ;

	// Find the event ID for this callbackID
	smlXMLEventId id = m_XMLEventMap.findFirstKeyByTest(&test, (smlXMLEventId)-1) ;

	if (id == -1)
		return false ;

	// Remove the handler from our map
	m_XMLEventMap.removeAllByTest(&test) ;

	// If we just removed the last handler, then unregister
	// for the matching print event (that we use to implement XML)
	if (m_XMLEventMap.getListSize(id) == 0)
	{
		UnregisterForPrintEvent(m_XMLCallback) ;
	}

	return true ;
}

/*************************************************************
* @brief Returns the id object for the input link.
*		 The agent retains ownership of this object.
*************************************************************/
Identifier* Agent::GetInputLink()
{
	return GetWM()->GetInputLink() ;
}

/*************************************************************
* @brief Returns the id object for the output link.
*		 The agent retains ownership of this object.
*************************************************************/
Identifier* Agent::GetOutputLink()
{
	return GetWM()->GetOutputLink() ;
}

/*************************************************************
* @brief Get number of changes to output link.
*        (This is since last call to "ClearOuputLinkChanges").
*************************************************************/
int	Agent::GetNumberOutputLinkChanges()
{
	OutputDeltaList* pDeltas = GetWM()->GetOutputLinkChanges() ;
	return pDeltas->GetSize() ;
}

/*************************************************************
* @brief Get the n-th wme added or deleted to output link
*        (This is since last call to "ClearOuputLinkChanges").
*************************************************************/
WMElement* Agent::GetOutputLinkChange(int index)
{
	OutputDeltaList* pDeltas = GetWM()->GetOutputLinkChanges() ;
	WMDelta* pDelta = pDeltas->GetDeltaWME(index) ;

	if (!pDelta)
		return NULL ;

	WMElement* pWME = pDelta->getWME() ;
	
	return pWME ;
}

/*************************************************************
* @brief Returns true if the n-th wme change to the output-link
*		 was a wme being added.  (false => it was a wme being deleted).
*        (This is since last call to "ClearOuputLinkChanges").
*************************************************************/
bool Agent::IsOutputLinkChangeAdd(int index)
{
	OutputDeltaList* pDeltas = GetWM()->GetOutputLinkChanges() ;
	WMDelta* pDelta = pDeltas->GetDeltaWME(index) ;

	if (!pDelta)
		return false ;

	bool isAddition = (pDelta->getChangeType() == WMDelta::kAdded) ;

	return isAddition ;
}

/*************************************************************
* @brief Clear the current list of changes to the output-link.
*		 You should call this after processing the list of changes.
*************************************************************/
void Agent::ClearOutputLinkChanges()
{
	GetWM()->ClearOutputLinkChanges() ;
}

/*************************************************************
* @brief Get the number of "commands".  A command in this context
*		 is an identifier wme that have been added to the top level of
*		 the output-link since the last call to "ClearOutputLinkChanges".
*
*		 NOTE: This function may involve searching a list so it's
*		 best to not call it repeatedly.
*		 
*************************************************************/
int	Agent::GetNumberCommands()
{
	// Method is to search all top level output link wmes and see which have
	// just been added and are identifiers.
	int count = 0 ;

	Identifier* pOutputLink = GetOutputLink() ;

	if (!pOutputLink)
		return 0 ;

	for (Identifier::ChildrenIter iter = pOutputLink->GetChildrenBegin() ; iter != pOutputLink->GetChildrenEnd() ; iter++)
	{
		WMElement *pWME = *iter ;

		if (pWME->IsIdentifier() && pWME->IsJustAdded())
			count++ ;
	}

	return count ;
}

/*************************************************************
* @brief Get the n-th "command".  A command in this context
*		 is an identifier wme that have been added to the top level of
*		 the output-link since the last call to "ClearOutputLinkChanges".
*
*		 Returns NULL if index is out of range.
*
* @param index	The 0-based index for which command to get.
*************************************************************/
Identifier* Agent::GetCommand(int index)
{
	// Method is to search all top level output link wmes and see which have
	// just been added and are identifiers.
	Identifier* pOutputLink = GetOutputLink() ;

	if (!pOutputLink)
		return NULL ;

	for (Identifier::ChildrenIter iter = pOutputLink->GetChildrenBegin() ; iter != pOutputLink->GetChildrenEnd() ; iter++)
	{
		WMElement *pWME = *iter ;

		if (pWME->IsIdentifier() && pWME->IsJustAdded())
		{
			if (index == 0)
				return (Identifier*)pWME ;
			index-- ;
		}
	}

	return NULL ;
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
StringElement* Agent::CreateStringWME(Identifier* parent, char const* pAttribute, char const* pValue)
{
	if (!parent)
		return NULL ;

	return GetWM()->CreateStringWME(parent, pAttribute, pValue) ;
}

/*************************************************************
* @brief Same as CreateStringWME but for a new WME that has
*		 an identifier as its value.
*
*		 The identifier value is generated here and will be
*		 different from the value Soar assigns in the kernel.
*		 The kernel will keep a map for translating back and forth.
*************************************************************/
Identifier* Agent::CreateIdWME(Identifier* parent, char const* pAttribute)
{
	if (!parent)
		return NULL ;

	return GetWM()->CreateIdWME(parent, pAttribute) ;
}

/*************************************************************
* @brief Creates a new WME that has an identifier as its value.
*		 The value in this case is the same as an existing identifier.
*		 This allows us to create a graph rather than a tree.
*************************************************************/
Identifier*	Agent::CreateSharedIdWME(Identifier* parent, char const* pAttribute, Identifier* pSharedValue)
{
	if (!parent || !pSharedValue)
		return NULL ;

	return GetWM()->CreateSharedIdWME(parent, pAttribute, pSharedValue) ;
}

/*************************************************************
* @brief Same as CreateStringWME but for a new WME that has
*		 an int as its value.
*************************************************************/
IntElement* Agent::CreateIntWME(Identifier* parent, char const* pAttribute, int value)
{
	if (!parent)
		return NULL ;

	return GetWM()->CreateIntWME(parent, pAttribute, value) ;
}

/*************************************************************
* @brief Same as CreateStringWME but for a new WME that has
*		 a floating point value.
*************************************************************/
FloatElement* Agent::CreateFloatWME(Identifier* parent, char const* pAttribute, double value)
{
	if (!parent)
		return NULL ;

	return GetWM()->CreateFloatWME(parent, pAttribute, value) ;
}

/*************************************************************
* @brief Update the value of an existing WME.
*		 The value is not actually sent to the kernel
*		 until "Commit" is called.
*************************************************************/
void Agent::Update(StringElement* pWME, char const* pValue) { GetWM()->UpdateString(pWME, pValue) ; }
void Agent::Update(IntElement* pWME, int value)				{ GetWM()->UpdateInt(pWME, value) ; }
void Agent::Update(FloatElement* pWME, double value)		{ GetWM()->UpdateFloat(pWME, value) ; }

/*************************************************************
* @brief Searches for a WME that has the given identifier value.
*		 There can be multiple WMEs that share the same identifier value.
*		 (You can use the index to find a specific one).
*
* @param pId			The id to look for (e.g. "O4" -- kernel side or "p3" -- client side)
* @param searchInput	If true, searches from input-link down
* @param searchOutput	If true, searches from output-link down
* @param index			If non-zero, finds the n-th match
*************************************************************/
Identifier*	Agent::FindIdentifier(char const* pID, bool searchInput, bool searchOutput, int index)
{
	return GetWM()->FindIdentifier(pID, searchInput, searchOutput, index) ;
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
bool Agent::DestroyWME(WMElement* pWME)
{
	if (!pWME)
		return false ;

	return GetWM()->DestroyWME(pWME) ;
}

/*************************************************************
* @brief Send the most recent list of changes to working memory
*		 over to the kernel.
*************************************************************/
bool Agent::Commit()
{
	return GetWM()->Commit() ;
}

/*************************************************************
* @brief Reinitialize this Soar agent.
*		 This will also cause the output link structures stored
*		 here to be erased and the current input link to be sent over
*		 to the Soar agent for the start of its next run.
*************************************************************/
char const* Agent::InitSoar()
{
	std::string cmd = "init-soar" ;

	// Execute the command.
	// The init-soar causes an event to be sent back from the kernel and when
	// we get that, we'll queue up the input link information to be sent over again
	// and also erase our output link information.
	// (See the InitSoarHandler in ClientKernel.cpp)
	char const* pResult = ExecuteCommandLine(cmd.c_str()) ;
	return pResult ;
}

/*************************************************************
* @brief Interrupt the currently running Soar agent.
*
* Call this after calling "Run" in order to stop a Soar agent.
* The usual way to do this is to register for an event (e.g. AFTER_DECISION_CYCLE)
* and in that event handler decide if the user wishes to stop soar.
* If so, call to this method inside that handler.
*
* The request to Stop may not be honored immediately.
* Soar will stop at the next point it is considered safe to do so.
*
*************************************************************/
char const*	Agent::StopSelf()
{
	std::string cmd = "stop-soar --self" ;

	// Execute the command.
	char const* pResult = ExecuteCommandLine(cmd.c_str()) ;
	return pResult ;
}

/*************************************************************
* @brief Run Soar for the specified number of decisions
*************************************************************/
char const* Agent::RunSelf(unsigned long numberSteps, smlRunStepSize stepSize)
{
	// Convert int to a string
	std::ostringstream ostr ;
	ostr << numberSteps ;

	// Create the command line for the run command
	std::string step = (stepSize == sml_DECISION) ? "-d" : (stepSize == sml_PHASE) ? "-p" : "-e" ;
	std::string cmd = "run " + step + " " + ostr.str() ;

	// Execute the run command.
	char const* pResult = ExecuteCommandLine(cmd.c_str()) ;
	return pResult ;
}

char const* Agent::RunSelfForever()
{
	// Create the command line for the run command
	std::string cmd = "run --self" ;

	// Execute the run command.
	char const* pResult = ExecuteCommandLine(cmd.c_str()) ;
	return pResult ;
}

/*************************************************************
* @brief   Controls whether this agent will break when it next generates
*		   output while running.
*
* @param state	If true, causes Soar to break on output.  If false, Soar will not break.
*************************************************************/
bool Agent::SetStopSelfOnOutput(bool state)
{
	AnalyzeXML response ;

	bool ok = GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_StopOnOutput, GetAgentName(), sml_Names::kParamValue, state ? sml_Names::kTrue : sml_Names::kFalse) ;
	return ok ;
}

/*************************************************************
* @brief   Run Soar until either output is generated or
*		   the maximum number of decisions is reached.
*
* This function also calls "ClearOutputLinkChanges" so methods
* like "IsJustAdded" will refer to the changes that occur as a result of
* this run.
*
* We don't generally want Soar to just run until it generates
* output without any limit as an error in the AI logic might cause
* it to never return control to the environment.
*
* @param maxDecisions	If Soar runs for this many decisions without generating output, stop.
*						15 was used in SGIO.
*************************************************************/
char const* Agent::RunSelfTilOutput(unsigned long maxDecisions)
{
	ClearOutputLinkChanges() ;

	// Send any pending input link changes to Soar
	Commit() ;

	SetStopSelfOnOutput(true) ;
	return RunSelf(maxDecisions) ;
}

/*************************************************************
* @brief Resend the complete input link to the kernel
*		 and remove our output link structures.
*		 We do this when the user issues an "init-soar" event.
*		 There should be no reason for the client to call this method directly.
*************************************************************/
void Agent::Refresh()
{
	GetWM()->Refresh() ;
}

/*************************************************************
* @brief Process a command line command and return the result
*        as a string.
*
* @param pCommandLine Command line string to process.
* @param pAgentName Agent name to apply the command line to.
* @returns The string form of output from the command.
*************************************************************/
char const* Agent::ExecuteCommandLine(char const* pCommandLine)
{
	return GetKernel()->ExecuteCommandLine(pCommandLine, GetAgentName()) ;
}

/*************************************************************
* @brief Execute a command line command and return the result
*		 as an XML object.
*
* @param pCommandLine Command line string to process.
* @param pAgentName   Agent name to apply the command line to.
* @param pResponse    The XML response will be returned within this object.
*                     The caller should allocate this and pass it in.
* @returns True if the command succeeds.
*************************************************************/
bool Agent::ExecuteCommandLineXML(char const* pCommandLine, AnalyzeXML* pResponse)
{
	return GetKernel()->ExecuteCommandLineXML(pCommandLine, GetAgentName(), pResponse) ;
}

/*************************************************************
* @brief Get last command line result
*
* (This is the last result for any command sent to the kernel,
*  not just for this agent).
*
* @returns True if the last command line call succeeded.
*************************************************************/
bool Agent::GetLastCommandLineResult()
{
	return GetKernel()->GetLastCommandLineResult() ;
}