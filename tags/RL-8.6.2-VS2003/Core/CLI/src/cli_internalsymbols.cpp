/////////////////////////////////////////////////////////////////
// internal-symbols command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "sml_Names.h"

#include "IgSKI_Agent.h"
#include "IgSKI_Kernel.h"
#include "IgSKI_DoNotTouch.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseInternalSymbols(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(argv);
	return DoInternalSymbols(pAgent);
}

bool CommandLineInterface::DoInternalSymbols(gSKI::IAgent* pAgent) {

	// Need agent pointer for function calls
	if (!RequireAgent(pAgent)) return false;

	// Attain the evil back door of doom, even though we aren't the TgD
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();
		
	AddListenerAndDisableCallbacks(pAgent);
	pKernelHack->PrintInternalSymbols(pAgent);
	RemoveListenerAndEnableCallbacks(pAgent);

	// put the result into a message(string) arg tag
	if (!m_RawOutput) ResultToArgTag();
	return true;
}
