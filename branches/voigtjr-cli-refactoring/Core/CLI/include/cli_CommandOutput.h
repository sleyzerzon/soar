#ifndef CLI_COMMANDOUTPUT_H
#define CLI_COMMANDOUTPUT_H

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

namespace cli
{
	class CommandOutput : public sml::KernelCallback
	{
	public:
		CommandOutput();
		virtual ~CommandOutput();

		void LogCommandLine( const std::vector< std::string >& argv );

		void SetUsingRawOutput( bool usingRawOutput );
		bool GetUsingRawOutput() const;

		void SetTrapPrintCallbacks( bool setting );

		virtual void OnKernelEvent(int eventID, AgentSML* pAgentSML, void* pCallData);

		void Clear();
		bool HaveOutput();

		// raw
		std::ostringstream m_RawOutput;

		// structured
		void AppendArgTag( const char* pParam, const char* pType, const char* pValue );
		void AppendArgTagFast( const char* pParam, const char* pType, const char* pValue );
		void PrependArgTag( const char* pParam, const char* pType, const char* pValue );
		void PrependArgTagFast( const char* pParam, const char* pType, const char* pValue );

		void RecordResponse( soarxml::ElementXML* pResponse );

	private:
		bool m_UsingRawOutput;
		std::vector< soarxml::ElementXML* > m_ResponseTags;
		std::ofstream* m_pLogFile; // The log file stream
		bool m_TrapPrintEvents;
		bool m_VarPrint

		CommandOutput( const CommandOutput& );
		CommandOutput& operator=( const CommandOutput& );
	};

}

#endif // CLI_COMMANDOUTPUT_H
