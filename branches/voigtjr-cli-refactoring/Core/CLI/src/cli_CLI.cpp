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
	// Log input
	if (m_pLogFile) {
		if (pAgentSML) 
		{
			(*m_pLogFile) << pAgent->GetName() << "> ";
		}
		std::for_each( argv.begin(), argv.end(), BuildOutputString( *m_pLogFile ) );
		(*m_pLogFile) << std::endl;
	}

	SetTrapPrintCallbacks( true );

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

void CLI::SetTrapPrintCallbacks( bool setting )
{
	if (!m_pAgentSML)
	{
		return;
	}

	// If we've already set it, don't re-set it
	if ( m_TrapPrintEvents == setting )
	{
		return;
	}

	if (setting)
	{
		// Trap print callbacks
		m_pAgentSML->DisablePrintCallback();
		m_TrapPrintEvents = true;
		if (!m_pLogFile) 
		{
			// If we're logging, we're already registered for this.
			RegisterWithKernel(smlEVENT_PRINT);
		}

		// Tell kernel to collect result in command buffer as opposed to trace buffer
		xml_begin_command_mode( m_pAgentSML->GetSoarAgent() );
	}
	else
	{
		// Retrieve command buffer, tell kernel to use trace buffer again
		ElementXML* pXMLCommandResult = xml_end_command_mode( m_pAgentSML->GetSoarAgent() );

		// The root object is just a <trace> tag.  The substance is in the children
		// Add childrend of the command buffer to response tags
		for ( int i = 0; i < pXMLCommandResult->GetNumberChildren(); ++i )
		{
			ElementXML* pChildXML = new ElementXML();
			pXMLCommandResult->GetChild( pChildXML, i );

			m_ResponseTags.push_back( pChildXML );
		}

		delete pXMLCommandResult;

		if ( !m_RawOutput )
		{
			// Add text result to response tags
			if ( m_Result.str().length() )
			{
				AppendArgTagFast( sml_Names::kParamMessage, sml_Names::kTypeString, m_Result.str().c_str() );
				m_Result.str("");
			}
		}

		// Re-enable print callbacks
		if (!m_pLogFile) 
		{
			// If we're logging, we want to stay registered for this
			UnregisterWithKernel(smlEVENT_PRINT);
		}
		m_TrapPrintEvents = false;
		m_pAgentSML->EnablePrintCallback();
	}
}

