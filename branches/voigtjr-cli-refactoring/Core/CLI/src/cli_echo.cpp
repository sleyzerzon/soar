#include <portability.h>

#include "cli_echo.h"
#include "cli_CLI.h"

#include "sml_Names.h"
#include "sml_StringOps.h"

#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>

using namespace cli;
using namespace sml;

Echo::Echo() 
: m_DontEchoNewline( 0 )
{
	m_OptionHandler.AddOption( 'n', "no-newline", &m_DontEchoNewline );
}

void Echo::operator()( std::vector< std::string >& argv, CommandOutput& commandOutput )
{
	m_DontEchoNewline = 0;

	m_OptionHandler.ProcessOptions( argv );

	// remove command name
	argv.erase( argv.begin() );

	// combine args
	std::ostringstream output;
	std::for_each( argv.begin(), argv.end(), BuildOutputString( output ) );

	// remove trailing space
	assert( output.str().length() );
	std::string outputString( output.str().substr( 0, output.str().length() - 1 ) );

	// Convert backslash characters
	for ( std::string::size_type pos = 0; ( pos = outputString.find( '\\', pos ) ) != std::string::npos; ++pos ) {
		if ( outputString.size() <= pos + 1 ) 
		{
			break;
		}

		// Found a backslash with a character after it, remove it
		outputString.erase( pos, 1 );
		switch ( outputString.at( pos ) ) 
		{
			case '\\':
				break;
			case 'b': // backspace
				outputString[pos] = '\b';
				break;
			case 'c': // supress trailing newline
				outputString.erase(pos, 1);
				--pos;
				m_DontEchoNewline = true;
				break;
			case 'f': // form feed
				outputString[pos] = '\f';
				break;
			case 'n': // newline
				outputString[pos] = '\n';
				break;
			case 'r': // carriage return
				outputString[pos] = '\r';
				break;
			case 't': // horizontal tab
				outputString[pos] = '\t';
				break;
			case 'v': // vertical tab
				outputString[pos] = '\v';
				break;
			default: // error
				throw InvalidEscapedCharacterCode( outputString.at( pos ) );
		} 
	}

	// Add newline if applicable
	if ( !m_DontEchoNewline ) {
		outputString += '\n';
	}

	if ( commandOutput.GetUsingRawOutput() ) {
		commandOutput.m_RawOutput << outputString;
	} else {
		commandOutput.AppendArgTagFast( sml_Names::kParamMessage, sml_Names::kTypeString, outputString.c_str() );
	}
}
