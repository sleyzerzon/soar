#ifndef CLI_ECHO_H
#define CLI_ECHO_H

#include "cli_Command.h"
#include "cli_OptionHandler.h"

namespace cli
{
	class Echo : public Command
	{
	public:
		Echo();
		virtual ~Echo() {}
		virtual char const* GetCommandName() const { return "echo"; }
		virtual bool GetEchoFlag() const { return true; }
		virtual void operator()( std::vector< std::string >& argv, CommandOutput& commandOutput );

		struct InvalidEscapedCharacterCode : public std::exception
		{
			char code;
			InvalidEscapedCharacterCode( char& code )
				: code( code ) {}
			char const* what() const throw()
			{
				return "InvalidEscapedCharacterCode";
			}
		};

	private:
		OptionHandler m_OptionHandler;
		int m_DontEchoNewline;

	};
}

#endif // CLI_ECHO_H
