/////////////////////////////////////////////////////////////////
// sp command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"

#include "IgSKI_Agent.h"
#include "IgSKI_ProductionManager.h"

using namespace cli;

bool CommandLineInterface::ParseSP(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	// One argument (in brackets)
	if (argv.size() < 2) return SetError(CLIError::kTooFewArgs);
	if (argv.size() > 2) {
		SetErrorDetail("Expected one argument (the production) enclosed in braces.");
		return SetError(CLIError::kTooManyArgs);
	}

	// Remove first and last characters (the braces)
	std::string production = argv[1];
	if (production.length() < 3) {
		return SetError(CLIError::kInvalidProduction);
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
	this->AddListenerAndDisableCallbacks(pAgent);
	pProductionManager->AddProduction(const_cast<char*>(production.c_str()), &m_gSKIError);
	this->RemoveListenerAndEnableCallbacks(pAgent);

	if (gSKI::isError(m_gSKIError)) {
		SetErrorDetail("Error adding production.");
		return SetError(CLIError::kgSKIError);
	}

	if (m_RawOutput) {
		m_Result << '*';
	}
	return true;
}

