/////////////////////////////////////////////////////////////////
// Connection class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// This class represents a logical connection between two entities that are communicating via SML.
// For example, an environment (the client) and the Soar kernel.
//
// The connection can be "embedded" which means both the client and the kernel are in the same process
// or it can be "remote" which means the client and the kernel are in different processes and possibly on different machines.
//
// Commands formatted as SML (a form of XML) are sent over this connection to issue commands etc.
//
/////////////////////////////////////////////////////////////////

#ifndef SML_CONNECTION_H
#define SML_CONNECTION_H

// A null pointer
#ifndef NULL
#define NULL 0
#endif

// The end of a null terminated string
#ifndef NUL
#define NUL 0
#endif

#include <string>
#include <list>
#include <map>

#ifndef unused
#define unused(x) (void)(x)
#endif

#include "sml_Errors.h"
#include "sml_MessageGenerator.h"

// These last ones are just for convenience, they could come out
#include "sml_ElementXML.h"
#include "sml_AnalyzeXML.h"
#include "sml_Names.h"

namespace sml
{

// Forward declarations
class ListenerConnection ;
class Connection ;
class ElementXML ;
class AnalyzeXML ;

// Define the ListenerCallback to pass the new connection and some user data.
typedef void (*ListenerCallback)(Connection*, void*);

// Define the IncomingCallback to pass the existing connection, incoming message
// and some user data.  The result can be NULL or a message that is sent back over the connection.
typedef ElementXML* (*IncomingCallback)(Connection*, ElementXML*, void*) ;

/*************************************************************
* @brief The Callback class is a simple wrapper around a callback.
*        We use it to keep together the callback function pointer and the
*        user data we'll pass to the function.
*************************************************************/
class Callback
{
protected:
	Connection*			m_pConnection ;
	IncomingCallback	m_pCallback ;
	void*				m_pUserData ;

public:
	/*************************************************************
	* @brief Accessors
	*************************************************************/
	IncomingCallback getFunction() { return m_pCallback ; }

	/*************************************************************
	* @brief Constructor
	*************************************************************/
	Callback(Connection* pConnection, IncomingCallback pFunc, void* pUserData)
	{
		m_pConnection	= pConnection ;
		m_pCallback		= pFunc ;
		m_pUserData		= pUserData ; 
	}
	
	/*************************************************************
	* @brief Invoke this callback, passing the message,
	*		 the connection and the user's data (which can be anything).
	* 
	* @returns NULL or a response to this message.
	*************************************************************/
	ElementXML* Invoke(ElementXML* pIncomingMessage)
	{
		ElementXML* pResult = (*m_pCallback)(m_pConnection, pIncomingMessage, m_pUserData) ;
		return pResult ;
	}
} ;

// Used to store a list of callback objects
typedef std::list<Callback*>	CallbackList ;
typedef CallbackList::iterator	CallbackListIter ;

// We need a comparator to make the map we're about to define work with char*
struct strCompareConnection
{
  bool operator()(const char* s1, const char* s2) const
  {
    return std::strcmp(s1, s2) < 0;
  }
};

// Used to store a map from type (a string) to list of callbacks
typedef std::map<char const*, CallbackList*, strCompareConnection>	CallbackMap ;
typedef CallbackMap::iterator										CallbackMapIter ;

/*************************************************************
* @brief The Connection class represents a logical link
*		 between two entities that are communicating through
*		 SML messages.
*************************************************************/
class Connection : public MessageGenerator
{
protected:
	// Maps from SML document types (e.g. "call") to a list of functions to call when that type of message is received.
	CallbackMap		m_CallbackMap ;

	// The client or kernel may wish to keep state information (e.g. a pointer into gSKI for the kernel) with the
	// connection.  If so, it can store it here.  The caller retains ownership of this object, so it won't be
	// deleted when the connection is deleted.
	void*			m_pUserData ;

public:
	Connection() ;
	virtual ~Connection() ;

	/*************************************************************
	* @brief Creates a connection to a receiver that is embedded
	*        within the same process.
	*
	* @param pLibraryName	The name of the library to load, without an extension (e.g. "ClientSML" or "KernelSML").  Case-sensitive (to support Linux).
	*						This library will be dynamically loaded and connected to.
	* @param pError			Pass in a pointer to an int and receive back an error code if there is a problem.
	* @returns An EmbeddedConnection instance.
	*************************************************************/
	static Connection* CreateEmbeddedConnection(char const* pLibraryName, ErrorCode* pError) ;

	/*************************************************************
	* @brief Creates a connection to a receiver that is in a different
	*        process.  The process can be on the same machine or a different machine.
	*
	* @param pIPaddress The IP address of the remote machine (e.g. "202.55.12.54").
	*                   Pass "127.0.0.1" to create a connection between two processes on the same machine.
	* @param port		The port number to connect to.  The default port for SML is 35353 (picked at random).
	* @param pError		Pass in a pointer to an int and receive back an error code if there is a problem.  (Can pass NULL).
	*
	* @returns A RemoteConnection instance.
	*************************************************************/
	static Connection* CreateRemoteConnection(char const* pIPaddress, int port, ErrorCode* pError) ;

	/*************************************************************
	* @brief Starts listening for incoming connections on a particular port.
	*        The callback function passed in is called once a connection has been made.
	*
	* @param pIPaddress The IP address of the remote machine (e.g. "202.55.12.54").
	*                   Pass "127.0.0.1" to create a connection between two processes on the same machine.
	* @param port		The port number to connect to.  The default port for SML is 35353 (picked at random).
	* @param callback	This function will be called when the connection is made and is passed the new connection and the user data
	* @param pUserData	This data is passed to the callback.  It allows the callback to have some context to work in.  Can be NULL.
	* @param pError		Pass in a pointer to an int and receive back an error code if there is a problem.  (Can pass NULL).
	*
	* @returns A ListenerConnection instance.
	*************************************************************/
	static ListenerConnection* CreateListener(int port, ListenerCallback callback, void* pUserData, ErrorCode* pError) ;

	/*************************************************************
	* @brief Shuts down this connection.
	*************************************************************/
	virtual void CloseConnection() = 0 ;

	/*************************************************************
	* @brief Returns true if this connection has been closed or
	*		 is otherwise not usable.
	*************************************************************/
	virtual bool IsClosed() = 0 ;

	/*************************************************************
	* @brief Send a message to the SML receiver (e.g. from the environment to the Soar kernel).
	*        The error code that is returned indicates whether the command was successfully sent,
	*		 not whether the command was interpreted successfully by Soar.
	*
	* @param pMsg	The message (as an object representing XML) that is to be sent.
	*				The caller should release this message object after making the send call
	*               once it if finished using it.
	*************************************************************/
	virtual void SendMessage(ElementXML* pMsg) = 0 ;

	/*************************************************************
	* @brief Retrieve any commands, notifications, responses etc. that are waiting.
	*		 Messages that are received are routed to callback functions in the client for processing.
	*
	*		 This call never blocks.  In an embedded situation, this does nothing as incoming messages are
	*        sent directly to the callback functions.
	*        In a remote situation, the client must call this function periodically, to read incoming messages
	*		 on the socket.
	*
	*        We use a callback model (rather than retrieving each message in turn here) so that the embedded model and
	*		 the remote model are closer to each other.
	*
	* @param allMessages	If false, only retrieves at most one message before returning, otherwise gets all messages.
	*************************************************************/
	virtual void ReceiveMessages(bool allMessages) = 0 ;

	/*************************************************************
	* @brief Retrieve the response to the last call message sent.
	*
	*		 In an embedded situation, this result is always immediately available and the "wait" parameter is ignored.
	*		 In a remote situation, if wait is false and the result is not immediately available this call returns false.
	*
	*		 The ID is only required when the client is remote (because then there might be many responses waiting on the socket).
	*		 A message can only be retrieved once, so a second call with the same ID will return NULL.
	*		 Only the response to the last call message can be retrieved.
	*
	*		 The client is not required to call to get the result of a command it has sent.
	*
	*		 The implementation of this function will call ReceiveMessages() to get messages one at a time and process them.  Thus callbacks may be
	*		 invoked while the client is blocked waiting for the particular response they requested.
	*
	*		 A response that is returned to the client through GetResultOfMessage() will not be passed to a callback
	*		 function registered for response messages.  This allows a client to register a general function to check for
	*		 any error messages and yet retrieve specific responses to calls that it is particularly interested in.
	*
	* @param pID	The id of the original SML message (the id is a attribute in the top level [sml] tag)
	* @param wait	If true wait until the result is received (or we time out and report an error).
	*
	* @returns The message that is a response to pID or NULL if none is found.
	*************************************************************/
	virtual ElementXML* GetResponseForID(char const* pID, bool wait) = 0 ;

	/*************************************************************
	* @brief Retrieve the response to the last call message sent.
	*
	*		 In an embedded situation, this result is always immediately available and the "wait" parameter is ignored.
	*		 In a remote situation, if wait is false and the result is not immediately available this call returns false.
	*
	*		 The message is only required when the client is remote (because then there might be many responses waiting on the socket).
	*		 A message can only be retrieved once, so a second call with the same ID will return NULL.
	*		 Only the response to the last call message can be retrieved.
	*
	*		 The client is not required to call to get the result of a command it has sent.
	*
	*		 The implementation of this function will call ReceiveMessages() to get messages one at a time and process them.  Thus callbacks may be
	*		 invoked while the client is blocked waiting for the particular response they requested.
	*
	*		 A response that is returned to the client through GetResultOfMessage() will not be passed to a callback
	*		 function registered for response messages.  This allows a client to register a general function to check for
	*		 any error messages and yet retrieve specific responses to calls that it is particularly interested in.
	*
	* @param pMsg	The original SML message that we wish to get a response from.
	* @param wait	If true wait until the result is received (or we time out and report an error).
	*
	* @returns The message that is a response to pMsg or NULL if none is found.
	*************************************************************/
	virtual ElementXML* GetResponse(ElementXML const* pMsg, bool wait = true) ;

	/*************************************************************
	* @brief Register a callback for a particular type of incoming message.
	*
	*		 Messages are currently one of:
	*		 "call", "response" or "notify"
	*		 A call is always paired to a response (think of this as a remote function call that returns a value)
	*		 while a notify does not receive a response (think of this as a remote function call that does not return a value).
	*		 This type is stored in the "doctype" attribute of the top level SML node in the message.
	*		 NOTE: doctype's are case sensitive.
	*
	*		 You MUST register a callback for the "call" type of message.  This callback must return a "response" message which is then
	*		 sent back over the connection.  Other callbacks should not return a message.
	*		 Once the returned message has been sent it will be deleted.
	*
	*		 We will maintain a list of callbacks for a given type of SML document and call each in turn.
	*		 Each callback on the list will be called in turn until one returns a non-NULL response.  No further callbacks
	*		 will be called for that message.  This ensures that only one response is sent to a message.
	*
	* @param callback	The function to call when an incoming message is received (of the right type)
	* @param pUserData	This data is passed to the callback.  It allows the callback to have some context to work in.  Can be NULL.
	* @param pType		The type of message to register for (currently one of "call", "response" or "notify").
	* @param addToEnd	If true add the callback to the end of the list (called last).  If false, add to front where it will be called first.
	*************************************************************/
	virtual void RegisterCallback(IncomingCallback callback, void* pUserData, char const* pType, bool addToEnd) ;

	/*************************************************************
	* @brief Removes a callback from the list of callbacks for a particular type of incoming message.
	*
	* @param callback	The function that was previously registered.  If NULL removes all callbacks for this type of message.
	* @param pType		The type of message to unregister from (currently one of "call", "response" or "notify").
	*************************************************************/
	virtual void UnregisterCallback(IncomingCallback callback, char const* pType) ;

	/*************************************************************
	* @brief Invoke the list of callbacks matching the doctype of the incoming message.
	*
	* @param pIncomingMsg	The SML message that should be passed to the callbacks.
	*
	* @returns The response message (or NULL if there is no response from any calback).
	*************************************************************/
	virtual ElementXML* InvokeCallbacks(ElementXML *pIncomingMsg) ;

	/*************************************************************
	* @brief Get and set the user data.
	*
	* The client or kernel may wish to keep state information (e.g. a pointer into gSKI for the kernel) with the
	* connection.  If so, it can store it here.  The caller retains ownership of this object, so it won't be
	* deleted when the connection is deleted.
	*************************************************************/
	void	SetUserData(void* pUserData)	{ m_pUserData = pUserData ; }
	void*	GetUserData()					{ return m_pUserData ; }

	/*************************************************************
	* @brief Send a message and get the response.
	*
	* @param pAnalysis	This will be filled in with the analyzed response
	* @param pMsg		The message to send
	* @returns			True if got a reply
	*************************************************************/
	bool SendMessageGetResponse(AnalyzeXML* pAnalysis, ElementXML* pMsg) ;

	/*************************************************************
	* @brief Build an SML message and send it over the connection
	*		 returning the analyzed version of the response.
	*
	* This family of commands are designed for access based on
	* a named agent.  This agent's name is passed as the first
	* parameter and then the other parameters define the details
	* of which method to call for the agent.
	* 
	* Passing NULL for the agent name is valid and indicates
	* that the command is not agent specific (e.g. "shutdown-kernel"
	* would pass NULL).
	*
	* Uses SendMessageGetResponse() to do its work.
	*
	* @param pResponse		The response from the kernel to this command.
	* @param pCommandName	The command to execute
	* @param pAgentName		The name of the agent this command is going to (can be NULL -> implies going to top level of kernel)
	* @param pParamName1	The name of the first argument for this command
	* @param pParamVal1		The value of the first argument for this command
	* @param rawOuput		If true, sends back a simple string form for the result which the caller will probably just print.
	*						If false, sendds back a structured XML object that the caller can analyze and do more with.
	* @returns	True if command was sent and received back without any errors (either in sending or in executing the command).
	*************************************************************/
	bool SendAgentCommand(AnalyzeXML* pResponse, char const* pCommandName, bool rawOutput = false) ;

	bool SendAgentCommand(AnalyzeXML* pResponse, char const* pCommandName, char const* pAgentName, bool rawOutput = false) ;

	bool SendAgentCommand(AnalyzeXML* pResponse, char const* pCommandName, char const* pAgentName,
					 char const* pParamName1, char const* pParamVal1, bool rawOutput = false) ;

	bool SendAgentCommand(AnalyzeXML* pResponse, char const* pCommandName, char const* pAgentName,
					 char const* pParamName1, char const* pParamVal1,
					 char const* pParamName2, char const* pParamVal2, bool rawOutput = false) ;

	bool SendAgentCommand(AnalyzeXML* pResponse, char const* pCommandName, char const* pAgentName,
					 char const* pParamName1, char const* pParamVal1,
					 char const* pParamName2, char const* pParamVal2,
					 char const* pParamName3, char const* pParamVal3, bool rawOutput = false) ;

	/*************************************************************
	* @brief Build an SML message and send it over the connection
	*		 returning the analyzed version of the response.
	*
	* This family of commands are designed for an object model access
	* to the kernel (e.g. using the gSKI interfaces).
	* In this model, the first parameter is always an indentifier
	* representing the "this" pointer.  The name of the command gives
	* the method name (in some manner) and the other parameters
	* define the arguments to the method.
	*
	* As of this writing, we are largely moving away from this model,
	* but the code is still here in case it has value in the future.
	*
	* Uses SendMessageGetResponse() to do its work.
	*************************************************************/
	bool SendClassCommand(AnalyzeXML* pResponse, char const* pCommandName) ;

	bool SendClassCommand(AnalyzeXML* pResponse, char const* pCommandName, char const* pThisID) ;

	bool SendClassCommand(AnalyzeXML* pResponse, char const* pCommandName, char const* pThisID,
					 char const* pParamName1, char const* pParamVal1) ;

	bool SendClassCommand(AnalyzeXML* pResponse, char const* pCommandName, char const* pThisID,
					 char const* pParamName1, char const* pParamVal1,
					 char const* pParamName2, char const* pParamVal2) ;

	bool SendClassCommand(AnalyzeXML* pResponse, char const* pCommandName, char const* pThisID,
					 char const* pParamName1, char const* pParamVal1,
					 char const* pParamName2, char const* pParamVal2,
					 char const* pParamName3, char const* pParamVal3) ;

protected:
	/*************************************************************
	* @brief Gets the list of callbacks associated with a given doctype (e.g. "call")
	**************************************************************/
	virtual CallbackList* Connection::GetCallbackList(char const* pType) ;

};

} // End of namespace

#endif // SML_CONNECTION_H
