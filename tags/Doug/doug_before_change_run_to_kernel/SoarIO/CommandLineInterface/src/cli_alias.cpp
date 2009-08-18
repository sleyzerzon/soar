#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"
#include "cli_GetOpt.h"

#include "sml_Names.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseAlias(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	static struct GetOpt::option longOptions[] = {
		{"disable",	1, 0, 'd'},
		{"off",		1, 0, 'd'},
		{0, 0, 0, 0}
	};

	bool disable = false;
	std::string command;

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, ":d:", longOptions, 0);
		if (option == -1) break;

		switch (option) {
			case 'd':
				disable = true;
				command = m_pGetOpt->GetOptArg();
				break;
			case ':':
				return SetError(CLIError::kMissingOptionArg);
			case '?':
				return SetError(CLIError::kUnrecognizedOption);
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	// If disabling, no additional argument.
	if (disable) {
		if (m_pGetOpt->GetAdditionalArgCount()) return SetError(CLIError::kTooManyArgs);
		return DoAlias(&command);
	}
	
	// If not disabling and no arguments, list aliases
	if (m_pGetOpt->GetAdditionalArgCount() == 0) return DoAlias();

	std::vector<std::string> substitution;
	std::vector<std::string>::iterator iter = argv.begin();

	command = *(++iter);
	while (++iter != argv.end()) {
		substitution.push_back(*iter);
	}

	return DoAlias(&command, &substitution);
}

bool CommandLineInterface::DoAlias(const std::string* pCommand, const std::vector<std::string>* pSubstitution) {

	if (!pCommand && !pSubstitution) {
		// list aliases
		if (m_RawOutput) {
			std::string result = m_Aliases.List();
			if (!result.size()) return SetError(CLIError::kAliasNotFound);
			m_Result << result;
			return true;

		} else {
			AliasMap::const_iterator citer = m_Aliases.GetAliasMapBegin();
			while (citer != m_Aliases.GetAliasMapEnd()) {
				AppendArgTagFast(sml_Names::kParamAlias, sml_Names::kTypeString, citer->first.c_str());

				std::string aliasedCommand;
				for (std::vector<std::string>::const_iterator iter = citer->second.begin(); iter != citer->second.end(); ++iter) {
					aliasedCommand += *iter;
					aliasedCommand += ' ';
				}
				aliasedCommand = aliasedCommand.substr(0, aliasedCommand.length() - 1);
				AppendArgTagFast(sml_Names::kParamAliasedCommand, sml_Names::kTypeString, aliasedCommand.c_str());
				++citer;
			}
			return true;
		}
	}

	// command needs to have a size
	if (!pCommand || !pCommand->size()) return SetError(CLIError::kAliasNotFound);

	if (!pSubstitution) {
		// no substitution, remove
		if (!m_Aliases.RemoveAlias(*pCommand)) return SetError(CLIError::kAliasNotFound);
		return true;
	} 

	// if substitution is empty, list only that alias
	if (!pSubstitution->size()) {
		// FIXME structured output
		std::string result = m_Aliases.List(pCommand);
		if (!result.size()) return SetError(CLIError::kAliasNotFound);
		m_Result << result;
		return true;
	}

	// new alias
	if (!m_Aliases.NewAlias((*pSubstitution), *pCommand)) return SetError(CLIError::kAliasExists);
	return true;
}
