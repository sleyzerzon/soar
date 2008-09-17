#include <portability.h>

#include "cli_echo.h"
#include "cli_CLI.h"

using namespace cli;
using namespace sml;

CLog::CLog() 
{
	m_OptionHandler.AddOption( 'a', "add", &m_Add );
	m_OptionHandler.AddOption( 'A', "append", &m_Append );
	m_OptionHandler.AddOption( 'c', "close", &m_Close );
	m_OptionHandler.AddOption( 'd', "disable", &m_Disable );
	m_OptionHandler.AddOption( 'e', "existing", &m_Existing );
	m_OptionHandler.AddOption( 'd', "off", &m_Disable );
	m_OptionHandler.AddOption( 'q', "query", &m_Query );
}

void CLog::operator()( std::vector< std::string >& argv, CommandOutput& commandOutput )
{
	m_OptionHandler.ProcessOptions( argv );

	if ( m_Add )
	{
		if ( argv.size() < 2 )
		{
		}
	}
}
