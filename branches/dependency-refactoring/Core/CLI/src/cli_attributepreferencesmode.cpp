/////////////////////////////////////////////////////////////////
// attribute-preferences command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2005
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "sml_Names.h"
#include "agent.h"
#include "cli_CLIError.h"

#include <boost/lexical_cast.hpp>

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseAttributePreferencesMode(std::vector<std::string>& argv) {
	if (argv.size() > 2) return SetError(CLIError::kTooManyArgs);
	
	// Display current mode if no args
	if (argv.size() < 2) return DoAttributePreferencesMode();

	// Set new mode
	if (!IsInteger(argv[1])) return SetError(CLIError::kIntegerExpected);
	int mode = atoi(argv[1].c_str());
	if (mode < 0) return SetError(CLIError::kIntegerMustBeNonNegative);
	if (mode > 2) return SetError(CLIError::kIntegerOutOfRange);
	return DoAttributePreferencesMode(&mode);
}

bool CommandLineInterface::DoAttributePreferencesMode(int* pMode) {

	if (m_pAgentSoar->operand2_mode) return SetError(CLIError::kSoar7Command);

	if (!pMode) {
		// query
		if (m_RawOutput) {
			m_Result << m_pAgentSoar->attribute_preferences_mode;
		} else {
			AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeInt, boost::lexical_cast< std::string >( m_pAgentSoar->attribute_preferences_mode ).c_str() );
		}
		return true;
	}

	m_pAgentSoar->attribute_preferences_mode = *pMode;

	return true;
}

