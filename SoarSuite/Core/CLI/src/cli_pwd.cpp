/////////////////////////////////////////////////////////////////
// pwd command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"
#include "cli_CLIError.h"

#include "cli_Commands.h"
#include "sml_Names.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParsePWD(std::vector<std::string>& argv) {
	// No arguments to print working directory
	if (argv.size() != 1) {
		SetErrorDetail("pwd takes no arguments.");
		return SetError(CLIError::kTooManyArgs);
	}
	return DoPWD();
}

bool CommandLineInterface::DoPWD() {

	std::string directory;
	bool ret = GetCurrentWorkingDirectory(directory);

	if (directory.size()) {
		if (m_RawOutput) {
			m_Result << directory;
		} else {
			AppendArgTagFast(sml_Names::kParamDirectory, sml_Names::kTypeString, directory.c_str());
		}
	}

	return ret;
}

