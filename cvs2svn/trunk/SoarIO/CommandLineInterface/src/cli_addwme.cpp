#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"

#include "sml_Names.h"
#include "sml_StringOps.h"

#include "IgSKI_WorkingMemory.h"
#include "IgSKI_Agent.h"
#include "IgSKI_Kernel.h"
#include "IgSKI_DoNotTouch.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseAddWME(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	if (argv.size() < 4) return m_Error.SetError(CLIError::kTooFewArgs);

	unsigned attributeIndex = (argv[2] == "^") ? 3 : 2;

	if (argv.size() < (attributeIndex + 2)) return m_Error.SetError(CLIError::kTooFewArgs);
    if (argv.size() > (attributeIndex + 3)) return m_Error.SetError(CLIError::kTooManyArgs);

	bool acceptable = false;
	if (argv.size() > (attributeIndex + 2)) {
		if (argv[attributeIndex + 2] != "+") return m_Error.SetError("Expected acceptable preference (+) or nothing, got '" + argv[attributeIndex + 2] + "'.");
		acceptable = true;
	}

	return DoAddWME(pAgent, argv[1], argv[attributeIndex], argv[attributeIndex + 1], acceptable);
}

bool CommandLineInterface::DoAddWME(gSKI::IAgent* pAgent, std::string id, std::string attribute, std::string value, bool acceptable) {
	// Need agent pointer for function calls
	if (!RequireAgent(pAgent)) return false;

	// Attain the evil back door of doom, even though we aren't the TgD
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	unsigned long timetag = pKernelHack->AddWme(pAgent, id.c_str(), attribute.c_str(), value.c_str(), acceptable);
	if (timetag < 0) {
		switch (timetag) {
			case -1:
				return m_Error.SetError(CLIError::kInvalidID);
			case -2:
				return m_Error.SetError(CLIError::kInvalidAttribute);
			case -3:
				return m_Error.SetError(CLIError::kInvalidValue);
			default:
				// unspecified error
				return false;
		}
	}

	char buf[kMinBufferSize];
	if (m_RawOutput) {
		AppendToResult("Timetag: ");
		AppendToResult(Int2String(timetag, buf, sizeof(buf)));
	} else {
		AppendArgTag(sml_Names::kParamValue, sml_Names::kTypeInt, Int2String(timetag, buf, sizeof(buf)));
	}
	return true;
}



