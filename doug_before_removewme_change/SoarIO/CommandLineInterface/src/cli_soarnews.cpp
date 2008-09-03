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

bool CommandLineInterface::DoSoarNews() {

	m_Result << "This is Soar ";
	
	// Make DoVersion print its result to the result string
	bool rawOutputSave = m_RawOutput;
	m_RawOutput = true;
	DoVersion();
	m_RawOutput = rawOutputSave;

	m_Result << ".\n";
	m_Result << "Please visit the Soar home page at\n\thttp://sitemaker.umich.edu/soar/\n";
	m_Result << "Please see the readme file included with this release for more information.";

	if (!m_RawOutput) ResultToArgTag();
	return true;
}

