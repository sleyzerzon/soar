/////////////////////////////////////////////////////////////////
// alias command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "sml_Names.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseAlias(std::vector<std::string>& argv) {

	if (argv.size() == 1) {
		// If no arguments, list aliases
		return DoAlias();
	}

	std::vector<std::string> substitution;
	std::vector<std::string>::iterator iter = argv.begin();

	++iter;	// Skip first argument (too be aliased)
	while (++iter != argv.end()) {
		substitution.push_back(*iter);
	}

	return DoAlias(&(argv[1]), &substitution);
}

bool CommandLineInterface::DoAlias(const std::string* pCommand, const std::vector<std::string>* pSubstitution) {

	if (!pCommand && !pSubstitution) {
		// list aliases
		if (m_RawOutput) {
			std::string result = m_Aliases.List();
			if (!result.size()) {
				SetErrorDetail("No aliases in alias database.");
				return SetError(kAliasNotFound);
			}
			m_Result << result;
			return true;

		} else {
			AliasMap::const_iterator citer = m_Aliases.GetAliasMapBegin();
			while (citer != m_Aliases.GetAliasMapEnd()) {
				AppendArgTagFast(sml_Names::kParamAlias, sml_Names::kTypeString, citer->first);

				std::string aliasedCommand;
				for (std::vector<std::string>::const_iterator iter = citer->second.begin(); iter != citer->second.end(); ++iter) {
					aliasedCommand += *iter;
					aliasedCommand += ' ';
				}
				aliasedCommand = aliasedCommand.substr(0, aliasedCommand.length() - 1);
				AppendArgTagFast(sml_Names::kParamAliasedCommand, sml_Names::kTypeString, aliasedCommand);
				++citer;
			}
			return true;
		}
	}

	// command needs to have a size
	if (!pCommand || !pCommand->size()) {
		SetErrorDetail("No alias parameter received.");
		return SetError(kAliasNotFound);
	}

	if (!pSubstitution) {
		// no substitution, remove
		if (!m_Aliases.RemoveAlias(*pCommand)) {
			SetErrorDetail("Didn't find '" + *pCommand + "' in alias database.");
			return SetError(kAliasNotFound);
		}
		return true;
	} 

	// if substitution is empty, list only that alias
	if (!pSubstitution->size()) {
		if (m_RawOutput) {
			std::string result = m_Aliases.List(pCommand);
			if (!result.size()) {
				SetErrorDetail("Didn't find '" + *pCommand + "' in alias database.");
				return SetError(kAliasNotFound);
			}
			m_Result << result;
		} else {
			AliasMap::const_iterator citer = m_Aliases.GetAliasMapBegin();
			while (citer != m_Aliases.GetAliasMapEnd()) {
				if (citer->first == *pCommand) {
					AppendArgTagFast(sml_Names::kParamAlias, sml_Names::kTypeString, citer->first);

					std::string aliasedCommand;
					for (std::vector<std::string>::const_iterator iter = citer->second.begin(); iter != citer->second.end(); ++iter) {
						aliasedCommand += *iter;
						aliasedCommand += ' ';
					}
					aliasedCommand = aliasedCommand.substr(0, aliasedCommand.length() - 1);
					AppendArgTagFast(sml_Names::kParamAliasedCommand, sml_Names::kTypeString, aliasedCommand);
					break;
				}
				++citer;
			}
			if (citer == m_Aliases.GetAliasMapEnd()) {
				SetErrorDetail("Didn't find '" + *pCommand + "' in alias database.");
				return SetError(kAliasNotFound);
			}
		}
		return true;
	}

	// new alias
	m_Aliases.NewAlias((*pSubstitution), *pCommand);
	return true;
}

