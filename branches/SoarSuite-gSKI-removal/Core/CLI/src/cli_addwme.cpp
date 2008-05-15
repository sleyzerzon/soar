/////////////////////////////////////////////////////////////////
// add-wme command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "cli_CLIError.h"

#include "sml_Names.h"
#include "sml_StringOps.h"

#include "sml_KernelSML.h"
#include "sml_KernelHelpers.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseAddWME(std::vector<std::string>& argv) {
	if (argv.size() < 4) return SetError(CLIError::kTooFewArgs);

	unsigned attributeIndex = (argv[2] == "^") ? 3 : 2;

	if (argv.size() < (attributeIndex + 2)) return SetError(CLIError::kTooFewArgs);
    if (argv.size() > (attributeIndex + 3)) return SetError(CLIError::kTooManyArgs);

	bool acceptable = false;
	if (argv.size() > (attributeIndex + 2)) {
		if (argv[attributeIndex + 2] != "+") {
			SetErrorDetail("Got: " + argv[attributeIndex + 2]);
			return SetError(CLIError::kAcceptableOrNothingExpected);
		}
		acceptable = true;
	}

	return DoAddWME(argv[1], argv[attributeIndex], argv[attributeIndex + 1], acceptable);
}

bool CommandLineInterface::DoAddWME(const std::string& id, const std::string& attribute, const std::string& value, bool acceptable) {
	// Need agent pointer for function calls
	if (!RequireAgent()) return false;

	// Attain the evil back door of doom, even though we aren't the TgD
	sml::KernelHelpers* pKernelHack = m_pKernelSML->GetKernelHelpers() ;

	unsigned long timetag = pKernelHack->AddWme(m_pAgentSML, id.c_str(), attribute.c_str(), value.c_str(), acceptable);
	if (timetag < 0) {
		switch (timetag) {
			default:
			case -1:
				SetErrorDetail("Got: " + id);
				return SetError(CLIError::kInvalidID);
			case -2:
				SetErrorDetail("Got: " + attribute);
				return SetError(CLIError::kInvalidAttribute);
			case -3:
				SetErrorDetail("Got: " + value);
				return SetError(CLIError::kInvalidValue);
		}
	}

	if (m_RawOutput) {
		m_Result << "Timetag: " << timetag;
	} else {
		char buf[kMinBufferSize];
		AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeInt, Int2String(timetag, buf, sizeof(buf)));
	}
	return true;
}



