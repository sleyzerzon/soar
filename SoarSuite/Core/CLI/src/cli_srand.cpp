/////////////////////////////////////////////////////////////////
// srand command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2006
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "IgSKI_DoNotTouch.h"
#include "IgSKI_Kernel.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseSRand(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {

	unused(pAgent);

	if (argv.size() < 2) return SetError(CLIError::kTooFewArgs);
	if (argv.size() > 2) return SetError(CLIError::kTooManyArgs);

	unsigned long int seed = 0;
	sscanf(argv[1].c_str(), "%lu", &seed);

	return DoSRand(seed);
}

bool CommandLineInterface::DoSRand(unsigned long int& seed) {

	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();
	pKernelHack->SeedRandomNumberGenerator(seed);

	return true;
}

