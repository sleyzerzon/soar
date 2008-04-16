#include "portability.h"

#include "handlers.h"

#include <cppunit/extensions/HelperMacros.h>

#include <vector>
#include <sstream>

#include "sml_Connection.h"
#include "sml_Client.h"
#include "sml_Utils.h"
#include "thread_Event.h"

#include "handlers.h"

void Handlers::MyBoolShutdownHandler(sml::smlSystemEventId, void* pUserData, sml::Kernel*)
{
	CPPUNIT_ASSERT( pUserData );
	bool* pHandlerReceived = static_cast< bool* >( pUserData );
	*pHandlerReceived = true;
}

void Handlers::MyEventShutdownHandler(sml::smlSystemEventId, void* pUserData, sml::Kernel*)
{
	CPPUNIT_ASSERT( pUserData );
	soar_thread::Event* pEvent = static_cast< soar_thread::Event* >( pUserData );
	pEvent->TriggerEvent();
}

void Handlers::MyDeletionHandler(sml::smlAgentEventId, void* pUserData, sml::Agent*)
{
	CPPUNIT_ASSERT( pUserData );
	bool* pHandlerReceived = static_cast< bool* >( pUserData );
	*pHandlerReceived = true;
}

void Handlers::MySystemEventHandler( sml::smlSystemEventId, void*, sml::Kernel* )
{
	// BUGBUG see comments above registration line
}

void Handlers::MyCreationHandler( sml::smlAgentEventId, void* pUserData, sml::Agent* )
{
	CPPUNIT_ASSERT( pUserData );
	bool* pHandlerReceived = static_cast< bool* >( pUserData );
	*pHandlerReceived = true;
}

void Handlers::MyEchoEventHandler( sml::smlPrintEventId, void* pUserData, sml::Agent*, char const* pMsg )
{
	CPPUNIT_ASSERT( pMsg != NULL );
	CPPUNIT_ASSERT( pUserData );
	bool* pHandlerReceived = static_cast< bool* >( pUserData );
	*pHandlerReceived = true;
}

void Handlers::MyProductionHandler( sml::smlProductionEventId id, void* pUserData, sml::Agent*, char const*, char const* )
{
	CPPUNIT_ASSERT( pUserData );
	int* pInt = static_cast< int* >( pUserData );

	// Increase the count
	*pInt += 1 ;

	CPPUNIT_ASSERT( id == sml::smlEVENT_BEFORE_PRODUCTION_REMOVED );
}

std::string Handlers::MyClientMessageHandler( sml::smlRhsEventId, void* pUserData, sml::Agent*, char const*, char const* pMessage)
{
	std::stringstream res;
	res << "handler-message" << pMessage;

	CPPUNIT_ASSERT( pUserData );
	bool* pHandlerReceived = static_cast< bool* >( pUserData );
	*pHandlerReceived = true;

	return res.str() ;
}

// This is a very dumb filter--it adds "--depth 2" to all commands passed to it.
std::string Handlers::MyFilterHandler( sml::smlRhsEventId, void* pUserData, sml::Agent*, char const*, char const* pCommandLine)
{
	sml::ElementXML* pXML = sml::ElementXML::ParseXMLFromString( pCommandLine ) ;
	CPPUNIT_ASSERT( pXML );
	CPPUNIT_ASSERT( pXML->GetAttribute( sml::sml_Names::kFilterCommand ) );

	std::stringstream commandLine;
	commandLine << pXML->GetAttribute( sml::sml_Names::kFilterCommand ) << " --depth 2";

	// Replace the command attribute in the XML
	CPPUNIT_ASSERT( pXML->AddAttribute( sml::sml_Names::kFilterCommand, commandLine.str().c_str() ) );

	// Convert the XML back to a string and put it into a std::string ready to return
	char *pXMLString = pXML->GenerateXMLString(true) ;
	CPPUNIT_ASSERT( pXMLString );
	std::string res( pXMLString );

	pXML->DeleteString( pXMLString );
	delete pXML ;

	CPPUNIT_ASSERT( pUserData );
	bool* pHandlerReceived = static_cast< bool* >( pUserData );
	*pHandlerReceived = true;

	return res ;
}

void Handlers::MyRunEventHandler( sml::smlRunEventId, void* pUserData, sml::Agent*, sml::smlPhase)
{
	CPPUNIT_ASSERT( pUserData );
	int* pInt = static_cast< int* >( pUserData );

	// Increase the count
	*pInt = *pInt + 1 ;
}

void Handlers::MyUpdateEventHandler( sml::smlUpdateEventId, void* pUserData, sml::Kernel*, sml::smlRunFlags )
{
	CPPUNIT_ASSERT( pUserData );
	int* pInt = static_cast< int* >( pUserData );

	// Increase the count
	*pInt = *pInt + 1 ;
}

void Handlers::MyOutputNotificationHandler(void* pUserData, sml::Agent*)
{
	CPPUNIT_ASSERT( pUserData );
	int* pInt = static_cast< int* >( pUserData );

	// Increase the count
	*pInt = *pInt + 1 ;
}

void Handlers::MyRunSelfRemovingHandler( sml::smlRunEventId, void* pUserData, sml::Agent* pAgent, sml::smlPhase)
{
	CPPUNIT_ASSERT( pUserData );
	int* myCallback = static_cast< int* >( pUserData );

	// This callback removes itself from the list of callbacks 
	// as a test to see if we can do that inside a callback handler.
	CPPUNIT_ASSERT( *myCallback != -1 );
	CPPUNIT_ASSERT( pAgent->UnregisterForRunEvent( *myCallback ) );

	*myCallback = -1 ;
}

std::string Handlers::MyStringEventHandler( sml::smlStringEventId id, void* pUserData, sml::Kernel*, char const* )
{
	CPPUNIT_ASSERT( pUserData );
	bool* pHandlerReceived = static_cast< bool* >( pUserData );
	*pHandlerReceived = true;

	// new: string events need to return empty string on success
	return "";
}

// Register a second handler for the same event, to make sure that's ok.
void Handlers::MyDuplicateRunEventHandler( sml::smlRunEventId, void* pUserData, sml::Agent*, sml::smlPhase )
{
	CPPUNIT_ASSERT( pUserData );
	int* pInt = static_cast< int* >( pUserData );
	CPPUNIT_ASSERT( *pInt == 25 );
}

void Handlers::MyPrintEventHandler( sml::smlPrintEventId, void* pUserData, sml::Agent*, char const* pMessage )
{
	// In this case the user data is a string we're building up
	CPPUNIT_ASSERT( pUserData );
	std::stringstream* pTrace = static_cast< std::stringstream* >( pUserData );

	(*pTrace) << pMessage ;
}

void Handlers::MyXMLEventHandler( sml::smlXMLEventId, void* pUserData, sml::Agent*, sml::ClientXML* pXML )
{
	// pXML should be some structured trace output.
	// Let's examine it a bit.
	// We'll start by turning it back into XML so we can look at it in the debugger.
	char* pStr = pXML->GenerateXMLString( true ) ;
	CPPUNIT_ASSERT( pStr );

	// This will always succeed.  If this isn't really trace XML
	// the methods checking on tag names etc. will just fail
	sml::ClientTraceXML* pRootXML = pXML->ConvertToTraceXML() ;
	CPPUNIT_ASSERT( pRootXML );

	// The root object is just a <trace> tag.  The substance is in the children
	// so we'll get the first child which should exist.
	sml::ClientTraceXML childXML ;
	CPPUNIT_ASSERT( pRootXML->GetChild( &childXML, 0 ) );
	sml::ClientTraceXML* pTraceXML = &childXML ;

	if (pTraceXML->IsTagState())
	{
		CPPUNIT_ASSERT( pTraceXML->GetDecisionCycleCount() );
		std::string count = pTraceXML->GetDecisionCycleCount() ;

		CPPUNIT_ASSERT( pTraceXML->GetStateID() );
		std::string stateID = pTraceXML->GetStateID() ;

		CPPUNIT_ASSERT( pTraceXML->GetImpasseObject() );
		std::string impasseObject = pTraceXML->GetImpasseObject() ;

		CPPUNIT_ASSERT( pTraceXML->GetImpasseType() );
		std::string impasseType = pTraceXML->GetImpasseType() ;
	}

	// Make a copy of the object we've been passed which should remain valid
	// after the event handler has completed.  We only keep the last message
	// in this test.  This is a stress test for our memory allocation logic.
	// We're not allowed to keep pXML that we're passed, but we can copy it and keep the copy.
	// (The copy is very efficient, the underlying object is ref-counted).
	sml::ClientXML** clientXMLStorage = static_cast< sml::ClientXML** >( pUserData );;
	if (*clientXMLStorage != NULL)
		delete *clientXMLStorage ;
	*clientXMLStorage = new sml::ClientXML( pXML ) ;

	pXML->DeleteString( pStr ) ;
}

void Handlers::MyInterruptHandler(sml::smlRunEventId, void* pUserData, sml::Agent* pAgent, sml::smlPhase)
{
	pAgent->GetKernel()->StopAllAgents() ;

	CPPUNIT_ASSERT( pUserData );
	bool* pHandlerReceived = static_cast< bool* >( pUserData );
	*pHandlerReceived = true;
}

std::string Handlers::MyRhsFunctionHandler(sml::smlRhsEventId, void* pUserData, sml::Agent*, char const*, char const* pArgument)
{
	// This is optional because the callback needs to be around for the program to function properly
	// we only test for this specifically in one part of clientsmltest
	if ( pUserData ) 
	{
		bool* pHandlerReceived = static_cast< bool* >( pUserData );
		*pHandlerReceived = true;
	}

	std::stringstream res;
	res << "my rhs result " << pArgument;
	return res.str() ;
}

void Handlers::MyMemoryLeakUpdateHandler( sml::smlUpdateEventId id, void* pUserData, sml::Kernel* pKernel, sml::smlRunFlags runFlags )
{
	static int step(0);

	static sml::Identifier* pRootID( 0 );
	static sml::StringElement* pRootString( 0 );
	static sml::FloatElement* pRootFloat( 0 );
	static sml::IntElement* pRootInt( 0 );

	static sml::Identifier* pChildID( 0 );
	static sml::StringElement* pChildString( 0 );
	static sml::FloatElement* pChildFloat( 0 );
	static sml::IntElement* pChildInt( 0 );

	CPPUNIT_ASSERT( pUserData );
	sml::Agent* pAgent = static_cast< sml::Agent* >( pUserData );

	//std::cout << "step: " << step << std::endl;

	switch ( step % 3 )
	{
	case 0:
		pRootID = pAgent->CreateIdWME( pAgent->GetInputLink(), "pRootID" ) ;
		pRootString = pAgent->CreateStringWME( pAgent->GetInputLink(), "pRootString", "RootValue" ) ;
		pRootFloat = pAgent->CreateFloatWME( pAgent->GetInputLink(), "pRootFloat", 1.0 ) ;
		pRootInt = pAgent->CreateIntWME( pAgent->GetInputLink(), "pRootInt", 1 ) ;

		pChildID = pAgent->CreateIdWME( pRootID, "pChildID" ) ;
		pChildString = pAgent->CreateStringWME( pRootID, "ChildWME", "ChildValue" ) ;
		pChildFloat = pAgent->CreateFloatWME( pRootID, "ChildWME", 2.0 ) ;
		pChildInt = pAgent->CreateIntWME( pRootID, "ChildWME", 2 ) ;
		break;

	case 1:
		CPPUNIT_ASSERT( pAgent->DestroyWME( pChildID ) );		// BUGBUG: Should be destroyed by deletion of pRootID
		CPPUNIT_ASSERT( pAgent->DestroyWME( pChildString ) );	// BUGBUG: Should be destroyed by deletion of pRootID
		CPPUNIT_ASSERT( pAgent->DestroyWME( pChildFloat ) );	// BUGBUG: Should be destroyed by deletion of pRootID
		CPPUNIT_ASSERT( pAgent->DestroyWME( pChildInt ) );		// BUGBUG: Should be destroyed by deletion of pRootID

		CPPUNIT_ASSERT( pAgent->DestroyWME( pRootID ) );		// BUGBUG: Should destroy children, doesn't
		CPPUNIT_ASSERT( pAgent->DestroyWME( pRootString ) );
		CPPUNIT_ASSERT( pAgent->DestroyWME( pRootFloat ) );
		CPPUNIT_ASSERT( pAgent->DestroyWME( pRootInt ) );

		pRootID = 0;
		pRootString = 0;
		pRootFloat = 0;
		pRootInt = 0;

		pChildID = 0;
		pChildString = 0;
		pChildFloat = 0;
		pChildInt = 0;
		break;

	default:
		break;
	}

	++step;
}