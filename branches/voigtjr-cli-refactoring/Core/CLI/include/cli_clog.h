#ifndef CLI_CLOG_H
#define CLI_CLOG_H

#include "cli_Command.h"
#include "cli_OptionHandler.h"

namespace cli
{
	class CLog : public Command
	{
	public:
		CLog();
		virtual ~CLog() {}
		virtual char const* GetCommandName() const { return "clog"; }
		virtual bool GetEchoFlag() const { return true; }
		virtual void operator()( std::vector< std::string >& argv, CommandOutput& commandOutput );

	private:
		OptionHandler m_OptionHandler;
		int m_Add;
		int m_Append;
		int m_Close;
		int m_Disable;
		int m_Existing;
		int m_Query;
	};
}

#endif // CLI_CLOG_H
