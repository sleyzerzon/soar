/////////////////////////////////////////////////////////////////
// pushd command file.
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

using namespace cli;

bool CommandLineInterface::ParsePushD(std::vector<std::string>& argv) {
	// Only takes one argument, the directory to change into
	if (argv.size() < 2) {
		SetErrorDetail("Expected directory to change to.");
		return SetError(CLIError::kTooFewArgs);
	}
	if (argv.size() > 2) {
		SetErrorDetail("Expected directory to change to, enclose in quotes if there are spaces in the path.");
		return SetError(CLIError::kTooManyArgs);
	}
	return DoPushD(argv[1]);
}

bool CommandLineInterface::DoPushD(const std::string& directory) {
	
	// Sanity check thanks to rchong
	if (directory.length() == 0) return true;

	// Target directory required, checked in DoCD call.

	// Save the current (soon to be old) directory
	std::string oldDirectory;
	if (!GetCurrentWorkingDirectory(oldDirectory)) return false;// Error message handled in function

	// Change to the new directory.
	if (!DoCD(&directory)) return false;// Error message handled in function

	// If we're sourcing, this will be non-negative
	if (m_SourceDirDepth >= 0) ++m_SourceDirDepth;// And if it is, increment it for each dir placed on the stack

	// Directory change successful, store old directory and move on
	m_DirectoryStack.push(oldDirectory);
	return true;
}

