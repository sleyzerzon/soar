/////////////////////////////////////////////////////////////////
// excise command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "sml_StringOps.h"
#include "sml_Names.h"

#include "gSKI_Agent.h"
#include "gSKI_ProductionManager.h"
#include "IgSKI_Production.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseExcise(gSKI::Agent* pAgent, std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'a', "all",		0},
		{'c', "chunks",		0},
		{'d', "default",	0},
		{'r', "rl",			0},
		{'t', "task",		0},
		{'T', "template",	0},
		{'u', "user",		0},
		{0, 0, 0}
	};

	ExciseBitset options(0);

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
			case 'a':
				options.set(EXCISE_ALL);
				break;
			case 'c':
				options.set(EXCISE_CHUNKS);
				break;
			case 'd':
				options.set(EXCISE_DEFAULT);
				break;
			case 'r':
				options.set(EXCISE_RL);
				break;
			case 't':
				options.set(EXCISE_TASK);
				break;
			case 'T':
				options.set(EXCISE_TEMPLATE);
				break;
			case 'u':
				options.set(EXCISE_USER);
				break;
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	// If there are options, no additional argument.
	if (options.any()) {
		if (m_NonOptionArguments) return SetError(CLIError::kTooManyArgs);
		return DoExcise(pAgent, options);
	}

	// If there are no options, there must be only one production name argument
	if (m_NonOptionArguments < 1) {
		SetErrorDetail("Production name is required.");
		return SetError(CLIError::kTooFewArgs);		
	}
	if (m_NonOptionArguments > 1) {
		SetErrorDetail("Only one production name allowed, call excise multiple times to excise more than one specific production.");
		return SetError(CLIError::kTooManyArgs);		
	}

	// Pass the production to the DoExcise function
	return DoExcise(pAgent, options, &(argv[m_Argument - m_NonOptionArguments]));
}

bool CommandLineInterface::DoExcise(gSKI::Agent* pAgent, const ExciseBitset& options, const std::string* pProduction) {
	if (!RequireAgent(pAgent)) return false;

	// Acquire production manager
	gSKI::ProductionManager *pProductionManager = pAgent->GetProductionManager();
	if (!pProductionManager) {
		SetErrorDetail("Failed to get production manager.");
		return SetError(CLIError::kgSKIError);
	}

	int exciseCount = 0;

	// Process the general options
	if (options.test(EXCISE_ALL)) {
		pProductionManager->RemoveAllProductions(exciseCount);
		this->DoInitSoar(pAgent);	// from the manual, init when --all or --task are executed

		//GetAllProductions leaks ref counts, do not use.
		//ExciseInternal(pProductionManager->GetAllProductions(), exciseCount);
		//if (exciseCount) this->DoInitSoar(pAgent);	// from the manual, init when --all or --task are executed
	}
	if (options.test(EXCISE_CHUNKS)) {
		pProductionManager->RemoveAllChunks(exciseCount);
		//ExciseInternal(pProductionManager->GetChunks(), exciseCount);
		//ExciseInternal(pProductionManager->GetJustifications(), exciseCount);
	}
	if (options.test(EXCISE_DEFAULT)) {
        pProductionManager->RemoveAllDefaultProductions(exciseCount);
		//ExciseInternal(pProductionManager->GetDefaultProductions(), exciseCount);
	}
	if (options.test(EXCISE_RL)) {					// NUMERIC_INDIFFERENCE
		//pProductionManager->RemoveAllRLProductions(exciseCount);
		m_Result << "excise rl";
	}
	if (options.test(EXCISE_TASK)) {
		pProductionManager->RemoveAllUserProductions(exciseCount);
		pProductionManager->RemoveAllChunks(exciseCount);
		this->DoInitSoar(pAgent);	// from the manual, init when --all or --task are executed
		//ExciseInternal(pProductionManager->GetChunks(), exciseCount);
		//ExciseInternal(pProductionManager->GetJustifications(), exciseCount);
		//ExciseInternal(pProductionManager->GetUserProductions(), exciseCount);
		//if (exciseCount) this->DoInitSoar(pAgent);	// from the manual, init when --all or --task are executed
	}
	if (options.test(EXCISE_TEMPLATE)) {			// NUMERIC_INDIFFERENCE
		//pProductionManager->RemoveAllTemplateProductions(exciseCount);
		m_Result << "excise template";
	}
	if (options.test(EXCISE_USER)) {
		pProductionManager->RemoveAllUserProductions(exciseCount);
		//ExciseInternal(pProductionManager->GetUserProductions(), exciseCount);
	}

	// Excise specific production
	if (pProduction) {
		// Check for the production
		gSKI::tIProductionIterator* pProdIter = pProductionManager->GetProduction((*pProduction).c_str());
		if (!pProdIter->GetNumElements()) {
			SetErrorDetail("Production: " + *pProduction);
			return SetError(CLIError::kProductionNotFound);
		}

		ExciseInternal(pProdIter, exciseCount);
	}

	if (m_RawOutput) {
		m_Result << "\n" << exciseCount << " production" << (exciseCount == 1 ? " " : "s ") << "excised.";
	} else {
		// Add the count tag to the front
		char buf[kMinBufferSize];
		PrependArgTag(sml_Names::kParamCount, sml_Names::kTypeInt, Int2String(exciseCount, buf, kMinBufferSize));
	}

	return true;
}

void CommandLineInterface::ExciseInternal(gSKI::tIProductionIterator *pProdIter, int& exciseCount) {
	// Iterate through the productions using the production iterator and
	// excise and release.
	while(pProdIter->IsValid()) {
		gSKI::IProduction* ip = pProdIter->GetVal();

		if (!m_RawOutput) {
			// Save the name for the structured response
			AppendArgTagFast(sml_Names::kParamName, sml_Names::kTypeString, ip->GetName());
		}

		// Increment the count for the structured response
		++exciseCount;	// Don't *need* to outside of if above but why not.

		ip->Excise();
		ip->Release();
		pProdIter->Next();
	}
	pProdIter->Release();

}

