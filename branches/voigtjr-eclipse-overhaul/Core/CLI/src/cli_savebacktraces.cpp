/////////////////////////////////////////////////////////////////
// save-backtraces command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "sml_Names.h"
#include "sml_StringOps.h"

#include "gSKI_Kernel.h"
#include "gSKI_DoNotTouch.h"
#include "gSKI_ProductionManager.h"
#include "IgSKI_Production.h"
#include "gsysparam.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseSaveBacktraces(gSKI::Agent* pAgent, std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'d', "disable",	0},
		{'e', "enable",		0},
		{'d', "off",		0},
		{'e', "on",			0},
		{0, 0, 0}
	};

	bool setting = true;
	bool query = true;

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
			case 'd':
				setting = false;
				query = false;
				break;
			case 'e':
				setting = true;
				query = false;
				break;
			default:
				return SetError(CLIError::kGetOptError);
		}
	}
	if (m_NonOptionArguments) return SetError(CLIError::kTooManyArgs);
	return DoSaveBacktraces(pAgent, query ? 0 : &setting);
}

bool CommandLineInterface::DoSaveBacktraces(gSKI::Agent* pAgent, bool* pSetting) {

	if (!RequireAgent(pAgent)) return false;

	// Attain the evil back door of doom, even though we aren't the TgD
	gSKI::EvilBackDoor::TgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	if (!pSetting) {
		if (m_RawOutput) {
			m_Result << "Save bactraces is " << (pKernelHack->GetSysparam(pAgent, EXPLAIN_SYSPARAM) ? "enabled." : "disabled.");
		} else {
			AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeBoolean, pKernelHack->GetSysparam(pAgent, EXPLAIN_SYSPARAM) ? sml_Names::kTrue : sml_Names::kFalse);
		}
		return true;
	}

	pKernelHack->SetSysparam(pAgent, EXPLAIN_SYSPARAM, *pSetting);
	return true;
}

