#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"

#include "sml_Names.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseSoarNews(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);
	unused(argv);
	return DoSoarNews();
}

/*************************************************************
* @brief soarnews command
*************************************************************/
EXPORT bool CommandLineInterface::DoSoarNews() {

	// TODO: read a file and put it here

	if (m_RawOutput) {
		m_Result << "News!";
	} else {
		AppendArgTagFast(sml_Names::kParamMessage, sml_Names::kTypeString, "News!");
	}
	return true;
}

