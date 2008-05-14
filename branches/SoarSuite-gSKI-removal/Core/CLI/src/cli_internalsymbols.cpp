/////////////////////////////////////////////////////////////////
// internal-symbols command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "sml_Names.h"

#include "gSKI_Kernel.h"
#include "sml_KernelSML.h"
#include "sml_KernelHelpers.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseInternalSymbols(std::vector<std::string>&) {
	return DoInternalSymbols();
}

bool CommandLineInterface::DoInternalSymbols() {

	// Need agent pointer for function calls
	if (!RequireAgent()) return false;

	// Attain the evil back door of doom, even though we aren't the TgD
	sml::KernelHelpers* pKernelHack = m_pKernelSML->GetKernelHelpers() ;
		
	AddListenerAndDisableCallbacks();
	pKernelHack->PrintInternalSymbols(m_pAgentSML);
	RemoveListenerAndEnableCallbacks();

	// put the result into a message(string) arg tag
	if (!m_RawOutput) ResultToArgTag();
	return true;
}

