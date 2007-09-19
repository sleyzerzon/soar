/////////////////////////////////////////////////////////////////
// stop-soar command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "gSKI_Agent.h"
#include "gSKI_AgentManager.h"
#include "gSKI_Kernel.h"
#include "sml_KernelSML.h"

using namespace cli;

bool CommandLineInterface::ParseStopSoar(gSKI::Agent* pAgent, std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'s', "self",		0},
		{0, 0, 0}
	};

	bool self = false;

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
			case 's':
				self = true;
				break;
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	// Concatinate remaining args for 'reason'
	if (m_NonOptionArguments) {
		std::string reasonForStopping;
		unsigned int optind = m_Argument - m_NonOptionArguments;
		while (optind < argv.size()) reasonForStopping += argv[optind++] + ' ';
		return DoStopSoar(pAgent, self, &reasonForStopping);
	}
	return DoStopSoar(pAgent, self);
}

bool CommandLineInterface::DoStopSoar(gSKI::Agent* pAgent, bool self, const std::string* reasonForStopping) {

	unused(reasonForStopping);

	if (self) {
		if (!RequireAgent(pAgent)) return false;
		if (!pAgent->Interrupt(gSKI_STOP_AFTER_DECISION_CYCLE, gSKI_STOP_BY_RETURNING, &m_gSKIError)) {
			SetErrorDetail("Error interrupting agent.");
			return SetError(CLIError::kgSKIError);
		}
		if (gSKI::isError(m_gSKIError)) return SetError(CLIError::kgSKIError);
		return true;
	} else {
		if (!RequireKernel()) return false;

		// Make sure the system stop event will be fired at the end of the run.
		// We used to call FireSystemStop() in this function, but that's no good because
		// it comes before the agent has stopped because interrupt only stops at the next
		// phase or similar boundary (so could be a long time off).
		// So instead we set a flag and allow system stop to fire at the end of the run.
		m_pKernelSML->RequireSystemStop(true) ;

		if (!m_pKernel->GetAgentManager()->InterruptAll(gSKI_STOP_AFTER_DECISION_CYCLE, &m_gSKIError)) {
			SetErrorDetail("Error interrupting all agents.");
			return SetError(CLIError::kgSKIError);
		}
		if (gSKI::isError(m_gSKIError)) return SetError(CLIError::kgSKIError);
		return true;
	}
}

/////////////////////////////////////////////////////////////////
// struct-to-string command file
//
// Author: Yongjia Wang
// Date  : 2007-08-06
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"
#include "sml_Names.h"
#include "gSKI_Agent.h"
#include "gSKI_DoNotTouch.h"
#include "gSKI_Kernel.h"

using namespace cli;
using namespace sml;
using namespace std;
using namespace gSKI;


bool StringToInt(std::string str, int& n){
	std::istringstream str2int(str);
	if (str2int >> n) {
		return true;
	}
	else{
		return false;
	}
}
bool CommandLineInterface::ParseStructToString(gSKI::Agent* pAgent, std::vector<std::string>& argv) {
	
	if(argv.size() < 2){
		m_Result << "No input identifier";
	}
	string struct_id = argv.at(1);
	string additional_feature = "";
	string excluded_feature = "";
	string equivalent_symbols_id = "";
	string identifier_symbols_id = "";
	
	int current_index = 2;
	while(argv.size() > current_index + 1){
		string option = argv[current_index];
		string value = argv[current_index+1];

		if(option == "-e"){
			excluded_feature = value;
		}
		else if(option == "-a"){
			additional_feature = value;
		}
		else if(option == "-i"){
			equivalent_symbols_id = value;
		}
		else if(option == "-s"){
			identifier_symbols_id = value;
		}
		current_index += 2;
	}
	

	// Attain the evil back door of doom, even though we aren't the TgD
	gSKI::EvilBackDoor::TgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();
	for(std::vector<string>::iterator itr = argv.begin(); itr != argv.end(); ++itr){
		//cout << *itr << endl;
	}
	if(struct_id.length() >= 3 && struct_id[0] =='|' && struct_id[struct_id.length() - 1] == '|'){
		struct_id = struct_id.substr(1, struct_id.length() - 2);
	}
	
	if(equivalent_symbols_id.length() >= 3 && equivalent_symbols_id[0] =='|' && equivalent_symbols_id[equivalent_symbols_id.length() - 1] == '|'){
		equivalent_symbols_id = equivalent_symbols_id.substr(1, equivalent_symbols_id.length() - 2);
	}

	if(identifier_symbols_id.length() >= 3 && identifier_symbols_id[0] =='|' && identifier_symbols_id[identifier_symbols_id.length() - 1] == '|'){
		identifier_symbols_id = identifier_symbols_id.substr(1, identifier_symbols_id.length() - 2);
	}
	int input_int;
	
	if(StringToInt(struct_id, input_int)){
		string gs_str = pKernelHack->IdToStruct(pAgent, input_int);
		m_Result << struct_id << endl;
		m_Result << gs_str;
	}
	else{
		string hash_str = pKernelHack->StructToString(pAgent, struct_id, additional_feature, excluded_feature, equivalent_symbols_id, identifier_symbols_id);
		m_Result << hash_str;
	}
	
	return true;
}
