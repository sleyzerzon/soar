/////////////////////////////////////////////////////////////////
// max-elaborations command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "sml_Names.h"
#include "cli_CLIError.h"

#include "agent.h"

#include <boost/lexical_cast.hpp>

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseMaxElaborations(std::vector<std::string>& argv) {

	// n defaults to 0 (print current value)
	int n = 0;

	if (argv.size() > 2) return SetError(CLIError::kTooManyArgs);

	// one argument, figure out if it is a positive integer
	if (argv.size() == 2) {
		n = atoi(argv[1].c_str());
		if (n <= 0) return SetError(CLIError::kIntegerMustBePositive);
	}

	return DoMaxElaborations(n);
}

bool CommandLineInterface::DoMaxElaborations(const int n) {
	if (!n) {
		// Query
		if (m_RawOutput) {
			m_Result << m_pAgentSoar->sysparams[MAX_ELABORATIONS_SYSPARAM];
		} else {
			AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeInt, boost::lexical_cast< std::string >( m_pAgentSoar->sysparams[MAX_ELABORATIONS_SYSPARAM] ));
		}
		return true;
	}

	m_pAgentSoar->sysparams[MAX_ELABORATIONS_SYSPARAM] = n;
	return true;
}
