#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"

#include "IgSKI_Agent.h"
#include "IgSKI_ProductionManager.h"
// BADBAD: I think we should be using an error class instead to work with error objects.
#include "../../gSKI/src/gSKI_Error.h"

using namespace cli;

bool CommandLineInterface::ParseSP(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	// One argument (in brackets)
	if (argv.size() < 2) return m_Error.SetError(CLIError::kTooFewArgs);
	if (argv.size() > 2) return m_Error.SetError(CLIError::kTooManyArgs);

	// Remove first and last characters (the braces)
	std::string production = argv[1];
	if (production.length() < 3) {
		return m_Error.SetError(Constants::kCLISP);
	}
	production = production.substr(1, production.length() - 2);

	return DoSP(pAgent, production);
}

bool CommandLineInterface::DoSP(gSKI::IAgent* pAgent, const std::string& production) {
	// Must have agent to give production to
	if (!RequireAgent(pAgent)) return false;

	// Acquire production manager
	gSKI::IProductionManager *pProductionManager = pAgent->GetProductionManager();

	// Load the production
	pProductionManager->AddProduction(const_cast<char*>(production.c_str()), m_pgSKIError);

	if(m_pgSKIError->Id != gSKI::gSKIERR_NONE) return m_Error.SetError(CLIError::kgSKIError);

	if (m_RawOutput) {
		AppendToResult('*');
	}
	return true;
}

