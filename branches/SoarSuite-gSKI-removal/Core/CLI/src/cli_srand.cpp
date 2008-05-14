/////////////////////////////////////////////////////////////////
// srand command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2006
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "sml_KernelSML.h"
#include "sml_KernelHelpers.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseSRand(std::vector<std::string>& argv) {

	if (argv.size() < 2) return DoSRand();

	if (argv.size() > 2) return SetError(CLIError::kTooManyArgs);

	unsigned long int seed = 0;
	sscanf(argv[1].c_str(), "%lu", &seed);
	return DoSRand(&seed);
}

bool CommandLineInterface::DoSRand(unsigned long int* pSeed) {

	sml::KernelHelpers* pKernelHack = m_pKernelSML->GetKernelHelpers() ;

	pKernelHack->SeedRandomNumberGenerator(pSeed);
	return true;
}

