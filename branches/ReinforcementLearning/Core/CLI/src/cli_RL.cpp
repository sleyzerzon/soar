#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "sml_Names.h"

#include "IgSKI_Agent.h"

 

#include "cli_Commands.h"

#include "IgSKI_Kernel.h"
#include "IgSKI_DoNotTouch.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseRL(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'e', "enable",		0},
		{'e',"on",			0},
		{'d', "disable",	0},
		{'d',"off",			0},
		{'a',"alpha",       1},
		{'g',"gamma",		1},
		{0, 0, 0}
	};

	int RLSetting = 0;
	double alpha = -1;
	double gamma = -1;

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

	switch (m_Option) {
	case 'e'://on
		RLSetting = 1;
		break;
	case 'd'://off
		RLSetting = 2;
		break;
	case 'a'://alpha
		if (m_OptionArgument.size()) {
			alpha = strtod(m_OptionArgument.c_str(),0);
			if (alpha < 0) return SetError(CLIError::kAlphaMustBeNonNegative);
		}
		break;
	case 'g'://gamma
		if (m_OptionArgument.size()) {
			gamma = strtod(m_OptionArgument.c_str(),0);
			if ((gamma < 0) || (gamma > 1)) return SetError(CLIError::kGammaOutsideUnitInterval);
		}
		break;
		
	default:
		return SetError(CLIError::kGetOptError);
		}
	}

	if (m_NonOptionArguments) return SetError(CLIError::kTooManyArgs);

	return DoRL(pAgent, RLSetting, alpha, gamma);
}

bool CommandLineInterface::DoRL(gSKI::IAgent* pAgent, const int RLSetting, const double alpha, const double gamma) {
	// Need agent pointer for function calls
	if (!RequireAgent(pAgent)) return false;

	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	
	if (alpha >= 0){
		pAgent->SetAlpha(alpha);
		return true;
	} 
	if (gamma >= 0){
		pAgent->SetGamma(gamma);
		return true;
	}
	if (RLSetting == 1) {
		pAgent->SetRL(true);
		pKernelHack->SetSysparam(pAgent, RL_ON_SYSPARAM, true);
		return true;
	}
	if (RLSetting == 2) {
		pAgent->SetRL(false);
		pKernelHack->SetSysparam(pAgent, RL_ON_SYSPARAM, false);
		pKernelHack->ResetRL(pAgent);
		return true;
	}
	
	if (m_RawOutput) {
			m_Result << "Current RL settings: ";
			if (pAgent->IsRLOn()) {
				m_Result << "Reinforcement learning is enabled. \n";
				m_Result << "alpha: " << pAgent->GetAlpha();
				m_Result << " gamma: " << pAgent->GetGamma();
			} else {
				m_Result << "Reinforcement learning is disabled."; 
			}
		}
	return true;	
}
