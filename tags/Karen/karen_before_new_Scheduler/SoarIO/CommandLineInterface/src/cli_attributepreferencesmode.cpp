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
#include "IgSKI_Agent.h"
#include "sml_Names.h"
#include "sml_StringOps.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseAttributePreferencesMode(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
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
	if (pAgent->GetOperand2Mode()) return SetError(CLIError::kSoar7Command);

	if (!pMode) {
		// query
		if (m_RawOutput) {
			m_Result << pAgent->GetAttributePreferencesMode();
		} else {
			char buf[kMinBufferSize];
			AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeInt, Int2String(pAgent->GetAttributePreferencesMode(), buf, kMinBufferSize));
		}
		return true;
	}
	
	pAgent->SetAttributePreferencesMode(*pMode);
	return true;
}
