#include "cli_CommandLineInterface.h"

using namespace cli;
using namespace sml;

// ____                     _   _      _
//|  _ \ __ _ _ __ ___  ___| | | | ___| |_ __
//| |_) / _` | '__/ __|/ _ \ |_| |/ _ \ | '_ \
//|  __/ (_| | |  \__ \  __/  _  |  __/ | |_) |
//|_|   \__,_|_|  |___/\___|_| |_|\___|_| .__/
//                                      |_|
bool CommandLineInterface::ParseHelp(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	if (argv.size() > 2) {
		return HandleSyntaxError(Constants::kCLIHelp, Constants::kCLITooManyArgs);
	}

	if (argv.size() == 2) {
		return DoHelp(&(argv[1]));
	}
	return DoHelp();
}

// ____        _   _      _
//|  _ \  ___ | | | | ___| |_ __
//| | | |/ _ \| |_| |/ _ \ | '_ \
//| |_| | (_) |  _  |  __/ | |_) |
//|____/ \___/|_| |_|\___|_| .__/
//                         |_|
bool CommandLineInterface::DoHelp(std::string* pCommand) {
	std::string output;

	if (!m_Constants.IsUsageFileAvailable()) {
		return HandleError(Constants::kCLINoUsageFile);
	}

	if (pCommand) {
		if (!m_Constants.GetUsageFor(*pCommand, output)) {
			return HandleError("Help for command '" + *pCommand + "' not found.");
		}
		m_Result += output;
		return true;
	}
	m_Result += "Help is available for the following commands:\n";
	std::list<std::string> commandList = m_Constants.GetCommandList();
	std::list<std::string>::const_iterator iter = commandList.begin();

	int i = 0;
	int tabs;
	while (iter != commandList.end()) {
		m_Result += *iter;
		if (m_CommandMap.find(*iter) == m_CommandMap.end()) {
			m_Result += '*';
		} else {
			m_Result += ' ';
		}
		tabs = (40 - (*iter).length() - 2) / 8; 
		if (i % 2) {
			m_Result += "\n";
		} else {
			do {
				m_Result += '\t';
			} while (--tabs > 0);
		}
		++iter;
		++i;
	}
	if (i % 2) {
		m_Result += '\n';
	}
	m_Result += "Type 'help' followed by the command name for help on a specific command.\n";
	m_Result += "A Star (*) indicates the command is not yet implemented.";
	return true;
}

