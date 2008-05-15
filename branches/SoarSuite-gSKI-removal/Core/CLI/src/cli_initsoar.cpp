/////////////////////////////////////////////////////////////////
// init-soar command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "cli_CLIError.h"

#include "sml_AgentSML.h"

using namespace cli;

bool CommandLineInterface::ParseInitSoar(std::vector<std::string>&) {
	return DoInitSoar();
}

bool CommandLineInterface::DoInitSoar() {
	// Need agent pointer for function calls
	if (!RequireAgent()) return false;

	// Save the current result
	std::string oldResult = m_Result.str();

	AddListenerAndDisableCallbacks();
	bool ok = m_pAgentSML->Reinitialize() ;
	RemoveListenerAndEnableCallbacks();

	// restore the old result, ignoring output from init-soar
	m_Result.str(oldResult); 

	if (!ok)
	{
		m_Result << "Agent failed to reinitialize" ;
		return SetError(CLIError::kInitSoarFailed);
	}

	if (m_RawOutput) m_Result << "Agent reinitialized.";

	return ok;
}

