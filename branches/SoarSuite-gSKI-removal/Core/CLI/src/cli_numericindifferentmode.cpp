/////////////////////////////////////////////////////////////////
// numeric-indifferent-mode command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "sml_Names.h"
#include "sml_StringOps.h"

#include "agent.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseNumericIndifferentMode(std::vector<std::string>& argv) {

	Options optionsData[] = {
		{'a', "average",	0},
		{'a', "avg",		0},
		{'s', "sum",		0},
		{0, 0, 0}
	};

	eNumericIndifferentMode mode = NUMERIC_INDIFFERENT_QUERY;

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
			case 'a':
				mode = NUMERIC_INDIFFERENT_AVERAGE;
				break;
			case 's':
				mode = NUMERIC_INDIFFERENT_SUM;
				break;
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	// No additional arguments
	if (m_NonOptionArguments) return SetError(CLIError::kTooManyArgs);		

	return DoNumericIndifferentMode(mode);
}

bool CommandLineInterface::DoNumericIndifferentMode(const eNumericIndifferentMode mode) {
	if (!RequireAgent()) return false;

	switch (mode) {
		case 0:
			break;
		case NUMERIC_INDIFFERENT_AVERAGE:
			m_pAgentSoar->numeric_indifferent_mode = NUMERIC_INDIFFERENT_MODE_AVG;
			return true;
		case NUMERIC_INDIFFERENT_SUM:
			m_pAgentSoar->numeric_indifferent_mode = NUMERIC_INDIFFERENT_MODE_SUM;
			return true;
		default:
			return SetError(CLIError::kInvalidNumericIndifferentMode);
	}
	
	char buf[kMinBufferSize];
	if (m_RawOutput) {
		m_Result << "Current numeric indifferent mode: ";
	}

	switch (m_pAgentSoar->numeric_indifferent_mode) {
		case NUMERIC_INDIFFERENT_MODE_AVG:
			if (m_RawOutput) {
				m_Result << "average";
			} else {
				AppendArgTagFast(sml_Names::kParamNumericIndifferentMode, sml_Names::kTypeInt, Int2String((int)NUMERIC_INDIFFERENT_AVERAGE, buf, kMinBufferSize));
			}
			break;
		case NUMERIC_INDIFFERENT_MODE_SUM:
			if (m_RawOutput) {
				m_Result << "sum";
			} else {
				AppendArgTagFast(sml_Names::kParamNumericIndifferentMode, sml_Names::kTypeInt, Int2String((int)NUMERIC_INDIFFERENT_SUM, buf, kMinBufferSize));
			}
			break;
		default:
			return SetError(CLIError::kInvalidNumericIndifferentMode);
	}
	return true;
}
