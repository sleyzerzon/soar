/////////////////////////////////////////////////////////////////
// pe-firing-mode command file.
//
// Author: Karen Coulter, karen.coulter@gmail.com
// Date  : 2007
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "sml_StringOps.h"
#include "sml_Names.h"

#include "gSKI_Agent.h"

#include "gSKI_Kernel.h"
#include "gSKI_DoNotTouch.h"
#include "gsysparam.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParsePEFiringMode(gSKI::Agent* pAgent, std::vector<std::string>& argv) {
	
	if (argv.size() > 2) return SetError(CLIError::kTooManyArgs);

	int mode = -1;
	if (argv.size() == 2) {
		if (!isdigit(argv[1][0])) {
			SetErrorDetail("Expected an integer from 0 to 2.");
			return SetError(CLIError::kIntegerOutOfRange);
		}
		mode = atoi(argv[1].c_str());
		if (mode < 0 || mode > 2) {
			SetErrorDetail("Expected an integer from 0 to 2.");
			return SetError(CLIError::kIntegerOutOfRange);
		}
	}

	return DoPEFiringMode(pAgent, mode);
}

bool CommandLineInterface::DoPEFiringMode(gSKI::Agent* pAgent, int mode) {
	if (!RequireAgent(pAgent)) return false;

	// Attain the evil back door of doom, even though we aren't the TgD, because we'll probably need it
	gSKI::EvilBackDoor::TgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	if (mode < 0) {
		// Print PEFiringMode settings.
		const long* pSysparams = pKernelHack->GetSysparams(pAgent);

		if (m_RawOutput) {
			m_Result << "\n  PE_firing_type sysparam:  ";
			switch (pSysparams[PE_FIRING_TYPE]) {
			default: // falls thru
			case PE_SINGLE_WAVE:							
				m_Result << " PE_SINGLE_WAVE (0)";
				mode = 0;
				break;
			case PE_ALLOW_CASCADE:
				m_Result << " PE_ALLOW_CASCADE (1)";
				mode = 1;
				break;
			case PE_FIRE_ALL:
				m_Result << " PE_FIRE_ALL (2)";
				mode = 2;
				break;
			}
		} else {
			char buf[kMinBufferSize];
			AppendArgTagFast(sml_Names::kParamPEFiringType, sml_Names::kTypeInt, Int2String(mode, buf, kMinBufferSize));
		}
	} else {
		switch (mode) {
			default:
				// falls through
			case 0:
				pKernelHack->SetSysparam(pAgent, PE_FIRING_TYPE, PE_SINGLE_WAVE);
				break;
			case 1:
				pKernelHack->SetSysparam(pAgent, PE_FIRING_TYPE, PE_ALLOW_CASCADE);
				break;
			case 2:
				pKernelHack->SetSysparam(pAgent, PE_FIRING_TYPE, PE_FIRE_ALL);
				break;
		}
	}

	return true;
}
