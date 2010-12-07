/////////////////////////////////////////////////////////////////
// source command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include <fstream>

#include "cli_Commands.h"
#include "sml_StringOps.h"
#include "sml_Names.h"
#include "sml_Events.h"
#include "cli_CLIError.h"
#include "misc.h"
#include "cli_Interp.h"

#include <algorithm>

#include <assert.h>

using namespace cli;
using namespace sml;
using namespace std;

bool CommandLineInterface::ParseSource(std::vector<std::string>& argv) 
{
	Options optionsData[] = 
    {
		{'a', "all",			OPTARG_NONE},
		{'d', "disable",		OPTARG_NONE},
		{'v', "verbose",		OPTARG_NONE},
		{0, 0, OPTARG_NONE}
	};

    SourceBitset options(0);

	for (;;) 
    {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) 
        {
			case 'd':
                options.set(SOURCE_DISABLE);
				break;
			case 'a':
                options.set(SOURCE_ALL);
				break;
			case 'v':
                options.set(SOURCE_VERBOSE);
				break;
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	if (m_NonOptionArguments < 1) 
    {
		SetErrorDetail("Please supply one file to source.");
		return SetError(CLIError::kTooFewArgs);

	} 
    else if (m_NonOptionArguments > 2) 
    {
		SetErrorDetail("Please supply one file to source. If there are spaces in the path, enclose it in quotes.");
		return SetError(CLIError::kSourceOnlyOneFile);
	}

    return DoSource(argv[m_Argument - m_NonOptionArguments], &options);
}

bool CommandHandler(std::vector<std::string>& argv, uintptr_t userData);

bool CommandLineInterface::DoSource(std::string path, SourceBitset* pOptions) {
    if (m_SourceFileStack.size() >= 100)
        return SetError(CLIError::kSourceDepthExceeded);

    StripQuotes(path);

    // Normalize separator chars.
    std::string::size_type j; 
#ifdef WIN32
    while ((j = path.find('/')) != std::string::npos)
        path.replace(j, 1, get_directory_separator());
#else
    while ((j = filename.find('\\')) != std::string::npos)
        filename.replace(j, 1, get_directory_separator());
#endif

	// Separate the path out of the filename if any
    std::string filename;
	std::string folder;
    std::string::size_type lastSeparator = path.rfind(get_directory_separator());
	if (lastSeparator == std::string::npos) 
        filename.assign(path);
    else
    {
		++lastSeparator;
		if (lastSeparator < path.length()) 
        {
			folder = path.substr(0, lastSeparator);
            filename.assign(path.substr(lastSeparator, path.length() - lastSeparator));
		}
	}

    if (!folder.empty()) if (!DoPushD(folder)) return false;

    FILE* pFile = fopen ( filename.c_str() , "rb" );
    if (!pFile) 
    {
        if (!folder.empty()) DoPopD();
        SetErrorDetail(path);
        return SetError(CLIError::kOpenFileFail);
    }

    // obtain file size:
    fseek(pFile, 0, SEEK_END);
    long lSize = ftell(pFile);
    rewind(pFile);

    // allocate memory to contain the whole file:
    char* buffer = reinterpret_cast<char*>(malloc(sizeof(char)*(lSize + 1)));
    if (!buffer) 
    {
        if (!folder.empty()) DoPopD();
        path.insert(0, "Memory allocation failed: ");
        SetErrorDetail(path);
        return SetError(CLIError::kOpenFileFail);
    }

    // copy the file into the buffer:
    size_t result = fread(buffer, 1, lSize, pFile);
    if (result != lSize)
    {
        free(buffer);
        if (!folder.empty()) DoPopD();
        path.insert(0, "Read failed: ");
        SetErrorDetail(path);
        return SetError(CLIError::kOpenFileFail);
    }
    buffer[lSize] = 0;

    // close file
    fclose (pFile);

    if (m_SourceFileStack.empty())
    {
        m_pSourceOptions = pOptions;
        m_SourceErrorDetail.clear();
    }
	m_SourceFileStack.push(path);

    cli::Interp interp;
    interp.SetHandler(CommandHandler, reinterpret_cast<uintptr_t>(this));
    bool ret = interp.Evaluate(buffer);
    if (!ret)
    {
        if (m_LastError == CLIError::kNoError)
        {
            // Parse error
            // TODO: more error detail.
            SetError(CLIError::kParseError);
        }

        m_SourceErrorDetail.append("\n\t");
        std::string temp;
        GetCurrentWorkingDirectory(temp);
        m_SourceErrorDetail.append(temp);
        m_SourceErrorDetail.append(get_directory_separator());
        m_SourceErrorDetail.append(filename);
        m_SourceErrorDetail.append(":");
        m_SourceErrorDetail.append(to_string(interp.GetLineNumber(), temp));
    }

    m_SourceFileStack.pop();
    if (m_SourceFileStack.empty())
    {
        m_pSourceOptions = 0;
        if (!m_SourceErrorDetail.empty())
            m_LastErrorDetail.append(m_SourceErrorDetail);
    }

    if (!folder.empty()) DoPopD();

    free(buffer);
    return ret;
}
