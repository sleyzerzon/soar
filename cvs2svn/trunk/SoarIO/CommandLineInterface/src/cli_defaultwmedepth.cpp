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
	// n defaults to 0 (print current value)
	int n = 0;

	if (argv.size() > 2) return SetError(CLIError::kTooManyArgs);

	// one argument, figure out if it is a positive integer
	if (argv.size() == 2) {
		n = atoi(argv[1].c_str());
		if (n <= 0) return SetError(CLIError::kIntegerMustBePositive);
	}

	return DoDefaultWMEDepth(pAgent, n);
}

bool CommandLineInterface::DoDefaultWMEDepth(gSKI::IAgent* pAgent, int n) {
	if (!RequireAgent(pAgent)) return false;

	if (!n) {
		char buf[kMinBufferSize];
		Int2String(pAgent->GetDefaultWMEDepth(), buf, kMinBufferSize);

		if (m_RawOutput) {
			AppendToResult(buf);
		} else {
			AppendArgTag(sml_Names::kParamValue, sml_Names::kTypeInt, buf);
		}
		return true;
	}

	pAgent->SetDefaultWMEDepth(n);
	return true;
}

