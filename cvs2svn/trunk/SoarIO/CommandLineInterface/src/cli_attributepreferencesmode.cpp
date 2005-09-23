/////////////////////////////////////////////////////////////////
// attribute-preferences command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2005
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"

#include "sml_Names.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseAttributePreferencesMode(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);
	if (argv.size() > 2) return SetError(CLIError::kTooManyArgs);
	
	// Display current mode if no args
	if (argv.size() < 2) return DoAttributePreferencesMode(pAgent);

	// Set new mode
	if (!IsInteger(argv[1])) return SetError(CLIError::kIntegerExpected);
	int mode = atoi(argv[1].c_str());
	if (mode < 0) return SetError(CLIError::kIntegerMustBeNonNegative);
	if (mode > 2) return SetError(CLIError::kIntegerOutOfRange);
	return DoAttributePreferencesMode(pAgent, &mode);
}

bool CommandLineInterface::DoAttributePreferencesMode(gSKI::IAgent* pAgent, int* pMode) {
	unused(pAgent);
	unused(pMode);
	return SetError(CLIError::kNotImplemented);
}

