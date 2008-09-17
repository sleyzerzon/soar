#include "cli_CommandOutput.h"

#include "sml_TagResult.h"
#include "sml_TagArg.h"
#include "sml_StringOps.h"
#include "sml_KernelCallback.h"
#include "sml_Events.h"
#include "xml.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <cassert>

using namespace cli;
using namespace sml;

CommandOutput::CommandOutput()
	: m_UsingRawOutput( true )
	, m_pLogFile( 0 )
	, m_TrapPrintEvents( false )
	, m_VarPrint( false )
{}

virtual CommandOutput::~CommandOutput()
{
	if ( m_pLogFile ) {
		(*m_pLogFile) << "Log file closed due to shutdown." << std::endl;
		delete m_pLogFile;
		m_pLogFile = 0;
	}
}

void CommandOutput::LogCommandLine( const std::vector< std::string >& argv )
{
	if ( !m_pLogFile )
	{
		return;
	}

	std::for_each( argv.begin(), argv.end(), ConcatinateStringsWithSpaces( *m_pLogFile ) );
	(*m_pLogFile) << std::endl;
}

void CommandOutput::SetUsingRawOutput( bool usingRawOutput ) 
{ 
	m_UsingRawOutput = usingRawOutput;
}

bool CommandOutput::GetUsingRawOutput() const 
{ 
	return m_UsingRawOutput; 
}

void CommandOutput::SetTrapPrintCallbacks( bool setting )
{
	if ( !GetAgentSML() )
	{
		return;
	}

	// If we've already set it, don't re-set it
	if ( m_TrapPrintEvents == setting )
	{
		return;
	}

	if ( setting )
	{
		// Trap print callbacks
		GetAgentSML()->DisablePrintCallback();
		m_TrapPrintEvents = true;
		if ( !m_pLogFile ) 
		{
			// If we're logging, we're already registered for this.
			RegisterWithKernel( smlEVENT_PRINT );
		}

		// Tell kernel to collect result in command buffer as opposed to trace buffer
		xml_begin_command_mode( GetAgentSML()->GetSoarAgent() );
	}
	else
	{
		// Retrieve command buffer, tell kernel to use trace buffer again
		ElementXML* pXMLCommandResult = xml_end_command_mode( GetAgentSML()->GetSoarAgent() );

		// The root object is just a <trace> tag.  The substance is in the children
		// Add childrend of the command buffer to response tags
		for ( int i = 0; i < pXMLCommandResult->GetNumberChildren(); ++i )
		{
			ElementXML* pChildXML = new ElementXML();
			pXMLCommandResult->GetChild( pChildXML, i );

			m_ResponseTags.push_back( pChildXML );
		}

		delete pXMLCommandResult;

		if ( !m_UsingRawOutput )
		{
			// Add text result to response tags
			if ( m_RawOutput.str().length() )
			{
				AppendArgTagFast( sml_Names::kParamMessage, sml_Names::kTypeString, m_RawOutput.str().c_str() );
				m_Result.str("");
			}
		}

		// Re-enable print callbacks
		if ( !m_pLogFile ) 
		{
			// If we're logging, we want to stay registered for this
			UnregisterWithKernel( smlEVENT_PRINT );
		}
		m_TrapPrintEvents = false;
		GetAgentSML()->EnablePrintCallback();
	}
}

virtual void CommandOutput::OnKernelEvent( int eventID, AgentSML* pAgentSML, void* pCallData )
{
	if ( eventID == smlEVENT_PRINT )
	{
		std::string message( std::string message( msg ) );

		if ( m_TrapPrintEvents || m_pLogFile )
		{
			if ( m_VarPrint )
			{
				// Transform if varprint, see print command
				regex_t comp;
				regcomp( &comp, "[A-Z][0-9]+", REG_EXTENDED );

				regmatch_t match;
				memset( &match, 0, sizeof( regmatch_t ) );

				while ( regexec( &comp, message.substr( match.rm_eo, message.size() - match.rm_eo ).c_str(), 1, &match, 0 ) == 0 ) 
				{
					message.insert( match.rm_so, "<" );
					message.insert( match.rm_eo + 1, ">" );
					match.rm_eo += 2;
				}  

				regfree(&comp);
			} 

			// Simply append to message result
			if ( m_TrapPrintEvents ) 
			{
				m_RawOutput << message;
			}
			
			if ( m_pLogFile )
			{
				(*m_pLogFile) << message;
			}
		}
	}
	else
	{
		assert( false && "unknown event" );
	}
}

void CommandOutput::Clear()
{
	if ( m_RawOutput.str().length() )
	{
		m_RawOutput.str( "" );
	}

	if ( m_ResponseTags.size() )
	{
		assert( false ); // this next line must be checked
		std::vector< soarxml::ElementXML* >( m_ResponseTags ).swap( m_ResponseTags );
	}
}

bool CommandOutput::HaveOutput()
{
	return m_RawOutput.str().length() || m_ResponseTags.size();
}

void CommandOutput::AppendArgTag( const char* pParam, const char* pType, const char* pValue ) 
{
	sml::TagArg* pTag = new sml::TagArg();
	pTag->SetParam( pParam );
	pTag->SetType( pType );
	pTag->SetValue( pValue );
	m_ResponseTags.push_back( pTag );
}

void CommandOutput::AppendArgTagFast( const char* pParam, const char* pType, const char* pValue )
{
	sml::TagArg* pTag = new sml::TagArg();
	pTag->SetParamFast( pParam );
	pTag->SetTypeFast( pType );
	pTag->SetValue( pValue );
	m_ResponseTags.push_back( pTag );
}

void CommandOutput::PrependArgTag( const char* pParam, const char* pType, const char* pValue ) 
{
	sml::TagArg* pTag = new sml::TagArg();
	pTag->SetParam( pParam );
	pTag->SetType( pType );
	pTag->SetValue( pValue );
	m_ResponseTags.insert( m_ResponseTags.begin(), pTag );
}

void CommandOutput::PrependArgTagFast( const char* pParam, const char* pType, const char* pValue ) 
{
	sml::TagArg* pTag = new sml::TagArg();
	pTag->SetParamFast( pParam );
	pTag->SetTypeFast( pType );
	pTag->SetValue( pValue );
	m_ResponseTags.insert( m_ResponseTags.begin(), pTag );
}

void CommandOutput::RecordResponse( soarxml::ElementXML* pResponse )
{
	sml::TagResult* pTag = new sml::TagResult();
	
	for ( std::vector< soarxml::ElementXML* >::iterator iter = m_ResponseTags.begin(); iter != m_ResponseTags.end(); ++iter )
	{
		pTag->AddChild( *iter );
	}

	pResponse->AddChild( pTag );
}
