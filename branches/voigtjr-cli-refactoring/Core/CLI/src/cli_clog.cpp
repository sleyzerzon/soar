#include <portability.h>

#include "cli_echo.h"
#include "cli_CLI.h"

using namespace cli;
using namespace sml;

CLog::CLog() 
: m_Add( 0 )
, m_Append( 0 )
, m_Close( 0 )
, m_Disable( 0 )
, m_Existing( 0 )
, m_Query( 0 )
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
	m_Add = 0;
	m_Append = 0;
	m_Close = 0;
	m_Disable = 0;
	m_Existing = 0;
	m_Query = 0;

	m_OptionHandler.ProcessOptions( argv );

	if ( m_Add )
	{
		if ( argv.size() < 2 )
		{

	}

}

bool CommandLineInterface::ParseCLog(std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'a', "add",		0},
		{'A', "append",		0},
		{'c', "close",		0},
		{'d', "disable",	0},
		{'e', "existing",	0},
		{'d', "off",		0},
		{'q', "query",		0},
		{0, 0, 0}
	};

	eLogMode mode = LOG_NEW;

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
			case 'a':
				mode = LOG_ADD;
				break;
			case 'c':
			case 'd':
			case 'o':
				mode = LOG_CLOSE;
				break;
			case 'e':
			case 'A':
				mode = LOG_NEWAPPEND;
				break;
			case 'q':
				mode = LOG_QUERY;
				break;
			default:
				return SetError(CLIError::kGetOptError);
		}
	}
	
	switch (mode) {
		case LOG_ADD:
			{
				std::string toAdd;
				// no less than one non-option argument
				if (m_NonOptionArguments < 1) {
					SetErrorDetail("Provide a string to add.");
					return SetError(CLIError::kTooFewArgs);
				}

				// move to the first non-option arg
				std::vector<std::string>::iterator iter = argv.begin();
				for (int i = 0; i < (m_Argument - m_NonOptionArguments); ++i) ++iter;

				// combine all args
				while (iter != argv.end()) {
					toAdd += *iter;
					toAdd += ' ';
					++iter;
				}
				return DoCLog(mode, 0, &toAdd);
			}

		case LOG_NEW:
			// no more than one argument, no filename == query
			if (m_NonOptionArguments > 1) {
				SetErrorDetail("Filename or nothing expected, enclose filename in quotes if there are spaces in the path.");
				return SetError(CLIError::kTooManyArgs);
			}
			if (m_NonOptionArguments == 1) return DoCLog(mode, &argv[1]);
			break; // no args case handled below

		case LOG_NEWAPPEND:
			// exactly one argument
			if (m_NonOptionArguments > 1) {
				SetErrorDetail("Filename expected, enclose filename in quotes if there are spaces in the path.");
				return SetError(CLIError::kTooManyArgs);
			}

			if (m_NonOptionArguments < 1) {
				SetErrorDetail("Please provide a filename.");
				return SetError(CLIError::kTooFewArgs);
			}
			return DoCLog(mode, &argv[1]);

		case LOG_CLOSE:
		case LOG_QUERY:
			// no arguments
			if (m_NonOptionArguments) {
				SetErrorDetail("No arguments when querying log status.");
				return SetError(CLIError::kTooManyArgs);
			}
			break; // no args case handled below

		default:
			return SetError(CLIError::kInvalidOperation);
	}

	// the no args case
	return DoCLog(mode);
}

bool CommandLineInterface::DoCLog(const eLogMode mode, const std::string* pFilename, const std::string* pToAdd, bool silent) {
	std::ios_base::openmode openmode = std::ios_base::out;

 	switch (mode) {
		case LOG_NEWAPPEND:
			openmode |= std::ios_base::app;
			// falls through

		case LOG_NEW:
			if (!pFilename) break; // handle as just a query

			if (m_pLogFile) {
				SetErrorDetail("Currently logging to " + m_LogFilename);
				return SetError(CLIError::kLogAlreadyOpen);
			}

            {
                std::string filename = *pFilename;
                StripQuotes(filename);

			    m_pLogFile = new std::ofstream(filename.c_str(), openmode);
			    if (!m_pLogFile) {
				    SetErrorDetail("Failed to open " + filename);
				    return SetError(CLIError::kLogOpenFailure);
			    }

			    m_LogFilename = filename;
            }
			break;

		case LOG_ADD:
			if (!m_pLogFile) return SetError(CLIError::kLogNotOpen);
			(*m_pLogFile) << *pToAdd << std::endl;
			return true;

		case LOG_CLOSE:
			if (!m_pLogFile) return SetError(CLIError::kLogNotOpen);

			delete m_pLogFile;
			m_pLogFile = 0;
			m_LogFilename.clear();
			break;

		case LOG_QUERY:
			break;
		default: assert(false);
	}

	if (!silent) {
		LogQuery();
	}
	return true;
}

void CommandLineInterface::LogQuery() {
	if (m_RawOutput) {
		m_Result << "Log file ";
		if (m_pLogFile) {
			m_Result << "'" + m_LogFilename + "' open.";
		} else {
			m_Result << "closed.";
		}

	} else {
		const char* setting = m_pLogFile ? sml_Names::kTrue : sml_Names::kFalse;
		AppendArgTagFast(sml_Names::kParamLogSetting, sml_Names::kTypeBoolean, setting);

		if (m_LogFilename.size()) AppendArgTagFast(sml_Names::kParamFilename, sml_Names::kTypeString, m_LogFilename.c_str());
	}
}
