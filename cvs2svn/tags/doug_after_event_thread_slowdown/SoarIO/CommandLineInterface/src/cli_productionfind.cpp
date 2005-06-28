/////////////////////////////////////////////////////////////////
// production-find command file.
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
#include "cli_GetOpt.h"
#include "sml_Names.h"

#include "IgSKI_Agent.h"
#include "IgSKI_Kernel.h"
#include "IgSKI_DoNotTouch.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseProductionFind(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	static struct GetOpt::option longOptions[] = {
		{"chunks",			0, 0, 'c'},
		{"lhs",				0, 0, 'l'},
		{"nochunks",		0, 0, 'n'},
		{"rhs",				0, 0, 'r'},
		{"show-bindings",	0, 0, 's'},
		{0, 0, 0, 0}
	};

	ProductionFindBitset options(0);

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, "clnrs", longOptions, 0);
		if (option == -1) break;

		switch (option) {
			case 'c':
				options.set(PRODUCTION_FIND_ONLY_CHUNKS);
				options.reset(PRODUCTION_FIND_NO_CHUNKS);
				break;
			case 'l':
				options.set(PRODUCTION_FIND_INCLUDE_LHS);
				break;
			case 'n':
				options.set(PRODUCTION_FIND_NO_CHUNKS);
				options.reset(PRODUCTION_FIND_ONLY_CHUNKS);
				break;
			case 'r':
				options.set(PRODUCTION_FIND_INCLUDE_RHS);
				break;
			case 's':
				options.set(PRODUCTION_FIND_SHOWBINDINGS);
				break;
			case '?':
				{
					std::string detail;
					if (m_pGetOpt->GetOptOpt()) {
						detail = static_cast<char>(m_pGetOpt->GetOptOpt());
					} else {
						detail = argv[m_pGetOpt->GetOptind() - 1];
					}
					SetErrorDetail("Bad option '" + detail + "'.");
				}
				return SetError(CLIError::kUnrecognizedOption);
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	if (m_pGetOpt->GetAdditionalArgCount()) {
		SetErrorDetail("Pattern required.");
		return SetError(CLIError::kTooFewArgs);
	}

	if (options.none()) options.set(PRODUCTION_FIND_INCLUDE_LHS);

	std::string pattern;
	for (unsigned i = m_pGetOpt->GetOptind(); i < argv.size(); ++i) {
		pattern += argv[i];
		pattern += ' ';
	}
	pattern = pattern.substr(0, pattern.length() - 1);

	return DoProductionFind(pAgent, options, pattern);
}

bool CommandLineInterface::DoProductionFind(gSKI::IAgent* pAgent, const ProductionFindBitset& options, const std::string& pattern) {
	// Need agent pointer for function calls
	if (!RequireAgent(pAgent)) return false;

	// Attain the evil back door of damnation, even though we aren't the TgD
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	AddListenerAndDisableCallbacks(pAgent);

	bool ret = pKernelHack->ProductionFind(pAgent, 
		0, 
		m_pKernel, 
		options.test(PRODUCTION_FIND_INCLUDE_LHS), 
		options.test(PRODUCTION_FIND_INCLUDE_RHS), 
		const_cast<char*>(pattern.c_str()), 
		options.test(PRODUCTION_FIND_SHOWBINDINGS), 
		options.test(PRODUCTION_FIND_ONLY_CHUNKS),
		options.test(PRODUCTION_FIND_NO_CHUNKS));
	
	RemoveListenerAndEnableCallbacks(pAgent);

	// put the result into a message(string) arg tag
	if (!m_RawOutput) ResultToArgTag();
	return ret;
}

