#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include <fstream>

#include <assert.h>

#include "cli_GetOpt.h"
#include "cli_Constants.h"
#include "cli_CommandData.h"

#include "sml_Names.h"

#include "IgSKI_Agent.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseLog(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	static struct GetOpt::option longOptions[] = {
		{"add",			0, 0, 'a'},
		{"append",		0, 0, 'A'},
		{"close",		0, 0, 'c'},
		{"disable",		0, 0, 'd'},
		{"existing",	0, 0, 'e'},
		{"off",			0, 0, 'd'},
		{"query",		0, 0, 'q'},
		{0, 0, 0, 0}
	};

	eLogMode mode = LOG_NEW;

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, "aAcdeoq", longOptions, 0);
		if (option == -1) break;

		switch (option) {
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
			case '?':
				return SetError(CLIError::kUnrecognizedOption);
			default:
				return SetError(CLIError::kGetOptError);
		}
	}
	
	switch (mode) {
		case LOG_ADD:
			{
				std::string toAdd;
				// no less than one non-option argument
				if (m_pGetOpt->GetAdditionalArgCount() < 1) return SetError(CLIError::kTooFewArgs);

				// move to the first non-option arg
				std::vector<std::string>::iterator iter = argv.begin();
				for (int i = 0; i < m_pGetOpt->GetOptind(); ++i) ++iter;

				// combine all args
				while (iter != argv.end()) {
					toAdd += *iter;
					toAdd += ' ';
					++iter;
				}
				return DoLog(pAgent, mode, 0, &toAdd);
			}

		case LOG_NEW:
			// no more than one argument, no filename == query
			if (m_pGetOpt->GetAdditionalArgCount() > 1) return SetError(CLIError::kTooManyArgs);
			if (m_pGetOpt->GetAdditionalArgCount() == 1) return DoLog(pAgent, mode, &argv[1]);
			break; // no args case handled below

		case LOG_NEWAPPEND:
			// exactly one argument
			if (m_pGetOpt->GetAdditionalArgCount() > 1) return SetError(CLIError::kTooManyArgs);
			if (m_pGetOpt->GetAdditionalArgCount() < 1) return SetError(CLIError::kTooFewArgs);
			return DoLog(pAgent, mode, &argv[1]);

		case LOG_CLOSE:
		case LOG_QUERY:
			// no arguments
			if (m_pGetOpt->GetAdditionalArgCount()) return SetError(CLIError::kTooManyArgs);
			break; // no args case handled below

		default:
			return SetError(CLIError::kInvalidOperation);
	}

	// the no args case
	return DoLog(pAgent, mode);
}

bool CommandLineInterface::DoLog(gSKI::IAgent* pAgent, const eLogMode mode, const std::string* pFilename, const std::string* pToAdd) {
	if (!RequireAgent(pAgent)) return false;

	std::ios_base::openmode openmode = std::ios_base::out;

	switch (mode) {
		case LOG_NEWAPPEND:
			openmode |= std::ios_base::app;
			// falls through

		case LOG_NEW:
			if (!pFilename) break; // handle as just a query
			if (m_pLogFile) return SetError(CLIError::kLogAlreadyOpen);
			m_pLogFile = new std::ofstream(pFilename->c_str(), openmode);
			if (!m_pLogFile) return SetError(CLIError::kLogOpenFailure);
			m_LogFilename = *pFilename;
			if (pAgent) pAgent->AddPrintListener(gSKIEVENT_PRINT, this);
			break;

		case LOG_ADD:
			if (!m_pLogFile) return SetError(CLIError::kLogNotOpen);
			(*m_pLogFile) << pToAdd << std::endl;
			return true;

		case LOG_CLOSE:
			if (!m_pLogFile) return SetError(CLIError::kLogNotOpen);
	
			(*m_pLogFile) << "Log file closed." << std::endl;

			if (pAgent) pAgent->RemovePrintListener(gSKIEVENT_PRINT, this);
			delete m_pLogFile;
			m_pLogFile = 0;
			m_LogFilename.clear();
			break;

		case LOG_QUERY:
			break;
		default: assert(false);
	}

	// Query at end of successful command, or by default (but not on _ADD)
	if (m_RawOutput) {
		m_Result << "Log file ";
		if (m_pLogFile) {
			m_Result << "'" + m_LogFilename + "' opened.";
		} else {
			m_Result << "closed.";
		}

	} else {
		const char* setting = m_pLogFile ? sml_Names::kTrue : sml_Names::kFalse;
		AppendArgTagFast(sml_Names::kParamLogSetting, sml_Names::kTypeBoolean, setting);

		if (m_LogFilename.size()) AppendArgTagFast(sml_Names::kParamFilename, sml_Names::kTypeString, m_LogFilename.c_str());
	}

	return true;
}

