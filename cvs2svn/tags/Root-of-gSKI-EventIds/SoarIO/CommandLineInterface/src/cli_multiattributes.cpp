#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"

#include "sml_Names.h"
#include "sml_StringOps.h"

#include "IgSKI_MultiAttribute.h"
#include "IgSKI_Agent.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseMultiAttributes(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	// No more than three arguments
	if (argv.size() > 3) {
		return SetError(CLIError::kTooManyArgs);
	}

	int n = 0;

	// If we have 3 arguments, third one is an integer
	if (argv.size() > 2) {
		if (!IsInteger(argv[2])) {
			// Must be an integer
			return SetError(CLIError::kIntegerMustBeNonNegative);
		}
		n = atoi(argv[2].c_str());
		if (n <= 0) {
			// Must be greater than 0
			return SetError(CLIError::kIntegerMustBeNonNegative);
		}
	}

	// If we have two arguments, second arg is an attribute/identifer/whatever
	if (argv.size() > 1) {
		return DoMultiAttributes(pAgent, &(argv[1]), n);
	} 
	return DoMultiAttributes(pAgent, 0, n);
}

bool CommandLineInterface::DoMultiAttributes(gSKI::IAgent* pAgent, std::string* pAttribute, int n) {
	if (!RequireAgent(pAgent)) return false;

	if (!pAttribute && !n) {
		// No args, print current setting
		int count = 0;
		
		// Arbitrary buffer size
		char buf[kMinBufferSize];

		gSKI::tIMultiAttributeIterator* pIt = pAgent->GetMultiAttributes();
		if (!pIt->GetNumElements()) return SetError(CLIError::kMultiAttributeNotFound);

		if (m_RawOutput) AppendToResult("Value\tSymbol");

		gSKI::IMultiAttribute* pMA;

		for(; pIt->IsValid(); pIt->Next()) {
			pMA = pIt->GetVal();

			if (m_RawOutput) {
				AppendToResult("\n");
				AppendToResult(Int2String(pMA->GetMatchingPriority(), buf, kMinBufferSize));
				AppendToResult("\t");
				AppendToResult(pMA->GetAttributeName());
			} else {
				// Value
				AppendArgTag(sml_Names::kParamValue, sml_Names::kTypeInt, Int2String(count, buf, sizeof(buf)));
				// Symbol
				AppendArgTag(sml_Names::kParamName, sml_Names::kTypeString, pMA->GetAttributeName());
			}

			++count;
			pMA->Release();
		}

		pIt->Release();

		if (!m_RawOutput) PrependArgTagFast(sml_Names::kParamCount, sml_Names::kTypeInt, Int2String(count, buf, kMinBufferSize));
		return true;
	}

	// TODO: Check whether attribute is a valid symbolic constant. The way the old kernel
	// does this is to call get_lexeme_from_string(m_agent, argv[1]) and check that
	// the lex type is symbolic constant. At this point, that functionality isn't
	// exposed by gSKI...

	// Setting defaults to 10
	if (!n) n = 10;

	// Set it
	pAgent->SetMultiAttribute(pAttribute->c_str(), n);
 	return true;
}

