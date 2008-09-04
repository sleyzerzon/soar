#ifndef CLI_CLI_H
#define CLI_CLI_H

#include "cli_Command.h"
#include "cli_Aliases.h"

#include <vector>
#include <string>
#include <algorithm>

namespace soarxml
{
	class ElementXML ;
}

namespace sml {
	class KernelSML;
	class Connection ;
	class AgentSML;
}

namespace cli
{
	class CLI
	{
	public:
		CLI();
		~CLI();

		void SetUsingRawOutput( bool usingRawOutput )
		{
			m_CommandOutput.SetUsingRawOutput( usingRawOutput );
		}

		bool Translate( std::vector< std::string >& argv ) const
		{
			return m_Aliases.Translate( argv );
		}

		bool ShouldEchoCommand( const std::string& command ) const
		{
			std::vector< Command* >::const_iterator commandIter = std::find_if( m_Commands.begin(), m_Commands.end(), CommandIs( command ) );
			if ( commandIter == m_Commands.end() )
			{
				return false; // command not found
			}

			return (*commandIter)->GetEchoFlag();
		}

		bool ExecuteCommand( sml::KernelSML& kernelSML, sml::Connection& connection, sml::AgentSML* pAgentSML, std::vector< std::string >& argv, bool echoResults, soarxml::ElementXML& response );

	private:
		std::vector< Command* > m_Commands;
		CommandOutput m_CommandOutput;
		Aliases m_Aliases;
		std::ofstream* m_pLogFile;
	};
}

#endif // CLI_CLI_H
