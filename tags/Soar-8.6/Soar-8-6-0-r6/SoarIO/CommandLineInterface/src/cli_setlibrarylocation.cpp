#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"
#include "sml_Names.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseSetLibraryLocation(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	if (argv.size() > 2) {
		SetErrorDetail("Expected a path, please enclose in quotes if there are spaces in the path.");
		return SetError(CLIError::kTooManyArgs);
	}
	if (argv.size() == 2) return DoSetLibraryLocation(&(argv[1]));
	return DoSetLibraryLocation();
}

bool CommandLineInterface::DoSetLibraryLocation(const std::string* pLocation) {

	if (!pLocation || !pLocation->size()) {
		if (m_RawOutput) {
			m_Result << "Current library location: " << m_LibraryDirectory;
		} else {
			AppendArgTagFast(sml_Names::kParamDirectory, sml_Names::kTypeString, m_LibraryDirectory.c_str());
		}
	} else {
		m_LibraryDirectory = *pLocation;
	}
	return true;
}

