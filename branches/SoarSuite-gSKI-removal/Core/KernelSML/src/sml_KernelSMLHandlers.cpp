#include <portability.h>

/////////////////////////////////////////////////////////////////
// KernelSML handlers file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// These are the command handler methods for KernelSML.
// Just moved to a separate implementation file to
// keep the code more manageable.
//
/////////////////////////////////////////////////////////////////

#include "sml_KernelSML.h"

#include "sml_Utils.h"
#include "sml_AgentSML.h"
#include "sml_Connection.h"
#include "sml_StringOps.h"
#include "sml_OutputListener.h"
#include "sml_ConnectionManager.h"
#include "sml_TagResult.h"
#include "sml_TagName.h"
#include "sml_TagWme.h"
#include "sml_TagFilter.h"
#include "sml_TagCommand.h"
#include "sml_Events.h"
#include "sml_RunScheduler.h"
#include "KernelHeaders.h"

#include <iostream>
#include <fstream>
#include <map>
#include <stdlib.h>
#include <assert.h>

#include "gSKI_KernelFactory.h"
#include "gSKI_Kernel.h"

#include "gSKI_Error.h"
#include "gSKI_ErrorIds.h"
#include "gSKI_Enumerations.h"
#include "gSKI_Events.h"
#include "IgSKI_OutputProcessor.h"
#include "IgSKI_InputProducer.h"
#include "IgSKI_Symbol.h"
#include "IgSKI_Wme.h"
#include "IgSKI_WorkingMemory.h"
#include "IgSKI_WMObject.h"
#include "gSKI_InputLink.h"
#include "gSKI_OutputLink.h"

using namespace sml ;

/*
  ==================================
  SML input producer
  ==================================
*/
class sml_InputProducer: public gSKI::IInputProducer
{
public:

   // Simple constructor
   sml_InputProducer(KernelSML* pKernelSML)
   {
	   m_KernelSML	= pKernelSML ;
   }
   
   virtual ~sml_InputProducer() 
   {
   }
   
   // The update function required by the IInputProducer interface
   // (Responsible for updating the state of working memory)
   virtual void Update(gSKI::IWorkingMemory* /*wmemory*/, gSKI::IWMObject* /*obj*/)
   {
	   // Check for any new incoming commands from remote connections.
	   // We do this in an input producer so it's once per decision during a run and
	   // the input phase seems like the correct point for incoming commands.
	   m_KernelSML->ReceiveAllMessages() ;
   }

private:
	sml::KernelSML*		m_KernelSML ;
};

void KernelSML::BuildCommandMap()
{
	m_CommandMap[sml_Names::kCommand_CreateAgent]		= &sml::KernelSML::HandleCreateAgent ;
	m_CommandMap[sml_Names::kCommand_DestroyAgent]		= &sml::KernelSML::HandleDestroyAgent ;
	m_CommandMap[sml_Names::kCommand_LoadProductions]	= &sml::KernelSML::HandleLoadProductions ;
	m_CommandMap[sml_Names::kCommand_GetInputLink]		= &sml::KernelSML::HandleGetInputLink ;
	m_CommandMap[sml_Names::kCommand_Input]				= &sml::KernelSML::HandleInput ;
	m_CommandMap[sml_Names::kCommand_CommandLine]		= &sml::KernelSML::HandleCommandLine ;
	m_CommandMap[sml_Names::kCommand_ExpandCommandLine]	= &sml::KernelSML::HandleExpandCommandLine ;
	m_CommandMap[sml_Names::kCommand_CheckForIncomingCommands] = &sml::KernelSML::HandleCheckForIncomingCommands ;
	m_CommandMap[sml_Names::kCommand_GetAgentList]		= &sml::KernelSML::HandleGetAgentList ;
	m_CommandMap[sml_Names::kCommand_RegisterForEvent]	= &sml::KernelSML::HandleRegisterForEvent ;
	m_CommandMap[sml_Names::kCommand_UnregisterForEvent]= &sml::KernelSML::HandleRegisterForEvent ;	// Note -- both register and unregister go to same handler
	m_CommandMap[sml_Names::kCommand_FireEvent]			= &sml::KernelSML::HandleFireEvent ;
	m_CommandMap[sml_Names::kCommand_SuppressEvent]		= &sml::KernelSML::HandleSuppressEvent ;
	m_CommandMap[sml_Names::kCommand_SetInterruptCheckRate] = &sml::KernelSML::HandleSetInterruptCheckRate ;
	m_CommandMap[sml_Names::kCommand_GetVersion]		= &sml::KernelSML::HandleGetVersion ;
	m_CommandMap[sml_Names::kCommand_Shutdown]			= &sml::KernelSML::HandleShutdown ;
	m_CommandMap[sml_Names::kCommand_IsSoarRunning]		= &sml::KernelSML::HandleIsSoarRunning ;
	m_CommandMap[sml_Names::kCommand_GetConnections]	= &sml::KernelSML::HandleGetConnections ;
	m_CommandMap[sml_Names::kCommand_SetConnectionInfo] = &sml::KernelSML::HandleSetConnectionInfo ;
	m_CommandMap[sml_Names::kCommand_GetAllInput]		= &sml::KernelSML::HandleGetAllInput ;
	m_CommandMap[sml_Names::kCommand_GetAllOutput]		= &sml::KernelSML::HandleGetAllOutput ;
	m_CommandMap[sml_Names::kCommand_GetRunState]		= &sml::KernelSML::HandleGetRunState ;
	m_CommandMap[sml_Names::kCommand_IsProductionLoaded]= &sml::KernelSML::HandleIsProductionLoaded ;
	m_CommandMap[sml_Names::kCommand_SendClientMessage] = &sml::KernelSML::HandleSendClientMessage ;
	m_CommandMap[sml_Names::kCommand_WasAgentOnRunList] = &sml::KernelSML::HandleWasAgentOnRunList ;
	m_CommandMap[sml_Names::kCommand_GetResultOfLastRun]= &sml::KernelSML::HandleGetResultOfLastRun ;
	m_CommandMap[sml_Names::kCommand_GetInitialTimeTag] = &sml::KernelSML::HandleGetInitialTimeTag ;
}

/*************************************************************
* @brief	A command handler (SML message->appropriate gSKI handling).
*
* @param pAgent			The agent this command is for (can be NULL if the command is not agent specific)
* @param pCommandName	The SML command name (so one handler can handle many incoming calls if we wish)
* @param pConnection	The connection this command came in on
* @param pIncoming		The incoming, analyzed message.
* @param pResponse		The partially formed response.  This handler needs to fill in more of this.
* @returns False if we had an error and wish to generate a generic error message (based on the incoming call + pError)
*          True if the call succeeded or we generated another more specific error already.
*************************************************************/
bool KernelSML::HandleCreateAgent(AgentSML* pAgentSML, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse)
{
	assert( !pAgentSML ); // FIXME: handle gracefully

	// Get the parameters
	char const* pName = pIncoming->GetArgString(sml_Names::kParamName) ;

	if (!pName)
	{
		return InvalidArg(pConnection, pResponse, pCommandName, "Agent name missing") ;
	}

	agent* pSoarAgent = create_soar_agent(GetSoarKernel(), (char*)pName);

	pAgentSML = new AgentSML(this, pSoarAgent) ;

	// Update our maps
	m_KernelAgentMap[ pSoarAgent ] = pAgentSML ;
	m_AgentMap[ pAgentSML->GetName() ] = pAgentSML ;

	pAgentSML->InitListeners() ;	// This must happen before the soar agent is initialized

	pAgentSML->Init() ;

	// Notify listeners that there is a new agent
	this->FireAgentEvent(pAgentSML, smlEVENT_AFTER_AGENT_CREATED) ;

	// Register for output from this agent

	// Mark the agent's status as just having been created for all connections
	// Note--agent status for connections just refers to the last agent created, i.e. this one.
	m_pConnectionManager->SetAgentStatus(sml_Names::kStatusCreated) ;

	// We also need to listen to input events so we can pump waiting sockets and get interrupt messages etc.
	sml_InputProducer* pInputProducer = new sml_InputProducer(this) ;
	pAgentSML->SetInputProducer(pInputProducer) ;

	// Add the input producer to the top level of the input link (doesn't matter for us which WME it's attached to)
	gSKI::IWMObject* pRoot = NULL ;
	pAgentSML->m_inputlink->GetRootObject(&pRoot) ;
	pAgentSML->m_inputlink->AddInputProducer(pRoot, pInputProducer) ;
	pRoot->Release() ;

	pAgentSML->m_inputlink->GetInputLinkMemory()->m_RemoveWmeCallback = RemoveInputWMERecordsCallback;

	// Return true if we got an agent constructed.
	return true ;
}

// Handle registering and unregistering for kernel events
bool KernelSML::HandleRegisterForEvent(AgentSML* pAgentSML, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse)
{
	// Decide if registering or unregistering
	bool registerForEvent = (strcmp(pCommandName, sml_Names::kCommand_RegisterForEvent) == 0) ;

	// The value sent over is actually defined in sml_Events.h but we're just casting it over to egSKIEventId.
	// So let's add some checks here to make sure that the two tables are synchronized.
	// (If we wish we could introduce a mapping here between the two sets of ids but for now we're not doing that).
	assert(gSKIEVENT_INVALID_EVENT == (egSKIGenericEventId)smlEVENT_INVALID_EVENT) ;	// First matches
	assert(gSKIEVENT_AFTER_RUNNING == (smlRunEventId)smlEVENT_AFTER_RUNNING) ;	// Random one in middle matches
	assert(gSKIEVENT_BEFORE_AGENT_REINITIALIZED == (egSKIAgentEventId)smlEVENT_BEFORE_AGENT_REINITIALIZED) ;	// Another middle one matches
	assert(gSKIEVENT_PRINT == (egSKIPrintEventId)smlEVENT_PRINT); // What the heck, another one
	assert(gSKIEVENT_RHS_USER_FUNCTION == (smlRhsEventId)smlEVENT_RHS_USER_FUNCTION) ;
	assert(gSKIEVENT_LAST== (egSKIGenericEventId)smlEVENT_LAST) ;

	// Get the parameters
	char const* pEventName = pIncoming->GetArgString(sml_Names::kParamEventID) ;

	if (!pEventName)
	{
		return InvalidArg(pConnection, pResponse, pCommandName, "Event id is missing") ;
	}

	// Convert from the event name to the id value
	int id = ConvertStringToEvent(pEventName) ;

	// Decide what type of event this is and where to register/unregister it
	// gSKI uses a different class for each type of event.  We collect those together
	// where possible to reduce the amount of extra scaffolding code.
	if(IsSystemEventID(id))
	{
		// System Events
		if (registerForEvent)
			this->AddSystemListener((smlSystemEventId)id, pConnection) ;
		else
			this->RemoveSystemListener((smlSystemEventId)id, pConnection) ;

	} else if(IsAgentEventID(id)) {

		// Agent events
		if (registerForEvent)
			this->AddAgentListener(static_cast<smlAgentEventId>(id), pConnection) ;
		else
			this->RemoveAgentListener(static_cast<smlAgentEventId>(id), pConnection) ;
	} else if(IsRhsEventID(id)) {

		// Rhs user functions
		char const* pRhsFunctionName = pIncoming->GetArgString(sml_Names::kParamName) ;

		if (!pRhsFunctionName)
			return InvalidArg(pConnection, pResponse, pCommandName, "Registering for rhs user function, but no function name was provided") ;

		if (registerForEvent)
			this->AddRhsListener(pRhsFunctionName, pConnection) ;
		else
			this->RemoveRhsListener(pRhsFunctionName, pConnection) ;
	} else if(IsRunEventID(id)) {

		// Run events
		if (!pAgentSML)
			return InvalidArg(pConnection, pResponse, pCommandName, "No agent name for an event that is handled by an agent") ;

		// Register or unregister for this event
		if (registerForEvent)
			pAgentSML->AddRunListener(static_cast<smlRunEventId>(id), pConnection) ;
		else
			pAgentSML->RemoveRunListener(static_cast<smlRunEventId>(id), pConnection) ;
	} else if(IsProductionEventID(id)) {

		// Production event
		if (!pAgentSML)
			return InvalidArg(pConnection, pResponse, pCommandName, "No agent name for an event that is handled by an agent") ;

		// Register or unregister for this event
		if (registerForEvent)
			pAgentSML->AddProductionListener(static_cast<smlProductionEventId>(id), pConnection) ;
		else
			pAgentSML->RemoveProductionListener(static_cast<smlProductionEventId>(id), pConnection) ;
	} else if(IsXMLEventID(id)) {

		// XML Event
		if (!pAgentSML)
			return InvalidArg(pConnection, pResponse, pCommandName, "No agent name for an event that is handled by an agent") ;

		// Register or unregister for this event
		if (registerForEvent)
			pAgentSML->AddXMLListener(static_cast<smlXMLEventId>(id), pConnection) ;
		else
			pAgentSML->RemoveXMLListener(static_cast<smlXMLEventId>(id), pConnection) ;

	} else if (IsUpdateEventID(id))	{
		if (registerForEvent)
			AddUpdateListener(static_cast<smlUpdateEventId>(id), pConnection) ;
		else
			RemoveUpdateListener(static_cast<smlUpdateEventId>(id), pConnection) ;
	} else if (IsStringEventID(id))	{
		if (registerForEvent)
			AddStringListener(static_cast<smlStringEventId>(id), pConnection) ;
		else
			RemoveStringListener(static_cast<smlStringEventId>(id), pConnection) ;
	}
	else if(IsPrintEventID(id)) {

		// Print event
		if (!pAgentSML)
			return InvalidArg(pConnection, pResponse, pCommandName, "No agent name for an event that is handled by an agent") ;

		// Register or unregister for this event
		if (registerForEvent)
			pAgentSML->AddPrintListener(static_cast<smlPrintEventId>(id), pConnection) ;
		else
			pAgentSML->RemovePrintListener(static_cast<smlPrintEventId>(id), pConnection) ;
	} else if( id == smlEVENT_OUTPUT_PHASE_CALLBACK ) {

		// Output event
		OutputListener* pOutputListener = pAgentSML->GetOutputListener() ;

		// Register this connection as listening for this event
		if (registerForEvent)
			pOutputListener->AddListener(smlEVENT_OUTPUT_PHASE_CALLBACK, pConnection) ;
		else
			pOutputListener->RemoveListener(smlEVENT_OUTPUT_PHASE_CALLBACK, pConnection) ;
	} else {
		// The event didn't match any of our handlers
		return InvalidArg(pConnection, pResponse, pCommandName, "KernelSML doesn't know how to handle that event id") ;
	}

	return true ;
}

bool KernelSML::HandleSetConnectionInfo(AgentSML* /*pAgentSML*/, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse)
{
	// Get the parameters
	char const* pName   = pIncoming->GetArgString(sml_Names::kConnectionName) ;
	char const* pStatus = pIncoming->GetArgString(sml_Names::kConnectionStatus) ;
	char const* pAgentStatus = pIncoming->GetArgString(sml_Names::kAgentStatus) ;

	if (!pName)
	{
		return InvalidArg(pConnection, pResponse, pCommandName, "Connection name is missing") ;
	}

	if (!pStatus)
	{
		return InvalidArg(pConnection, pResponse, pCommandName, "Connection status is missing") ;
	}

	if (!pAgentStatus)
	{
		return InvalidArg(pConnection, pResponse, pCommandName, "Agent status is missing") ;
	}

	// Execute the command
	pConnection->SetName(pName) ;
	pConnection->SetStatus(pStatus) ;
	pConnection->SetAgentStatus(pAgentStatus) ;

	return true ;
}

bool KernelSML::HandleGetConnections(AgentSML* /*pAgentSML*/, char const* /*pCommandName*/, Connection* /*pCallingConnection*/, AnalyzeXML* /*pIncoming*/, ElementXML* pResponse)
{
	// Create the result tag
	TagResult* pTagResult = new TagResult() ;
	pTagResult->AddAttribute(sml_Names::kCommandOutput, sml_Names::kStructuredOutput) ;

	// Walk the list of connections and return their info
	int index = 0 ;
	Connection* pConnection = m_pConnectionManager->GetConnectionByIndex(index) ;

	while (pConnection)
	{
		// Create the connection tag
		ElementXML* pTagConnection = new ElementXML() ;
		pTagConnection->SetTagName(sml_Names::kTagConnection) ;

		// Fill in the info
		pTagConnection->AddAttribute(sml_Names::kConnectionId, pConnection->GetID()) ;
		pTagConnection->AddAttribute(sml_Names::kConnectionName, pConnection->GetName()) ;
		pTagConnection->AddAttribute(sml_Names::kConnectionStatus, pConnection->GetStatus()) ;
		pTagConnection->AddAttribute(sml_Names::kAgentStatus, pConnection->GetAgentStatus()) ;

		// Add the connection into the result
		pTagResult->AddChild(pTagConnection) ;

		// Get the next connection.  Returns null when go beyond limit.
		// (This provides thread safe access to the list, in case it changes during this enumeration)
		index++ ;
		pConnection = m_pConnectionManager->GetConnectionByIndex(index) ;
	}

	// Add the result tag to the response
	pResponse->AddChild(pTagResult) ;

	// Return true to indicate we've filled in all of the result tag we need
	return true ;

}

bool KernelSML::HandleDestroyAgent(AgentSML* pAgentSML, char const* /*pCommandName*/, Connection* /*pConnection*/, AnalyzeXML* /*pIncoming*/, ElementXML* /*pResponse*/)
{
	if (!pAgentSML)
		return false ;

	FireAgentEvent( pAgentSML, smlEVENT_BEFORE_AGENT_DESTROYED );

	// Release any wmes or other objects we're keeping
	pAgentSML->DeleteSelf() ;
	pAgentSML = NULL ;	// At this point the pointer is invalid so clear it.

	return true ;
}

class KernelSML::OnSystemStopDeleteAll: public gSKI::ISystemListener
{
public:
	// This handler is called right before the agent is actually deleted
	// inside gSKI.  We need to clean up any object we own now.
	virtual void HandleEvent(smlSystemEventId, gSKI::Kernel* /*pKernel*/)
	{
		KernelSML* pKernelSML = KernelSML::GetKernelSML() ;

		pKernelSML->DeleteAllAgents(true) ;

		delete this ;
	}
};

// Shutdown is an irrevocal request to delete all agents and prepare for kernel deletion.
bool KernelSML::HandleShutdown(AgentSML* /*pAgentSML*/, char const* /*pCommandName*/, Connection* /*pConnection*/, AnalyzeXML* /*pIncoming*/, ElementXML* /*pResponse*/)
{
	// Notify everyone that the system is about to shutdown.
	FireSystemEvent(smlEVENT_BEFORE_SHUTDOWN) ;

	// Delete all agents explicitly now (so listeners can hear that the agents have been destroyed).
	DeleteAllAgents(true) ;

	return true ;
}

// Return information about the current runtime state of the agent (e.g. phase, decision cycle count etc.)
bool KernelSML::HandleGetRunState(AgentSML* pAgentSML, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse)
{
	// Look up what type of information to report.
	char const* pValue = pIncoming->GetArgString(sml_Names::kParamValue) ;

	if (!pValue)
	{
		return InvalidArg(pConnection, pResponse, pCommandName, "Need to specify the type of information wanted.") ;
	}

	std::ostringstream buffer;

	if (strcmp(pValue, sml_Names::kParamPhase) == 0)
	{
		// Report the current phase.
		buffer << pAgentSML->GetCurrentPhase();
	}
	else if (strcmp(pValue, sml_Names::kParamDecision) == 0)
	{
		// Report the current decision number of decisions that have been executed
		buffer << pAgentSML->GetNumDecisionsExecuted();
	}
	else if (strcmp(pValue, sml_Names::kParamRunState) == 0) 	 
	{ 	 
		// Report the current run state 	 
		buffer << pAgentSML->GetRunState() ; 	 
	}
	else
	{
		return InvalidArg(pConnection, pResponse, pCommandName, "Didn't recognize the type of information requested in GetRunState().") ;
	}

	std::string bufferStdString = buffer.str();
	const char* bufferCString = bufferStdString.c_str();
	return this->ReturnResult(pConnection, pResponse, bufferCString) ;
}

// Return information about the current runtime state of the agent (e.g. phase, decision cycle count etc.)
bool KernelSML::HandleWasAgentOnRunList(AgentSML* pAgentSML, char const* /*pCommandName*/, Connection* pConnection, AnalyzeXML* /*pIncoming*/, ElementXML* pResponse)
{
	bool wasRun = pAgentSML->WasAgentOnRunList() ;
	return this->ReturnBoolResult(pConnection, pResponse, wasRun) ;
}

// Return the result code from the last run
bool KernelSML::HandleGetResultOfLastRun(AgentSML* pAgentSML, char const* /*pCommandName*/, Connection* pConnection, AnalyzeXML* /*pIncoming*/, ElementXML* pResponse)
{
	smlRunResult runResult = pAgentSML->GetResultOfLastRun() ;
	return this->ReturnIntResult(pConnection, pResponse, runResult) ;
}

// Return a starting value for client side time tags for this client to use 	 
bool KernelSML::HandleGetInitialTimeTag(AgentSML* /*pAgentSML*/, char const* /*pCommandName*/, Connection* pConnection, AnalyzeXML* /*pIncoming*/, ElementXML* pResponse) 	 
{ 	 
	// We use negative values for client time tags (so we can tell they're client side not kernel side) 	 
	long timeTagStart = -1 ; 	 

	// Allow up to 8 simultaneous clients using different ids 	 
	int maxTries = 8 ; 	 
	bool done = false ; 	 

	while (maxTries > 0 && !done) 	 
	{ 	 
		// Walk the list of connections and see if we can find an time tag start value that's not in use 	 
		// (We do this walking so if connections are made, broken and remade and we'll reuse the id space). 	 
		int index = 0 ; 	 
		Connection* connect = m_pConnectionManager->GetConnectionByIndex(index) ; 	 

		// See if any existing connection is using the timeTagStart value already 	 
		bool ok = true ; 	 
		while (connect && ok) 	 
		{ 	 
			if (connect->GetInitialTimeTagCounter() == timeTagStart) 	 
			{ 	 
				ok = false ; 	 
				timeTagStart -= (1<<27) ;       // 8 * (1<<27) is (1<<30) so won't overflow.  Allows (1<<27) values per client w/o collision or more than 100 million wmes each. 	 
			} 	 

			index++ ; 	 
			connect = m_pConnectionManager->GetConnectionByIndex(index) ; 	 
		} 	 

		// If this value's not already in use we're done 	 
		// Otherwise, we'll test the new value. 	 
		if (ok) 	 
			done = true ; 	 

		maxTries-- ; 	 
	} 	 

	// If we fail this it means we couldn't find a valid start value for the time tag counter. 	 
	// Either we have 8 existing connections or there's a bug in this code. 	 
	assert(maxTries >= 0) ; 	 

	// Record the value we picked and return it. 	 
	pConnection->SetInitialTimeTagCounter(timeTagStart) ; 	 
	return this->ReturnIntResult(pConnection, pResponse, timeTagStart) ; 	 
}

 // Returns true if the production name is currently loaded
bool KernelSML::HandleIsProductionLoaded(AgentSML* pAgentSML, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse)
{
	// Look up the name of the production
	char const* pName = pIncoming->GetArgString(sml_Names::kParamName) ;

	if (!pName)
	{
		return InvalidArg(pConnection, pResponse, pCommandName, "Need to specify the production name to check.") ;
	}

	Symbol* sym = find_sym_constant( pAgentSML->GetSoarAgent(), pName );

	bool found = true;
	if (!sym || !(sym->sc.production))
	{
		found = false;
	}

	return ReturnBoolResult(pConnection, pResponse, found) ;
}

bool KernelSML::HandleGetVersion(AgentSML* /*pAgentSML*/, char const* /*pCommandName*/, Connection* pConnection, AnalyzeXML* /*pIncoming*/, ElementXML* pResponse)
{
	std::ostringstream buffer;

	// Look up the current version of Soar and return it as a string
	gSKI::Version version = this->m_pKernelFactory->GetKernelVersion() ;
	buffer << version.major << "." << version.minor << "." << version.micro;
	std::string bufferStdString = buffer.str();
	const char* bufferCString = bufferStdString.c_str();

	// Our hard-coded string should match the version returned from Soar
	assert(strcmp(sml_Names::kSoarVersionValue, bufferCString) == 0) ;

	return this->ReturnResult(pConnection, pResponse, bufferCString) ;
}

bool KernelSML::HandleIsSoarRunning(AgentSML* /*pAgentSML*/, char const* /*pCommandName*/, Connection* pConnection, AnalyzeXML* /*pIncoming*/, ElementXML* pResponse)
{
	bool isRunning = this->GetRunScheduler()->IsRunning() ;

	return this->ReturnBoolResult(pConnection, pResponse, isRunning) ;
}

bool KernelSML::HandleGetAgentList(AgentSML* /*pAgentSML*/, char const* /*pCommandName*/, Connection* /*pConnection*/, AnalyzeXML* /*pIncoming*/, ElementXML* pResponse)
{
	// Create the result tag
	TagResult* pTagResult = new TagResult() ;
	pTagResult->AddAttribute(sml_Names::kCommandOutput, sml_Names::kStructuredOutput) ;

	// Walk the list of agents and return their names
	for (AgentMapIter iter = m_AgentMap.begin() ; iter != m_AgentMap.end() ; iter++)
	{
		// Add a name tag to the output
		TagName* pTagName = new TagName() ;
		pTagName->SetName( iter->first.c_str() ) ;
		pTagResult->AddChild(pTagName) ;
	}

	// Add the result tag to the response
	pResponse->AddChild(pTagResult) ;

	// Return true to indicate we've filled in all of the result tag we need
	return true ;
}

// Controls the frequency of the smlEVENT_INTERRUPT_CHECK event
bool KernelSML::HandleSetInterruptCheckRate(AgentSML* /*pAgentSML*/, char const* /*pCommandName*/, Connection* /*pConnection*/, AnalyzeXML* pIncoming, ElementXML* /*pResponse*/)
{
	// Get the parameters
	int newRate = pIncoming->GetArgInt(sml_Names::kParamValue, 1) ;

	// Make the call.
	GetKernel()->SetInterruptCheckRate(newRate) ;

	return true ;
}

// Fire a particular event at the request of the client.
bool KernelSML::HandleFireEvent(AgentSML* /*pAgentSML*/, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse)
{
	// Get the parameters
	char const* pEventName = pIncoming->GetArgString(sml_Names::kParamEventID) ;

	if (!pEventName)
	{
		return InvalidArg(pConnection, pResponse, pCommandName, "Event id is missing") ;
	}

	// Convert from the event name to the id value
	int id = ConvertStringToEvent(pEventName) ;

	// Make the call.  These are the only events which we allow
	// explicit client control over to date.
	if (id == smlEVENT_SYSTEM_START || id == smlEVENT_SYSTEM_STOP)
		this->FireSystemEvent((smlSystemEventId)id) ;

	return true ;
}

bool KernelSML::HandleSendClientMessage(AgentSML* pAgentSML, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse)
{
	// Get the parameters
	char const* pMessageType = pIncoming->GetArgString(sml_Names::kParamName) ;
	char const* pMessage     = pIncoming->GetArgString(sml_Names::kParamMessage) ;

	if (!pMessageType || !pMessage)
	{
		return InvalidArg(pConnection, pResponse, pCommandName, "Require a message type and a message and one is missing") ;
	}

	std::string result = this->SendClientMessage(pAgentSML, pMessageType, pMessage) ;

	return ReturnResult(pConnection, pResponse, result.c_str()) ;
}

// Prevent a particular event from firing when it next would normally do so
bool KernelSML::HandleSuppressEvent(AgentSML* /*pAgentSML*/, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse)
{
	// Get the parameters
	char const* pEventName = pIncoming->GetArgString(sml_Names::kParamEventID) ;
	bool state = pIncoming->GetArgBool(sml_Names::kParamValue, true) ;

	if (!pEventName)
	{
		return InvalidArg(pConnection, pResponse, pCommandName, "Event id is missing") ;
	}

	// Convert from the event name to the id value
	int id = ConvertStringToEvent(pEventName) ;

	// Make the call.
	if (id == smlEVENT_SYSTEM_STOP)
	{
		SetSuppressSystemStop(state) ;
	}

	return true ;
}

// Check if anyone has sent us a command (e.g. over a socket from a remote debugger)
bool KernelSML::HandleCheckForIncomingCommands(AgentSML* /*pAgentSML*/, char const* /*pCommandName*/, Connection* pConnection, AnalyzeXML* /*pIncoming*/, ElementXML* pResponse)
{
	// We let the caller know if we read at least one message
	bool receivedOneMessage = false ;

	// Also check for any incoming calls from remote sockets
	if (m_pConnectionManager)
		receivedOneMessage = m_pConnectionManager->ReceiveAllMessages() ;

	return this->ReturnBoolResult(pConnection, pResponse, receivedOneMessage) ;
}

bool KernelSML::HandleLoadProductions(AgentSML* pAgentSML, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse)
{
	if (!pAgentSML)
		return false ;

	// Get the parameters
	char const* pFilename = pIncoming->GetArgString(sml_Names::kParamFilename) ;

	if (!pFilename)
	{
		return InvalidArg(pConnection, pResponse, pCommandName, "Filename missing") ;
	}
	
	return m_CommandLineInterface.DoSource( pFilename );
}

bool KernelSML::HandleGetInputLink(AgentSML* pAgentSML, char const* /*pCommandName*/, Connection* pConnection, AnalyzeXML* /*pIncoming*/, ElementXML* pResponse)
{
	if (!pAgentSML)
		return false ;

	// We want the input link's id
	// Start with the root object for the input link
	gSKI::IWMObject* pRootObject = NULL ;
	pAgentSML->m_inputlink->GetRootObject(&pRootObject) ;

	if (pRootObject == NULL)
		return false ;

	// Get the symbol for the id of this object
	gSKI::ISymbol const* pID = pRootObject->GetId() ;

	if (pID == NULL)
	{
		pRootObject->Release() ;
		return false ;
	}

	// Turn the id symbol into an actual string
	char const* id = pID->GetString() ;
	
	if (id)
	{
		// Fill in the id string as the result of this command
		this->ReturnResult(pConnection, pResponse, id) ;
	}

	// No need to release the pID
	// because it's returned as a const by GetId().
	// pID->Release() ;

	// Clean up
	pRootObject->Release() ;

	// We succeeded if we got an id string
	return (id != NULL) ;
}

bool KernelSML::AddInputWME(AgentSML* pAgentSML, char const* pID, char const* pAttribute, char const* pValue, char const* pType, char const* pTimeTag)
{
	// We store additional information for SML in the AgentSML structure, so look that up.
	bool addingToInputLink = true ;
	gSKI::IWorkingMemory* pInputWM = pAgentSML->m_inputlink->GetInputLinkMemory() ;

	// First get the object which will own this new wme
	// Because we build from the top down, this should always exist by the
	// time we wish to add structure beneath it.
	gSKI::IWMObject* pParentObject = NULL ;
	pInputWM->GetObjectById(pID, &pParentObject) ;

	// Soar also allows the environment to modify elements on the output link.
	// This is a form of backdoor input, so we need to check on the output side too
	// if we don't find our parent on the input side.
	if (!pParentObject)
	{
		pInputWM = pAgentSML->m_outputlink->GetOutputMemory() ;
		pInputWM->GetObjectById(pID, &pParentObject) ;
		addingToInputLink = false ;
	}

	// Failed to find the parent.
	if (!pParentObject)
		return false ;

	gSKI::IWme* pWME = NULL ;

	if (IsStringEqual(sml_Names::kTypeString, pType))
	{
		// Add a WME with a string value
		pWME = pInputWM->AddWmeString(pParentObject, pAttribute, pValue) ;
	}
	else if (IsStringEqual(sml_Names::kTypeID, pType))
	{
		// There are two cases here.  We're either adding a new identifier
		// or we're adding a new wme that has an existing identifier as it's value.

		// Convert the value (itself an identifier) from client to kernel
		std::string value ;
		pAgentSML->ConvertID(pValue, &value) ;

		// See if we can find an object with this id (if so we're not adding a new identifier)
		gSKI::IWMObject* pLinkObject = NULL ;
		pInputWM->GetObjectById(value.c_str(), &pLinkObject) ;

		if (pLinkObject)
		{
			// Create a new wme with the same value as an existing wme
			pWME = pInputWM->AddWmeObjectLink(pParentObject, pAttribute, pLinkObject) ;
			pLinkObject->Release() ;
		}
		else
		{
			// Add a WME with an identifier value
			pWME = pInputWM->AddWmeNewObject(pParentObject, pAttribute) ;
		}

		if (pWME)
		{
			// We need to record the id that the kernel assigned to this object and match it against the id the
			// client is using, so that in future we can map the client's id to the kernel's.
			// voigtjr 02/2008: We need to call RecordIDMapping regardless of whether it is already
			// mapped because we need to keep track of reference counts to behave correctly regarding shared IDs
			char const* pKernelID = pWME->GetValue()->GetString() ;
			pAgentSML->RecordIDMapping(pValue, pKernelID) ;
		}
	}
	else if (IsStringEqual(sml_Names::kTypeInt, pType))
	{
		// Add a WME with an int value
		int value = atoi(pValue) ;
		pWME = pInputWM->AddWmeInt(pParentObject, pAttribute, value) ;
	}
	else if (IsStringEqual(sml_Names::kTypeDouble, pType))
	{
		// Add a WME with a float value
		double value = atof(pValue) ;
		pWME = pInputWM->AddWmeDouble(pParentObject, pAttribute, value) ;
	}

	if (!pWME)
	{
		pParentObject->Release() ;
		return false ;
	}

	// Well here's a surprise.  The kernel doesn't support a direct lookup from timeTag to wme.
	// So we need to maintain our own map out here so we can find the WME's quickly for removal.
	// So where we had planned to map from client time tag to kernel time tag, we'll instead
	// map from client time tag to IWme*.
	// That means we need to be careful to delete the IWme* objects later.
	if (pWME && addingToInputLink)
		pAgentSML->RecordTimeTag(pTimeTag, pWME) ;

	// We'll release this when the table of time tags is eventually destroyed or
	// when the wme is deleted.
//	pWME->Release() ;

	// If this is the output lin, we have to release the WME immediately,
	// otherwise there is a memory leak because there is no way for the client
	// to remove WMEs once they're added to the output link. The WME will get
	// cleaned up properly when the output command retracts and it is garbage
	// collected (DR 11/8/2007)
	if(!addingToInputLink)
	{
		pWME->Release();
	}
	pParentObject->Release() ;

	return true ;
}

bool KernelSML::RemoveInputWME(AgentSML* pAgentSML, char const* pTimeTag)
{
	gSKI::IWorkingMemory* pInputWM = pAgentSML->m_inputlink->GetInputLinkMemory() ;

	// Get the wme that matches this time tag
	gSKI::IWme* pWME = pAgentSML->ConvertTimeTag(pTimeTag) ;

	// Failed to find the wme--that shouldn't happen.
	if (!pWME)
		return false ;

	// If this is an identifier, need to remove it from the ID mapping table too.
	if (pWME->GetValue()->GetType() == gSKI_OBJECT)
	{
		// Get the kernel-side identifier
		std::string id = pWME->GetValue()->GetString() ;

		// Remove it from the id mapping table
		pAgentSML->RemoveID(id.c_str()) ;
	}

	// Remove the wme from working memory
	pInputWM->RemoveWme(pWME) ;

	// Remove the object from the time tag table because
	// we no longer own it.
	pAgentSML->RemoveTimeTag(pTimeTag) ;

	return true ;
}

void KernelSML::RemoveInputWMERecordsCallback(agent* pSoarAgent, gSKI::IWme* pWME)
{
	assert( pSoarAgent );
	s_pKernel->RemoveInputWMERecords(pSoarAgent, pWME);
}

void KernelSML::RemoveInputWMERecords(agent* pSoarAgent, gSKI::IWme* pWME)
{
	// We store additional information for SML in the AgentSML structure, so look that up.
	AgentSML* pAgentSML = GetAgentSML( pSoarAgent->name ) ;

	if (!pAgentSML || !pWME)
		return;

	// If this is an identifier, need to remove it from the ID mapping table too.
	if (pWME->GetValue()->GetType() == gSKI_OBJECT)
	{
		// Get the kernel-side identifier
		std::string id = pWME->GetValue()->GetString() ;

		// Remove it from the id mapping table
		pAgentSML->RemoveID(id.c_str()) ;
	}

	// Remove the object from the time tag table because
	// we no longer own it.
	pAgentSML->RemoveTimeTagByWmeSLOW(pWME) ;
}

static char const* GetValueType(egSKISymbolType type)
{
	switch (type)
	{
	case gSKI_DOUBLE: return sml_Names::kTypeDouble ;
	case gSKI_INT:	  return sml_Names::kTypeInt ;
	case gSKI_STRING: return sml_Names::kTypeString ;
	case gSKI_OBJECT: return sml_Names::kTypeID ;
	default: return NULL ;
	}
}

static bool AddWmeChildrenToXML(gSKI::IWMObject* pRoot, ElementXML* pTagResult, std::list<gSKI::IWMObject*> *pTraversedList)
{
	if (!pRoot || !pTagResult)
		return false ;

	gSKI::tIWmeIterator* iter = pRoot->GetWMEs() ;

	while (iter->IsValid())
	{
		gSKI::IWme* pWME = iter->GetVal() ;

		// In some cases, wmes either haven't been added yet or have already been removed from the kernel
		// but still exist in gSKI.  In both cases, we can't (naturally) get a correct time tag for the wme
		// so I think we should skip these wmes.  That's clearly correct if the wme has been removed, but I'm
		// less sure if it's in the process of getting added.
		if (pWME->HasBeenRemoved())
		{
			pWME->Release() ;
			iter->Next() ;

			continue ;
		}

		TagWme* pTagWme = new TagWme() ;

		// Sometimes gSKI's owning object links are null -- esp. on the output side so I'm adding
		// a workaround to use the root object's ID.
		if (pWME->GetOwningObject())
			pTagWme->SetIdentifier(pWME->GetOwningObject()->GetId()->GetString()) ;
		else
			pTagWme->SetIdentifier(pRoot->GetId()->GetString()) ;

		pTagWme->SetAttribute(pWME->GetAttribute()->GetString()) ;
		pTagWme->SetValue(pWME->GetValue()->GetString(), GetValueType(pWME->GetValue()->GetType())) ;
		pTagWme->SetTimeTag(pWME->GetTimeTag()) ;
		pTagWme->SetActionAdd() ;

		// Add this wme into the result
		pTagResult->AddChild(pTagWme) ;

		// If this is an identifier then add all of its children too
		if (pWME->GetValue()->GetType() == gSKI_OBJECT)
		{
			gSKI::IWMObject* pChild = pWME->GetValue()->GetObject() ;

			// Check that we haven't already added this identifier before
			// (there can be cycles).
			if (std::find(pTraversedList->begin(), pTraversedList->end(), pChild) == pTraversedList->end())
			{
				pTraversedList->push_back(pChild) ;
				AddWmeChildrenToXML(pChild, pTagResult, pTraversedList) ;
			}
		}

		pWME->Release() ;
		iter->Next() ;
	}

	iter->Release() ;

	return true ;
}

// Send the current state of the input link back to the caller.  (This is not a commonly used method).
bool KernelSML::HandleGetAllInput(AgentSML* pAgentSML, char const* /*pCommandName*/, Connection* /*pConnection*/, AnalyzeXML* /*pIncoming*/, ElementXML* pResponse)
{
	// This is not available on the gSKI removal branch yet -- more work needed to implement it.
	assert(false) ;

	// Create the result tag
	TagResult* pTagResult = new TagResult() ;

	// Walk the list of wmes on the input link and send them over
	gSKI::IWMObject* pRootObject = NULL ;
	pAgentSML->m_inputlink->GetRootObject(&pRootObject) ;

	// We need to keep track of which identifiers we've already added
	// because this is a graph, so we may cycle back.
	std::list<gSKI::IWMObject*> traversedList ;

	// Add this wme's children to XML
	AddWmeChildrenToXML(pRootObject, pTagResult, &traversedList) ;

	// Add the result tag to the response
	pResponse->AddChild(pTagResult) ;

	if (pRootObject)
		pRootObject->Release() ;

	// Return true to indicate we've filled in all of the result tag we need
	return true ;
}

// Send the current state of the output link back to the caller.  (This is not a commonly used method).
bool KernelSML::HandleGetAllOutput(AgentSML* pAgentSML, char const* /*pCommandName*/, Connection* /*pConnection*/, AnalyzeXML* /*pIncoming*/, ElementXML* pResponse)
{
	// Build the SML message we're doing to send which in this case is an output command
	// (just like one you'd get if the agent was generating output rather than being queried for its output link)
	TagCommand* pTagResult = new TagCommand() ;
	pTagResult->SetName(sml_Names::kCommand_Output) ;

	agent* pSoarAgent = pAgentSML->GetSoarAgent() ;

	output_link *ol = pSoarAgent->existing_output_links ;	// This is technically a list but we only support one output link

	//remove_output_link_tc_info (pSoarAgent, ol);
	//calculate_output_link_tc_info (pSoarAgent, ol);
	io_wme* iw_list = get_io_wmes_for_output_link (pSoarAgent, ol);

	// Start with the output link itself
	TagWme* pOutputLinkWme = OutputListener::CreateTagWme(ol->link_wme) ;
	pTagResult->AddChild(pOutputLinkWme) ;

	for (;iw_list != 0 ; iw_list = iw_list->next) {
		// Create the wme tag for the output link itself
		TagWme* pTagWme = OutputListener::CreateTagIOWme(iw_list) ;

		// Add this wme into the result
		pTagResult->AddChild(pTagWme) ;
	}

	deallocate_io_wme_list(pSoarAgent, iw_list) ;

	// Add the message to the response
	pResponse->AddChild(pTagResult) ;

#ifdef _DEBUG
	// Set a break point in here to look at the message as a string
	char *pStr = pResponse->GenerateXMLString(true) ;
	pResponse->DeleteString(pStr) ;
#endif

	// Return true to indicate we've filled in all of the result tag we need
	return true ;
}

// Add or remove a list of wmes we've been sent
bool KernelSML::HandleInput(AgentSML* pAgentSML, char const* /*pCommandName*/, Connection* /*pConnection*/, AnalyzeXML* pIncoming, ElementXML* /*pResponse*/)
{
	// Flag to control printing debug information about the input link
#ifdef _DEBUG
	bool kDebugInput = false ;
#else
	bool kDebugInput = false ;
#endif

	if (!pAgentSML)
		return false ;

	// Record the input coming input message on a list
	pAgentSML->AddToPendingInputList(pIncoming->GetElementXMLHandle()) ;

	bool ok = true ;

	// Get the command tag which contains the list of wmes
	ElementXML const* pCommand = pIncoming->GetCommandTag() ;

	// Echo back the list of wmes received, so other clients can see what's been added (rarely used).
	pAgentSML->FireInputReceivedEvent(pCommand) ;

	if (kDebugInput)
		sml::PrintDebugFormat("--------- %s ending input ----------", pAgentSML->GetName()) ;

	// Returns false if any of the adds/removes fails
	return ok ;
}

// Executes a generic command line for a specific agent
bool KernelSML::HandleCommandLine(AgentSML* pAgentSML, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse)
{
#ifdef _DEBUG
	bool kDebugCommandLine = false ;
#else
	bool kDebugCommandLine = false ;
#endif

	// Get the parameters
	char const* pLine = pIncoming->GetArgString(sml_Names::kParamLine) ;
	bool echoResults  = pIncoming->GetArgBool(sml_Names::kParamEcho, false) ;
	bool noFiltering  = pIncoming->GetArgBool(sml_Names::kParamNoFiltering, false) ;

	// If the user chooses to enable this feature, certain commands are always echoed back.
	// This is primarily to support two users connected to and debugging the same kernel at once.
	if (GetEchoCommands() && m_CommandLineInterface.ShouldEchoCommand(pLine))
		echoResults = true ;

	bool rawOutput = false;

	// The caller can ask for simple string output (raw output) or more complex, structured XML output
	// which can then be parsed.
	ElementXML const* pCommand = pIncoming->GetCommandTag() ;
	const char* pCommandOutput = pCommand->GetAttribute(sml_Names::kCommandOutput) ;

	if (pCommandOutput)
		rawOutput = (strcmp(pCommandOutput, sml_Names::kRawOutput) == 0) ;

	if (!pLine)
	{
		return InvalidArg(pConnection, pResponse, pCommandName, "Command line missing") ;
	}

	if (kDebugCommandLine)
		sml::PrintDebugFormat("Executing %s", pLine) ;
	
	// If we're echoing the results, also echo the command we're executing
	if (echoResults && pAgentSML)
		pAgentSML->FireEchoEvent(pConnection, pLine) ;

	if (kDebugCommandLine)
		sml::PrintDebugFormat("Echoed line\n") ;

	// Send this command line through anyone registered filters.
	// If there are no filters (or this command requested not to be filtered), this copies the original line into the filtered line unchanged.
	char const* pFilteredLine   = pLine ;
	bool filteredError = false ;
	ElementXML* pFilteredXML = NULL ;

	if (!noFiltering && HasFilterRegistered())
	{
		// Expand any aliases before passing the command to the filter.
		// It's possible this is a mistake because a really powerful filter might want to do
		// something with the original, unaliased form (and could call this expansion itself) but
		// it seems this will be correct in almost all cases, so let's start with this assumption and
		// wait until it's proved incorrect.
		std::string expandedLine ;
		if (m_CommandLineInterface.ExpandCommandToString(pLine, &expandedLine))
			pLine = expandedLine.c_str() ;

		// We'll send the command over as an XML packet, so there's some structure to work with.
		// The current structure is:
		// <filter command="command" output="generated output" error="true | false"></filter>
		// Each filter is passed this string and can modify it as they go.
		// All attributes are optional although either command or output & error should exist.
		// It's possible, although unlikely that all 3 could exist at once (i.e. another command to execute, yet still have output already)
		TagFilter filterXML ;
		filterXML.SetCommand(pLine) ;

		char* pXMLString = filterXML.GenerateXMLString(true) ;

		std::string filteredXML ;
		bool filtered = this->SendFilterMessage(pAgentSML, pXMLString, &filteredXML) ;

		// Clean up the XML message
		filterXML.DeleteString(pXMLString) ;

		// If a filter consumed the entire command, there's no more work for us to do.
		if (filteredXML.empty())
			return true ;

		if (filtered)
		{
			pFilteredXML = ElementXML::ParseXMLFromString(filteredXML.c_str()) ;
			if (!pFilteredXML)
			{
				// Error parsing the XML that the filter returned
				return false ;
			}

			// Get the results of the filtering
			pFilteredLine    = pFilteredXML->GetAttribute(sml_Names::kFilterCommand) ;
			char const* pFilteredOutput  = pFilteredXML->GetAttribute(sml_Names::kFilterOutput) ;
			char const* pErr = pFilteredXML->GetAttribute(sml_Names::kFilterError) ;
			filteredError    = (pErr && strcasecmp(pErr, "true") == 0) ;

			// See if the filter consumed the command.  If so, we just need to return the output.
			if (!pFilteredLine || strlen(pFilteredLine) == 0)
			{
				// We may have no output defined and that's not an error so cover that case
				if (pFilteredOutput == NULL)
					pFilteredOutput = "" ;

            if(filteredError)
            {
               pConnection->AddErrorToSMLResponse(pResponse, pFilteredOutput, -1) ;
            }
				bool res = this->ReturnResult(pConnection, pResponse, pFilteredOutput) ;

				// Can only clean this up after we're finished using it or pFilteredLine will become invalid
				delete pFilteredXML ;

				return res ;
			}
		}
	}

	if (kDebugCommandLine)
		sml::PrintDebugFormat("Filtered line is %s\n", pFilteredLine) ;

	// Make the call.
	m_CommandLineInterface.SetRawOutput(rawOutput);
	bool result = m_CommandLineInterface.DoCommand(pConnection, pAgentSML, pFilteredLine, echoResults, pResponse) ;

	if (kDebugCommandLine)
		sml::PrintDebugFormat("Completed %s", pLine) ;

	// Can only clean this up after we're finished using it or pFilteredLine will become invalid
	delete pFilteredXML ;

	return result ;
}

// Expands a command line's aliases and returns it without executing it.
bool KernelSML::HandleExpandCommandLine(AgentSML* /*pAgentSML*/, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse)
{
	// Get the parameters
	char const* pLine = pIncoming->GetArgString(sml_Names::kParamLine) ;

	if (!pLine)
	{
		return InvalidArg(pConnection, pResponse, pCommandName, "Command line missing") ;
	}

	// Make the call.
	return m_CommandLineInterface.ExpandCommand(pConnection, pLine, pResponse) ;
}

