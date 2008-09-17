#include <portability.h>

#include "cli_CLI.h"
#include "cli_echo.h"

using namespace cli;


CLI::CLI()
{
	m_Commands.push_back( new Echo() );
}

void deleteEach( Command*& command )
{
	delete command;
}

CLI::~CLI()
{
	std::for_each( m_Commands.begin(), m_Commands.end(), deleteEach );
}

bool CLI::ExecuteCommand( sml::KernelSML& kernelSML, sml::Connection& connection, sml::AgentSML* pAgentSML, std::vector< std::string >& argv, bool echoResults, soarxml::ElementXML& response )
{
	PushAgent( pAgent );

	// Log command line
	m_CommandOutput.LogCommandLine( std::vector< std::string >& argv );

	m_CommandOutput.SetTrapPrintCallbacks( true );

	m_SourceDepth = 0;
	m_SourceMode = SOURCE_DEFAULT;
	m_SourceVerbose = false; 

	// Process the command, ignoring its result (errors detected with m_LastError)
	//DoCommandInternal(pCommandLine);
	std::stringstream soarStream;
	soarStream << pCommandLine;
	StreamSource( soarStream, 0 );

	SetTrapPrintCallbacks( false );

	GetLastResultSML(pConnection, pResponse);

	PopAgent();

	// Always returns true to indicate that we've generated any needed error message already
	return true;
}

void CLI::PushAgent( sml::AgentSML* pAgent )
{
	m_pAgentSMLStack.push( pAgent );

	m_CommandOutput.SetAgentSML( pAgent );
}

void CLI::PopAgent()
{
	m_pAgentSMLStack.pop();
	if ( m_pAgentSMLStack.size() )
	{
		m_CommandOutput.SetAgentSML( m_pAgentSMLStack.top() );
	}
	else
	{
		m_CommandOutput.SetAgentSML( 0 ) ;
	}
}

