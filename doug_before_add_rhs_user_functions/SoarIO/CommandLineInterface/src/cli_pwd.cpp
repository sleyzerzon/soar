#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"
#include "sml_Names.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParsePWD(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	// No arguments to print working directory
	if (argv.size() != 1) {
		return m_Error.SetError(CLIError::kTooManyArgs);
	}
	return DoPWD();
}

bool CommandLineInterface::DoPWD() {

	std::string directory;
	bool ret = GetCurrentWorkingDirectory(directory);

	if (directory.size()) {
		if (m_RawOutput) {
			AppendToResult(directory);
		} else {
			AppendArgTag(sml_Names::kParamDirectory, sml_Names::kTypeString, directory.c_str());
		}
	}

	return ret;
}

