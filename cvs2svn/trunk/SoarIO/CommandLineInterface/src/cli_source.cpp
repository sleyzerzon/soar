/////////////////////////////////////////////////////////////////
// source command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include <fstream>

#include "cli_Constants.h"
#include "sml_StringOps.h"
#include "sml_Names.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseSource(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'d', "disable",		0},
		{'a', "all",			0},
		{0, 0, 0}
	};

	// Set to default on first call to source
	if (m_SourceDepth == 0) {
		m_SourceMode = SOURCE_DEFAULT;
	}

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
			case 'd':
				// Only process this option on first call to source
				if (m_SourceDepth == 0) {
					m_SourceMode = SOURCE_DISABLE;
				}
				break;
			case 'a':
				// Only process this option on first call to source
				if (m_SourceDepth == 0) {
					m_SourceMode = SOURCE_ALL;
				}
				break;
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	if (m_NonOptionArguments < 1) {
		SetErrorDetail("Please supply one file to source. If there are spaces in the path, enclose it in quotes.");
		return SetError(CLIError::kTooFewArgs);

	} else if (m_NonOptionArguments > 2) {
		SetErrorDetail("Please supply one file to source. If there are spaces in the path, enclose it in quotes.");
		return SetError(CLIError::kSourceOnlyOneFile);
	}

	return DoSource(pAgent, argv[argv.size() - 1]);
}

bool CommandLineInterface::DoSource(gSKI::IAgent* pAgent, std::string filename) {
	if (!RequireAgent(pAgent)) return false;

    StripQuotes(filename);

    // Separate the path out of the filename if any
	std::string path;
	unsigned int separator1 = filename.rfind('/');
	if (separator1 != std::string::npos) {
		++separator1;
		if (separator1 < filename.length()) {
			path = filename.substr(0, separator1);
			filename = filename.substr(separator1, filename.length() - separator1);
			if (!DoPushD(path)) return false;
		}
	}
	unsigned int separator2 = filename.rfind('\\');
	if (separator2 != std::string::npos) {
		++separator2;
		if (separator2 < filename.length()) {
			path = filename.substr(0, separator2);
			filename = filename.substr(separator2, filename.length() - separator2);
			if (!DoPushD(path)) return false;
		}
	}

	// Open the file
	std::ifstream soarFile(filename.c_str());
	if (!soarFile) {
		if (path.length()) DoPopD();
		SetErrorDetail(filename);
		return SetError(CLIError::kOpenFileFail);
	}

	std::string line;				// Each line removed from the file
	std::string command;			// The command, sometimes spanning multiple lines
	std::string::size_type pos;		// Used to find braces on a line (triggering multiple line spanning commands)
	int braces = 0;					// Brace nest level (hopefully all braces are supposed to be closed)
	std::string::iterator iter;		// Iterator when parsing for braces and pounds
	int lineCount = 0;				// Count the lines per file
	int lineCountCache = 0;			// Used to save a line number
	
	static int numTotalProductionsSourced;

	// Set directory depth to zero on first call to source, even though it should be zero anyway
	if (m_SourceDepth == 0) {
		m_SourceDirDepth = 0;
		m_NumProductionsSourced = 0;		// set production number cache to zero on top level
		numTotalProductionsSourced = 0;	// set production number cache to zero on top level
	}
	++m_SourceDepth;

	// Go through each line of the file (Yay! C++ file parsing!)
	while (getline(soarFile, line)) {
	
		// Increment line count
		++lineCount;

		// Clear out the old command
		command.clear();

		// Trim whitespace and comments
		if (!Trim(line)) {
			HandleSourceError(lineCount, filename);
			if (path.length()) DoPopD();
			return false;
		}

		if (!line.length()) continue; // Nothing on line, skip it

		// If there is a brace on the line, concatenate lines until the closing brace
		pos = line.find('{');

		if (pos != std::string::npos) {
			
			// Save this line number for error messages
			lineCountCache = lineCount;

			// While we are inside braces, stay in special parsing mode
			do {
				if (lineCountCache != lineCount) {
					if (!Trim(line)) { // Trim whitespace and comments on additional lines
						HandleSourceError(lineCount, filename);
						if (path.length()) DoPopD();
						return false; 
					}
				}

				// nothing on line or just whitespace and comments
				if (!line.size()) continue;

				// Enter special parsing mode
				iter = line.begin();
				while (iter != line.end()) {
					// Go through each of the characters, counting brace nesting level
					if (*iter == '{') ++braces;
					else if (*iter == '}') --braces;

					// Next character
					++iter;
				}
				
				// We finished that line, add it to the command
				command += line;

				// Did we close all of the braces?
				if (!braces) break; // Yes, break out of special parsing mode

				// Did we go negative?
				if (braces < 0) break; // Yes, break out on error

				// Put the newline back on it (getline eats the newline)
				command += '\n';

				// We're getting another line, increment count now
				++lineCount;

				// Get the next line from the file and repeat
			} while (getline(soarFile, line));

			// Did we break out because of closed braces or EOF?
			if (braces > 0) {
				// EOF while still nested
				SetError(CLIError::kUnmatchedBrace);
				HandleSourceError(lineCountCache, filename);
				if (path.length()) DoPopD();
				return false;

			} else if (braces < 0) {
				SetError(CLIError::kExtraClosingBrace);
				HandleSourceError(lineCountCache, filename);
				if (path.length()) DoPopD();
				return false;
			}

			// We're good to go

		} else {
			// No braces on line, set command to line
			command = line;

			// Set cache to same line for error message
			lineCountCache = lineCount;
		}

		// Fire off the command
		unsigned oldResultSize = m_Result.str().size();
		if (DoCommandInternal(pAgent, command)) {
			// Add trailing newline if result changed size
			unsigned newResultSize = m_Result.str().size();
			if (oldResultSize != newResultSize) {
				// but don't add after sp's
				if (m_Result.str()[m_Result.str().size()-1] != '*') {
					m_Result << '\n';
				}
			}

		} else {
			// Command failed, error in result
			HandleSourceError(lineCountCache, filename);
			if (path.length()) DoPopD();
			return false;
		}	
	}

	// Completion
	--m_SourceDepth;

	// If mode ALL, print summary
	if (m_SourceMode == SOURCE_ALL) {
		if (m_RawOutput) {
			if (m_NumProductionsSourced) m_Result << '\n';	// add a newline if a production was sourced
			m_Result << filename << ": " << m_NumProductionsSourced << " production" << ((m_NumProductionsSourced == 1) ? " " : "s ") << "sourced.";
		} else {
			char buf[kMinBufferSize];
			AppendArgTagFast(sml_Names::kParamFilename, sml_Names::kTypeString, filename.c_str());
			AppendArgTag(sml_Names::kParamCount, sml_Names::kTypeInt, Int2String(m_NumProductionsSourced, buf, kMinBufferSize));
		}
		numTotalProductionsSourced += m_NumProductionsSourced;
		m_NumProductionsSourced = 0;	// set production number cache to zero after each summary
	}

	// if we're returning to the user
	if (!m_SourceDepth) {
		if (m_RawOutput) {
			if (m_SourceMode == SOURCE_DEFAULT) {
				if (m_NumProductionsSourced) m_Result << '\n';	// add a newline if a production was sourced
				// If default mode, print file name
				m_Result << filename << ": " << m_NumProductionsSourced << " production" 
					<< ((m_NumProductionsSourced == 1) ? " " : "s ") << "sourced.";

			} else if (m_SourceMode == SOURCE_ALL) {
				m_Result << "\nTotal: " << numTotalProductionsSourced << " production" 
					<< ((numTotalProductionsSourced == 1) ? " " : "s ") << "sourced.";
			}
		} else {
			char buf[kMinBufferSize];
			if (m_SourceMode == SOURCE_DEFAULT) {
                AppendArgTagFast(sml_Names::kParamFilename, sml_Names::kTypeString, filename.c_str());
				AppendArgTag(sml_Names::kParamCount, sml_Names::kTypeInt, Int2String(m_NumProductionsSourced, buf, kMinBufferSize));

			} else if (m_SourceMode == SOURCE_ALL) {
				AppendArgTag(sml_Names::kParamCount, sml_Names::kTypeInt, Int2String(numTotalProductionsSourced, buf, kMinBufferSize));
			}
		}

		m_NumProductionsSourced = 0;
		numTotalProductionsSourced = 0;

		// Print working directory if source directory depth !=  0
		if (m_SourceDirDepth != 0) DoPWD();	// Ignore error
		m_SourceDirDepth = 0;

		// Add finished message
		if (m_RawOutput) {
			if (m_Result.str()[m_Result.str().size()-1] != '\n') m_Result << '\n';	// add a newline if none present
			m_Result << "Source finished.";
		}
	}

	soarFile.close();
	if (path.length()) DoPopD();
	return true;
}

void CommandLineInterface::HandleSourceError(int errorLine, const std::string& filename) {
	if (!m_SourceError) {

		// Output error message
		m_SourceErrorDetail.clear();
		m_SourceErrorDetail += "\nSource command error on (or near) line ";

		char buf[kMinBufferSize];
		m_SourceErrorDetail += Int2String(errorLine, buf, kMinBufferSize);

		m_SourceErrorDetail += " of ";
		
		std::string directory;
		GetCurrentWorkingDirectory(directory); // Again, ignore error here

		m_SourceErrorDetail += filename + " (" + directory + ")";

		// PopD to original source directory
		while (m_SourceDirDepth) {
			if (m_SourceDirDepth < 0) m_SourceDirDepth = 0; // don't loop forever
			DoPopD(); // Ignore error here since it will be rare and a message confusing
		}

		// Reset depth to zero
		m_SourceDepth = 0;

		m_SourceError = true;

	} else {
		char buf[kMinBufferSize];
		m_SourceErrorDetail += "\n\t--> Sourced by: " + filename + " (line " + Int2String(errorLine, buf, kMinBufferSize) + ")";
	}
}

