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
#include "sml_StringOps.h"

#include "IgSKI_Agent.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseTime(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	// There must at least be a command
	if (argv.size() < 2) {
		return SetError(CLIError::kTooFewArgs);
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
		return SetError(CLIError::kgettimeofdayFail);
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
		return SetError(CLIError::kgettimeofdayFail);
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
	char buf[kMinBufferSize];
	if (m_RawOutput) {
		m_ResultStream << "\n(" << procElapsed << "s) proc" << "\n(" << realElapsed << "s) real";
	} else {
		AppendArgTag(sml_Names::kParamSeconds, sml_Names::kTypeDouble, Double2String(realElapsed, buf, kMinBufferSize));
		AppendArgTag(sml_Names::kParamSeconds, sml_Names::kTypeDouble, Double2String(procElapsed, buf, kMinBufferSize));
	}
	return ret;
}

