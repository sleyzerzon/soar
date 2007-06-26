/////////////////////////////////////////////////////////////////
// remove-wme command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "sml_Names.h"
#include "sml_StringOps.h"

#include "IgSKI_WorkingMemory.h"
#include "gSKI_Kernel.h"
#include "gSKI_DoNotTouch.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseRemoveWME(gSKI::Agent* pAgent, std::vector<std::string>& argv) {
	// Exactly one argument
	if (argv.size() < 2) {
		SetErrorDetail("Please supply a timetag.");
		return SetError(CLIError::kTooFewArgs);
	}
	if (argv.size() > 2) {
		SetErrorDetail("Please supply only one timetag.");
		return SetError(CLIError::kTooManyArgs);
	}

	int timetag = atoi(argv[1].c_str());
	if (!timetag) return SetError(CLIError::kIntegerMustBePositive);

	return DoRemoveWME(pAgent, timetag);
}

bool CommandLineInterface::DoRemoveWME(gSKI::Agent* pAgent, int timetag) {
	// Need agent pointer for function calls
	if (!RequireAgent(pAgent)) return false;

	// Attain the evil back door of doom, even though we aren't the TgD
	gSKI::EvilBackDoor::TgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	if (pKernelHack->RemoveWmeByTimetag(pAgent, timetag)) return SetError(CLIError::kRemoveWMEFailed);
	return true;
}

