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
#ifndef SML_AGENT_H
#define SML_AGENT_H

#include "sml_ClientWorkingMemory.h"
#include "sml_ClientEvents.h"
#include "sml_ClientErrors.h"
#include "sml_ListMap.h"

#include <string>
#include <map>
#include <list>
#include <utility>	// For std::pair

namespace sml {

class Kernel ;
class Connection ;
class AnalyzeXML ;
class ElementXML ;

// We'll store a handler function together with a generic pointer to data of the user's choosing
// (which is then passed back into the handler when the event occurs).
// We also include a callback "id" which is a unique way to refer to this callback--used during unregistering.
struct EventHandlerPlusData
{
	void*			m_UserData ;
	int				m_CallbackID ;
} ;

struct RunEventHandlerPlusData : public EventHandlerPlusData
{
	RunEventHandler m_Handler ;

	RunEventHandlerPlusData(RunEventHandler handler, void* userData, int callbackID)
	{
		m_Handler = handler ;
		m_UserData = userData ;
		m_CallbackID = callbackID ;
	}
} ;

struct ProductionEventHandlerPlusData : public EventHandlerPlusData
{
	ProductionEventHandler m_Handler ;

	ProductionEventHandlerPlusData(ProductionEventHandler handler, void* userData, int callbackID)
	{
		m_Handler = handler ;
		m_UserData = userData ;
		m_CallbackID = callbackID ;
	}
} ;

struct AgentEventHandlerPlusData : public EventHandlerPlusData
{
	AgentEventHandler m_Handler ;

	AgentEventHandlerPlusData(AgentEventHandler handler, void* userData, int callbackID)
	{
		m_Handler = handler ;
		m_UserData = userData ;
		m_CallbackID = callbackID ;
	}
} ;

struct PrintEventHandlerPlusData : public EventHandlerPlusData
{
	PrintEventHandler m_Handler ;

	PrintEventHandlerPlusData(PrintEventHandler handler, void* userData, int callbackID)
	{
		m_Handler = handler ;
		m_UserData = userData ;
		m_CallbackID = callbackID ;
	}
} ;


class Agent : public ClientErrors
{
	// Don't want users creating and destroying agent objects without
	// going through the kernel
	friend class Kernel ;
	friend class WorkingMemory ;
	friend class ObjectMap<Agent*> ;	// So can delete agent
	friend class WMElement ;
	friend class StringElement ;
	friend class IntElement ;
	friend class FloatElement ;
	friend class Identifier ;

protected:
	// The mapping from event number to a list of handlers to call when that event fires
	typedef sml::ListMap<smlRunEventId, RunEventHandlerPlusData>				RunEventMap ;
	typedef sml::ListMap<smlProductionEventId, ProductionEventHandlerPlusData>	ProductionEventMap ;
	typedef sml::ListMap<smlAgentEventId, AgentEventHandlerPlusData>			AgentEventMap ;
	typedef sml::ListMap<smlPrintEventId, PrintEventHandlerPlusData>			PrintEventMap ;

protected:
	// We maintain a local copy of working memory so we can just send changes
	WorkingMemory	m_WorkingMemory ;

	// The kernel that owns this agent
	Kernel*			m_Kernel ;

	// The name of this agent
	std::string		m_Name ;

	// Map from event id to handler function(s)
	RunEventMap			m_RunEventMap ;
	ProductionEventMap	m_ProductionEventMap ;
	AgentEventMap		m_AgentEventMap ;
	PrintEventMap		m_PrintEventMap ;

	// Used to generate unique IDs for callbacks
	int		m_CallbackIDCounter ;

protected:
	Agent(Kernel* pKernel, char const* pAgentName);

	// This is protected so the client doesn't try to delete it.
	// Client should just delete the kernel object (or call Kernel::DestroyAgent() if just want to destroy this agent).
	virtual ~Agent();

	Connection* GetConnection() const ;
	WorkingMemory* GetWM() 				{ return &m_WorkingMemory ; } 

	/*************************************************************
	* @brief This function is called when output is received
	*		 from the Soar kernel.
	*
	* @param pIncoming	The output command (list of wmes added/removed from output link)
	* @param pResponse	The reply (no real need to fill anything in here currently)
	*************************************************************/
	void ReceivedOutput(AnalyzeXML* pIncoming, ElementXML* pResponse) ;

	/*************************************************************
	* @brief This function is called when an event is received
	*		 from the Soar kernel.
	*
	* @param pIncoming	The event command
	* @param pResponse	The reply (no real need to fill anything in here currently)
	*************************************************************/
	void ReceivedEvent(AnalyzeXML* pIncoming, ElementXML* pResponse) ;
	void ReceivedRunEvent(smlRunEventId id, AnalyzeXML* pIncoming, ElementXML* pResponse) ;
	void ReceivedProductionEvent(smlProductionEventId id, AnalyzeXML* pIncoming, ElementXML* pResponse) ;
	void ReceivedAgentEvent(smlAgentEventId id, AnalyzeXML* pIncoming, ElementXML* pResponse) ;
	void ReceivedPrintEvent(smlPrintEventId id, AnalyzeXML* pIncoming, ElementXML* pResponse) ;

	/*************************************************************
	* @brief Register for a particular event with the kernel.
	*		 (This is a primitive function, should call one of the
	*		  higher level methods which will call here if needed)
	*************************************************************/
	void	RegisterForEvent(int id) ;

	/*************************************************************
	* @brief Unregister for a particular event with the kernel.
	*		 (This is a primitive function, should call one of the
	*		  higher level methods which will call here if needed)
	*************************************************************/
	void	UnregisterForEvent(int id) ;

public:
	/*************************************************************
	* @brief Returns this agent's name.
	*************************************************************/
	char const* GetAgentName() const	{ return m_Name.c_str() ; }

	/*************************************************************
	* @brief Returns a pointer to the kernel object that owns this Agent.
	*************************************************************/
	Kernel*		GetKernel() const		{ return m_Kernel ; }

	/*************************************************************
	* @brief Load a set of productions from a file.
	*
	* The file must currently be on a filesystem that the kernel can
	* access (i.e. can't send to a remote PC unless that PC can load
	* this file).
	*
	* @returns True if finds file to load successfully.
	*************************************************************/
	bool LoadProductions(char const* pFilename) ;

	/*************************************************************
	* @brief Returns the id object for the input link.
	*		 The agent retains ownership of this object.
	*************************************************************/
	Identifier* GetInputLink() ;

	/*************************************************************
	* @brief An alternative that matches the older SGIO interface.
	*************************************************************/
	Identifier* GetILink() { return GetInputLink() ; }

	/*************************************************************
	* @brief Returns the id object for the output link.
	*		 The agent retains ownership of this object.
	*************************************************************/
	Identifier* GetOutputLink() ;

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
	Identifier*		FindIdentifier(char const* pID, bool searchInput, bool searchOutput, int index = 0) ;

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
	StringElement* CreateStringWME(Identifier* parent, char const* pAttribute, char const* pValue);

	/*************************************************************
	* @brief Same as CreateStringWME but for a new WME that has
	*		 an int as its value.
	*************************************************************/
	IntElement* CreateIntWME(Identifier* parent, char const* pAttribute, int value) ;

	/*************************************************************
	* @brief Same as CreateStringWME but for a new WME that has
	*		 a floating point value.
	*************************************************************/
	FloatElement* CreateFloatWME(Identifier* parent, char const* pAttribute, double value) ;

	/*************************************************************
	* @brief Same as CreateStringWME but for a new WME that has
	*		 an identifier as its value.
	*		 The identifier value is generated here and will be
	*		 different from the value Soar assigns in the kernel.
	*		 The kernel will keep a map for translating back and forth.
	*************************************************************/
	Identifier*		CreateIdWME(Identifier* parent, char const* pAttribute) ;

	/*************************************************************
	* @brief Creates a new WME that has an identifier as its value.
	*		 The value in this case is the same as an existing identifier.
	*		 This allows us to create a graph rather than a tree.
	*************************************************************/
	Identifier*		CreateSharedIdWME(Identifier* parent, char const* pAttribute, Identifier* pSharedValue) ;

	/*************************************************************
	* @brief Update the value of an existing WME.
	*		 The value is not actually sent to the kernel
	*		 until "Commit" is called.
	*************************************************************/
	void	Update(StringElement* pWME, char const* pValue) ;
	void	Update(IntElement* pWME, int value) ;
	void	Update(FloatElement* pWME, double value) ;

	/*************************************************************
	* @brief Schedules a WME from deletion from the input link and removes
	*		 it from the client's model of working memory.
	*
	*		 The caller should not access this WME after calling
	*		 DestroyWME().
	*		 The WME is not removed from the input link until
	*		 the client calls "Commit"
	*************************************************************/
	bool	DestroyWME(WMElement* pWME) ;

	/*************************************************************
	* @brief Reinitialize this Soar agent.
	*		 This will also cause the output link structures stored
	*		 here to be erased and the current input link to be sent over
	*		 to the Soar agent for the start of its next run.
	*************************************************************/
	char const*	InitSoar() ;

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
	*************************************************************/
	char const*	Stop(bool stopAllAgents = true) ;

	/*************************************************************
	* @brief Register for a "RunEvent".
	*		 Multiple handlers can be registered for the same event.
	* @param smlEventId		The event we're interested in (see the list below for valid values)
	* @param handler		A function that will be called when the event happens
	* @param pUserData		Arbitrary data that will be passed back to the handler function when the event happens.
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
	int	RegisterForRunEvent(smlRunEventId id, RunEventHandler handler, void* pUserData) ;

	/*************************************************************
	* @brief Unregister for a particular event
	*************************************************************/
	void	UnregisterForRunEvent(smlRunEventId id, int callbackID) ;

	/*************************************************************
	* @brief Register for a "ProductionEvent".
	*		 Multiple handlers can be registered for the same event.
	* @param smlEventId		The event we're interested in (see the list below for valid values)
	* @param handler		A function that will be called when the event happens
	* @param pUserData		Arbitrary data that will be passed back to the handler function when the event happens.
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
	int	RegisterForProductionEvent(smlProductionEventId id, ProductionEventHandler handler, void* pUserData) ;

	/*************************************************************
	* @brief Unregister for a particular event
	*************************************************************/
	void	UnregisterForProductionEvent(smlProductionEventId id, int callbackID) ;

	/*************************************************************
	* @brief Register for an "AgentEvent".
	*		 Multiple handlers can be registered for the same event.
	* @param smlEventId		The event we're interested in (see the list below for valid values)
	* @param handler		A function that will be called when the event happens
	* @param pUserData		Arbitrary data that will be passed back to the handler function when the event happens.
	*
	* Current set is:
	* // Agent manager
	* smlEVENT_AFTER_AGENT_CREATED,
	* smlEVENT_BEFORE_AGENT_DESTROYED,
	* smlEVENT_BEFORE_AGENT_REINITIALIZED,
	* smlEVENT_AFTER_AGENT_REINITIALIZED,
	*
	* @returns A unique ID for this callback (used to unregister the callback later) 
	*************************************************************/
	int	RegisterForAgentEvent(smlAgentEventId id, AgentEventHandler handler, void* pUserData) ;

	/*************************************************************
	* @brief Unregister for a particular event
	*************************************************************/
	void	UnregisterForAgentEvent(smlAgentEventId id, int callbackID) ;

	/*************************************************************
	* @brief Register for an "PrintEvent".
	*		 Multiple handlers can be registered for the same event.
	* @param smlEventId		The event we're interested in (see the list below for valid values)
	* @param handler		A function that will be called when the event happens
	* @param pUserData		Arbitrary data that will be passed back to the handler function when the event happens.
	*
	* Current set is:
	* // Agent manager
	* smlEVENT_PRINT
	*
	* @returns A unique ID for this callback (used to unregister the callback later) 
	*************************************************************/
	int	RegisterForPrintEvent(smlPrintEventId id, PrintEventHandler handler, void* pUserData) ;

	/*************************************************************
	* @brief Unregister for a particular event
	*************************************************************/
	void	UnregisterForPrintEvent(smlPrintEventId id, int callbackID) ;

	/*==============================================================================
	===
	=== There are a number of different ways to read information from
	=== the output link.  Choose whichever method seems easiest to you.
	===
	=== Method 1: a) Call "RunTilOutput".
	===			b) Call "Commands", "GetCommand" and "GetParamValue"
	===			   to get top level WMEs that have been added since the last cycle.
	===
	=== Method 2: a) Call "RunTilOutput".
	===			b) Call "GetOutputLink" and "GetNumberChildren", "GetChild"
	===			   to walk the tree and examine its current state.
	===			c) You can use "IsJustAdded" and "AreChildrenModified"
	===			   to see what WMEs just changed.
	===
	=== Method 3: a) Call "RunTilOutput".
	===			b) Call "GetNumberOutputLinkChanges" and "GetOutputLinkChange"
	===			   and "IsOutputLinkChangeAdd" to get the list of
	===			   all WMEs added and removed since the last cycle.
	===
	=== Method 1 is the closest to the original SGIO and should be sufficient
	=== in almost all cases.  However, Methods 2 & 3 provide complete
	=== access to the output-link, while Method 1 only allows access to
	=== top level wmes with this format: (I1 ^output-link I3) (I3 ^move M3) (M3 ^position 10).
	=== i.e. All commands are added as identifiers at the top level.
	=== 
	==============================================================================*/

	/*************************************************************
	* @brief Get number of changes to output link.
	*        (This is since last call to "ClearOuputLinkChanges").
	*************************************************************/
	int		GetNumberOutputLinkChanges() ;

	/*************************************************************
	* @brief Get the n-th wme added or deleted to output link
	*        (This is since last call to "ClearOuputLinkChanges").
	*************************************************************/
	WMElement*	GetOutputLinkChange(int index) ;

	/*************************************************************
	* @brief Returns true if the n-th wme change to the output-link
	*		 was a wme being added.  (false => it was a wme being deleted).
	*        (This is since last call to "ClearOuputLinkChanges").
	*************************************************************/
	bool		IsOutputLinkChangeAdd(int index) ;

	/*************************************************************
	* @brief Clear the current list of changes to the output-link.
	*		 You should call this after processing the list of changes.
	*************************************************************/
	void	ClearOutputLinkChanges() ;

	/*************************************************************
	* @brief Get the number of "commands".  A command in this context
	*		 is an identifier wme that have been added to the top level of
	*		 the output-link since the last call to "ClearOutputLinkChanges".
	*
	*		 NOTE: This function may involve searching a list so it's
	*		 best to not call it repeatedly.
	*		 
	*************************************************************/
	int		GetNumberCommands() ;

	/*************************************************************
	* @brief Returns true if there are "commands" available.
	*		 A command in this context
	*		 is an identifier wme that have been added to the top level of
	*		 the output-link since the last call to "ClearOutputLinkChanges".
	*
	*		 NOTE: This function may involve searching a list so it's
	*		 best to not call it repeatedly.
	*		 
	*************************************************************/
	bool	Commands() { return GetNumberCommands() > 0 ; }

	/*************************************************************
	* @brief Get the n-th "command".  A command in this context
	*		 is an identifier wme that have been added to the top level of
	*		 the output-link since the last call to "ClearOutputLinkChanges".
	*
	*		 Returns NULL if index is out of range.
	*
	* @param index	The 0-based index for which command to get.
	*************************************************************/
	Identifier* GetCommand(int index) ;

	/*************************************************************
	* @brief Send the most recent list of changes to working memory
	*		 over to the kernel.
	*************************************************************/
	bool Commit() ;

	/*************************************************************
	* @brief   Run Soar for the specified number of decisions
	*
	* This command will currently run all agents, even though it's part of the
	* Agent class here.
	*
	* @returns The result of executing the start of the run command.
	*		   The output from during the run is sent to a different callback.
	*************************************************************/
	char const* Run(unsigned long decisions) ;

	/*************************************************************
	* @brief   Controls whether Soar will break when it next generates
	*		   output while running.
	*
	* @param state	If true, causes Soar to break on output.  If false, Soar will not break.
	*************************************************************/
	bool SetStopOnOutput(bool state) ;

	/*************************************************************
	* @brief   Run Soar until either output is generated or
	*		   the maximum number of decisions is reached.
	*
	* This function also calls "ClearOutputLinkChanges" so methods
	* like "IsJustAdded" will refer to the changes that occur as a result of
	* this run.
	*
	* This function also calls "Commit" to make sure any pending input
	* link changes have been sent to Soar.
	*
	* We don't generally want Soar to just run until it generates
	* output without any limit as an error in the AI logic might cause
	* it to never return control to the environment.
	*
	* @param maxDecisions	If Soar runs for this many decisions without generating output, stop.
	*						15 was used in SGIO.
	*************************************************************/
	char const* RunTilOutput(unsigned long maxDecisions) ;

	/*************************************************************
	* @brief Resend the complete input link to the kernel
	*		 and remove our output link structures.
	*		 We do this when the user issues an "init-soar" event.
	*		 There should be no reason for the client to call this method directly.
	*************************************************************/
	void Refresh() ;

	/*************************************************************
	* @brief Process a command line command and return the result
	*        as a string.
	*
	* @param pCommandLine Command line string to process.
	* @param pAgentName Agent name to apply the command line to.
	* @returns The string form of output from the command.
	*************************************************************/
	char const* ExecuteCommandLine(char const* pCommandLine) ;

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
	bool ExecuteCommandLineXML(char const* pCommandLine, AnalyzeXML* pResponse) ;

	/*************************************************************
	* @brief Get last command line result
	*
	* (This is the last result for any command sent to the kernel,
	*  not just for this agent).
	*
	* @returns True if the last command line call succeeded.
	*************************************************************/
	bool GetLastCommandLineResult();
};

}//closes namespace

#endif //SML_AGENT_H
