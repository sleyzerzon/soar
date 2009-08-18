#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"

#include "IgSKI_Agent.h"

using namespace cli;

bool CommandLineInterface::ParseInitSoar(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(argv);
	return DoInitSoar(pAgent);
}

bool CommandLineInterface::DoInitSoar(gSKI::IAgent* pAgent) {
	// Need agent pointer for function calls
	if (!RequireAgent(pAgent)) return false;

	// Simply call reinitialize
	pAgent->Halt();

	// Save the current result
	std::string oldResult = m_Result.str();

	AddListenerAndDisableCallbacks(pAgent);
	pAgent->Reinitialize();
	RemoveListenerAndEnableCallbacks(pAgent);

	// restore the old result, ignoring output from init-soar
	m_Result.str(oldResult); 

	if (m_RawOutput) m_Result << "Agent reinitialized.";
	return true;
}
