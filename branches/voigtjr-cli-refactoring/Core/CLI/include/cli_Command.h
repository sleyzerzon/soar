#ifndef CLI_COMMAND_H
#define CLI_COMMAND_H

#include "cli_CommandOutput.h"

#include <vector>

namespace cli
{
	struct Command
	{
		virtual ~Command() {}
		virtual char const* GetCommandName() const = 0;
		virtual bool GetEchoFlag() const = 0;
		virtual void operator()( std::vector< std::string >& argv, CommandOutput& commandOutput ) = 0;
	};

	struct CommandIs 
	{
		const std::string& command;

		CommandIs( const std::string& command ) 
			: command( command )
		{
		}

		bool operator()( const Command* const value ) const
		{
			return command == value->GetCommandName();
		}

	private:
		CommandIs& operator=( const CommandIs& );
	};
}


#endif // CLI_COMMAND_H
