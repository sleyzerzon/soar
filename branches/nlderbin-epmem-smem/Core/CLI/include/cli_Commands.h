/////////////////////////////////////////////////////////////////
// Commands file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2006
//
// This structure contains string constants for the CLI commands.
//
/////////////////////////////////////////////////////////////////

#ifndef CLI_COMMANDS_H
#define CLI_COMMANDS_H

namespace cli {
	struct Commands {
		static char const* kCLIAddWME;
		static char const* kCLIAlias;
		static char const* kCLICaptureInput;
		static char const* kCLICD;
		static char const* kCLIChunkNameFormat;
		static char const* kCLICLog;
		static char const* kCLICommandToFile;
		static char const* kCLIDefaultWMEDepth;
		static char const* kCLIDirs;
		static char const* kCLIEcho;
		static char const* kCLIEchoCommands;
		static char const* kCLIEditProduction;
		static char const* kCLIEpMem;
		static char const* kCLIExcise;
		static char const* kCLIExplainBacktraces;
		static char const* kCLIFiringCounts;
		static char const* kCLIGDSPrint;
		static char const* kCLIGP;
		static char const* kCLIHelp;
		static char const* kCLIIndifferentSelection;
		static char const* kCLIInitSoar;
		static char const* kCLIInternalSymbols;
		static char const* kCLILearn;
		static char const* kCLILoadLibrary;
		static char const* kCLILS;
		static char const* kCLIMatches;
		static char const* kCLIMaxChunks;
		static char const* kCLIMaxElaborations;
		static char const* kCLIMaxMemoryUsage;
		static char const* kCLIMaxNilOutputCycles;
		static char const* kCLIMemories;
		static char const* kCLIMultiAttributes;
		static char const* kCLINumericIndifferentMode;
		static char const* kCLIOSupportMode;
		static char const* kCLIPopD;
		static char const* kCLIPredict;
		static char const* kCLIPreferences;
		static char const* kCLIPrint;
		static char const* kCLIProductionFind;
		static char const* kCLIPushD;
		static char const* kCLIPWatch;
		static char const* kCLIPWD;
		static char const* kCLIQuit;
		static char const* kCLIRand;
		static char const* kCLIRemoveWME;
		static char const* kCLIReplayInput;
		static char const* kCLIReteNet;
		static char const* kCLIRL;
		static char const* kCLIRun;
		static char const* kCLISaveBacktraces;
		static char const* kCLISelect;
		static char const* kCLISetLibraryLocation;
		static char const* kCLISMem;
		static char const* kCLISoarNews;
		static char const* kCLISource;
		static char const* kCLISP;
		static char const* kCLISRand;
		static char const* kCLIStats;
		static char const* kCLISetStopPhase;
		static char const* kCLIStopSoar;
		static char const* kCLITime;
		static char const* kCLITimers;
		static char const* kCLIUnalias;
		static char const* kCLIVerbose;
		static char const* kCLIVersion;
		static char const* kCLIWaitSNC;
		static char const* kCLIWarnings;
		static char const* kCLIWatch;
		static char const* kCLIWatchWMEs;
		static char const* kCLIWMA;
	}; // struct
} // namespace cli

#endif // CLI_COMMANDS_H
