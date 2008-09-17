/////////////////////////////////////////////////////////////////
// input-period command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2005
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "cli_CLIError.h"
#include "sml_Names.h"

#include "agent.h"

#include <boost/lexical_cast.hpp>

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseInputPeriod(std::vector<std::string>& argv) {
	if (argv.size() > 2) return SetError(CLIError::kTooManyArgs);
	if (argv.size() == 1) return DoInputPeriod();

	if (!IsInteger(argv[1])) return SetError(CLIError::kIntegerExpected);
	
	int period = atoi(argv[1].c_str());
	if (period < 0) return SetError(CLIError::kIntegerMustBeNonNegative);

	return DoInputPeriod(&period);
}

bool CommandLineInterface::DoInputPeriod(int* pPeriod) {
	if (m_pAgentSoar->operand2_mode) return SetError(CLIError::kSoar7Command);

	if (!pPeriod) {
		if (m_RawOutput) {
			m_Result << m_pAgentSoar->input_period;
		} else {
			AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeInt, boost::lexical_cast< std::string >( m_pAgentSoar->input_period ));
		}
		return true;
	}

	m_pAgentSoar->input_period = *pPeriod;
	return true;
}

