#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#ifdef WIN32
#include <windows.h>
#else // WIN32
#include <sys/time.h>
#endif // WIN32

#include <time.h>

#include "cli_Constants.h"

#include "sml_Names.h"

#include "IgSKI_Agent.h"

#ifdef _MSC_VER
#define snprintf _snprintf 
#endif // _MSC_VER

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseTime(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	// There must at least be a command
	if (argv.size() < 2) {
		return m_Error.SetError(CLIError::kTooFewArgs);
	}

	std::vector<std::string>::iterator iter = argv.begin();
	argv.erase(iter);

	return DoTime(pAgent, argv);
}

bool CommandLineInterface::DoTime(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {

	// Look at clock
#ifdef WIN32
	DWORD realStart = GetTickCount();
#else // WIN32
	struct timeval realStart;
	if (gettimeofday(&realStart, 0) != 0) {
		return m_Error.SetError(CLIError::kgettimeofdayFail);
	}
#endif // WIN32

	// Look at clock for process time
	clock_t procStart;
	procStart = clock();

	// Execute command
	bool ret = DoCommandInternal(pAgent, argv);

	// Look at clock for process time
	clock_t procFinish;
	procFinish = clock();

	// Look at clock again, evaluate elapsed time in seconds
#ifdef WIN32
	DWORD elapsedx = GetTickCount() - realStart;
	float realElapsed = elapsedx / 1000.0f;
#else // WIN32
	struct timeval realFinish;
	if (gettimeofday(&realFinish, 0) != 0) {
		return m_Error.SetError(CLIError::kgettimeofdayFail);
	}
	double realElapsed = (realFinish.tv_sec + (realFinish.tv_usec / 1000000.0)) 
		- (realStart.tv_sec + (realStart.tv_usec / 1000000.0));
#endif

	// TODO: from clock man page:
	// The C standard allows for arbitrary values at the start of the program; 
	// subtract the value returned from a call to clock() at the start
    // of the program to get maximum portability.
	double procElapsed = (procFinish - procStart) / (double)CLOCKS_PER_SEC;


	// Print elapsed time and return
	char realBuf[32];
	memset(realBuf, 0, 32);
	snprintf(realBuf, 31, "%f", realElapsed);

	char procBuf[32];
	memset(procBuf, 0, 32);
	snprintf(procBuf, 31, "%f", procElapsed);

	if (m_RawOutput) {
		AppendToResult("\n(");
		AppendToResult(procBuf);
		AppendToResult("s) proc");
		AppendToResult("\n(");
		AppendToResult(realBuf);
		AppendToResult("s) real");
	} else {
		AppendArgTagFast(sml_Names::kParamSeconds, sml_Names::kTypeDouble, procBuf);
		AppendArgTagFast(sml_Names::kParamSeconds, sml_Names::kTypeDouble, realBuf);
	}
	return ret;
}

