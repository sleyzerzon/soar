#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"
#include "sml_Names.h"
#include "sml_StringOps.h"

#include "IgSKI_Agent.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseDefaultWMEDepth(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	// n defaults to 0 (query)
	int n = 0;

	if (argv.size() > 2) return SetError(CLIError::kTooManyArgs);

	// one argument, figure out if it is a positive integer
	if (argv.size() == 2) {
		n = atoi(argv[1].c_str());
		if (n <= 0) return SetError(CLIError::kIntegerMustBePositive);
	}

	return DoDefaultWMEDepth(pAgent, n ? &n : 0);
}

/*************************************************************
* @brief default-wme-depth command
* @param pAgent The pointer to the gSKI agent interface
* @param pDepth The pointer to the new wme depth, a positive integer.  Pass 0 (null) pointer for query.
*************************************************************/
EXPORT bool CommandLineInterface::DoDefaultWMEDepth(gSKI::IAgent* pAgent, const int* pDepth) {
	if (!RequireAgent(pAgent)) return false;

	if (!pDepth) {
		if (m_RawOutput) {
			m_Result << pAgent->GetDefaultWMEDepth();
		} else {
			char buf[kMinBufferSize];
			AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeInt, Int2String(pAgent->GetDefaultWMEDepth(), buf, kMinBufferSize));
		}
		return true;
	}

	pAgent->SetDefaultWMEDepth(*pDepth);
	return true;
}

