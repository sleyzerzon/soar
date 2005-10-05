/////////////////////////////////////////////////////////////////
// CommandLineInterface class file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#ifdef WIN32
#include <direct.h>
#include <Windows.h>
#endif // WIN32

#include <assert.h>

#include "cli_Constants.h"
#include "cli_Aliases.h"

#include "../../gSKI/src/gSKI_Error.h"
#include "IgSKI_Agent.h"

// SML includes
#include "sml_Connection.h"
#include "sml_TagResult.h"
#include "sml_TagArg.h"
#include "sml_StringOps.h"
#include "sml_KernelSML.h"
#include "sml_AgentSML.h"

using namespace cli;
using namespace sml;

std::ostringstream CommandLineInterface::m_Result;	

EXPORT CommandLineInterface::CommandLineInterface() {

	// Map command names to processing function pointers
	m_CommandMap[Constants::kCLIAddWME]					= &cli::CommandLineInterface::ParseAddWME;
	m_CommandMap[Constants::kCLIAlias]					= &cli::CommandLineInterface::ParseAlias;
	m_CommandMap[Constants::kCLIAttributePreferencesMode]	= &cli::CommandLineInterface::ParseAttributePreferencesMode;
	m_CommandMap[Constants::kCLICD]						= &cli::CommandLineInterface::ParseCD;
	m_CommandMap[Constants::kCLIChunkNameFormat]		= &cli::CommandLineInterface::ParseChunkNameFormat;
	m_CommandMap[Constants::kCLIDefaultWMEDepth]		= &cli::CommandLineInterface::ParseDefaultWMEDepth;
	m_CommandMap[Constants::kCLIDirs]					= &cli::CommandLineInterface::ParseDirs;
	m_CommandMap[Constants::kCLIEcho]					= &cli::CommandLineInterface::ParseEcho;
	m_CommandMap[Constants::kCLIEditProduction]			= &cli::CommandLineInterface::ParseEditProduction;
	m_CommandMap[Constants::kCLIExcise]					= &cli::CommandLineInterface::ParseExcise;
	m_CommandMap[Constants::kCLIExplainBacktraces]		= &cli::CommandLineInterface::ParseExplainBacktraces;
	m_CommandMap[Constants::kCLIFiringCounts]			= &cli::CommandLineInterface::ParseFiringCounts;
	m_CommandMap[Constants::kCLIGDSPrint]				= &cli::CommandLineInterface::ParseGDSPrint;
	m_CommandMap[Constants::kCLIHelp]					= &cli::CommandLineInterface::ParseHelp;
	m_CommandMap[Constants::kCLIIndifferentSelection]	= &cli::CommandLineInterface::ParseIndifferentSelection;
	m_CommandMap[Constants::kCLIInitSoar]				= &cli::CommandLineInterface::ParseInitSoar;
	m_CommandMap[Constants::kCLIInputPeriod]			= &cli::CommandLineInterface::ParseInputPeriod;
	m_CommandMap[Constants::kCLIInternalSymbols]		= &cli::CommandLineInterface::ParseInternalSymbols;
	m_CommandMap[Constants::kCLILearn]					= &cli::CommandLineInterface::ParseLearn;
	m_CommandMap[Constants::kCLILog]					= &cli::CommandLineInterface::ParseLog;
	m_CommandMap[Constants::kCLILS]						= &cli::CommandLineInterface::ParseLS;
	m_CommandMap[Constants::kCLIMatches]				= &cli::CommandLineInterface::ParseMatches;
	m_CommandMap[Constants::kCLIMaxChunks]				= &cli::CommandLineInterface::ParseMaxChunks;
	m_CommandMap[Constants::kCLIMaxElaborations]		= &cli::CommandLineInterface::ParseMaxElaborations;
	m_CommandMap[Constants::kCLIMaxNilOutputCycles]		= &cli::CommandLineInterface::ParseMaxNilOutputCycles;
	m_CommandMap[Constants::kCLIMemories]				= &cli::CommandLineInterface::ParseMemories;
	m_CommandMap[Constants::kCLIMultiAttributes]		= &cli::CommandLineInterface::ParseMultiAttributes;
	m_CommandMap[Constants::kCLINumericIndifferentMode]	= &cli::CommandLineInterface::ParseNumericIndifferentMode;
	m_CommandMap[Constants::kCLIOSupportMode]			= &cli::CommandLineInterface::ParseOSupportMode;
	m_CommandMap[Constants::kCLIPopD]					= &cli::CommandLineInterface::ParsePopD;
	m_CommandMap[Constants::kCLIPreferences]			= &cli::CommandLineInterface::ParsePreferences;
	m_CommandMap[Constants::kCLIPrint]					= &cli::CommandLineInterface::ParsePrint;
	m_CommandMap[Constants::kCLIProductionFind]			= &cli::CommandLineInterface::ParseProductionFind;
	m_CommandMap[Constants::kCLIPushD]					= &cli::CommandLineInterface::ParsePushD;
	m_CommandMap[Constants::kCLIPWatch]					= &cli::CommandLineInterface::ParsePWatch;
	m_CommandMap[Constants::kCLIPWD]					= &cli::CommandLineInterface::ParsePWD;
	m_CommandMap[Constants::kCLIQuit]					= &cli::CommandLineInterface::ParseQuit;
	m_CommandMap[Constants::kCLIRemoveWME]				= &cli::CommandLineInterface::ParseRemoveWME;
	m_CommandMap[Constants::kCLIReteNet]				= &cli::CommandLineInterface::ParseReteNet;
	m_CommandMap[Constants::kCLIRun]					= &cli::CommandLineInterface::ParseRun;
	m_CommandMap[Constants::kCLISaveBacktraces]			= &cli::CommandLineInterface::ParseSaveBacktraces;
	m_CommandMap[Constants::kCLISetLibraryLocation]		= &cli::CommandLineInterface::ParseSetLibraryLocation;
	m_CommandMap[Constants::kCLISoar8]					= &cli::CommandLineInterface::ParseSoar8;
	m_CommandMap[Constants::kCLISoarNews]				= &cli::CommandLineInterface::ParseSoarNews;
	m_CommandMap[Constants::kCLISource]					= &cli::CommandLineInterface::ParseSource;
	m_CommandMap[Constants::kCLISP]						= &cli::CommandLineInterface::ParseSP;
	m_CommandMap[Constants::kCLIStats]					= &cli::CommandLineInterface::ParseStats;
	m_CommandMap[Constants::kCLISetStopPhase]			= &cli::CommandLineInterface::ParseSetStopPhase;
	m_CommandMap[Constants::kCLIStopSoar]				= &cli::CommandLineInterface::ParseStopSoar;
	m_CommandMap[Constants::kCLITime]					= &cli::CommandLineInterface::ParseTime;
	m_CommandMap[Constants::kCLITimers]					= &cli::CommandLineInterface::ParseTimers;
	m_CommandMap[Constants::kCLIVerbose]				= &cli::CommandLineInterface::ParseVerbose;
	m_CommandMap[Constants::kCLIVersion]				= &cli::CommandLineInterface::ParseVersion;
	m_CommandMap[Constants::kCLIWaitSNC]				= &cli::CommandLineInterface::ParseWaitSNC;
	m_CommandMap[Constants::kCLIWarnings]				= &cli::CommandLineInterface::ParseWarnings;
	m_CommandMap[Constants::kCLIWatch]					= &cli::CommandLineInterface::ParseWatch;
	m_CommandMap[Constants::kCLIWatchWMEs]				= &cli::CommandLineInterface::ParseWatchWMEs;

	m_EchoMap[Constants::kCLIAddWME]					= true ;
	m_EchoMap[Constants::kCLIAlias]						= true ;
	m_EchoMap[Constants::kCLIAttributePreferencesMode]	= true ;
	m_EchoMap[Constants::kCLICD]						= true ;
	m_EchoMap[Constants::kCLIChunkNameFormat]			= true ;
	m_EchoMap[Constants::kCLIDefaultWMEDepth]			= true ;
	m_EchoMap[Constants::kCLIEcho]						= true ;
	m_EchoMap[Constants::kCLIExcise]					= true ;
	m_EchoMap[Constants::kCLIIndifferentSelection]		= true ;
	m_EchoMap[Constants::kCLIInitSoar]					= true ;
	m_EchoMap[Constants::kCLIInputPeriod]				= true ;
	m_EchoMap[Constants::kCLILearn]						= true ;
	m_EchoMap[Constants::kCLILog]						= true ;
	m_EchoMap[Constants::kCLIMaxChunks]					= true ;
	m_EchoMap[Constants::kCLIMaxElaborations]			= true ;
	m_EchoMap[Constants::kCLIMaxNilOutputCycles]		= true ;
	m_EchoMap[Constants::kCLIMultiAttributes]			= true ;
	m_EchoMap[Constants::kCLINumericIndifferentMode]	= true ;
	m_EchoMap[Constants::kCLIOSupportMode]				= true ;
	m_EchoMap[Constants::kCLIPopD]						= true ;
	m_EchoMap[Constants::kCLIPushD]						= true ;
	m_EchoMap[Constants::kCLIQuit]						= true ;
	m_EchoMap[Constants::kCLIRemoveWME]					= true ;
	m_EchoMap[Constants::kCLIReteNet]					= true ;
	m_EchoMap[Constants::kCLIRun]						= true ;
	m_EchoMap[Constants::kCLISetLibraryLocation]		= true ;
	m_EchoMap[Constants::kCLISoar8]						= true ;
	m_EchoMap[Constants::kCLISource]					= true ;
	m_EchoMap[Constants::kCLISP]						= true ;
	m_EchoMap[Constants::kCLISetStopPhase]				= true ;
	m_EchoMap[Constants::kCLIStopSoar]					= true ;
	m_EchoMap[Constants::kCLITimers]					= true ;
	m_EchoMap[Constants::kCLIVerbose]					= true ;
	m_EchoMap[Constants::kCLIWaitSNC]					= true ;
	m_EchoMap[Constants::kCLIWatch]						= true ;
	m_EchoMap[Constants::kCLIWatchWMEs]					= true ;

	// Set library directory to sane default value
	GetCurrentWorkingDirectory(m_LibraryDirectory);

	// Initialize other members
	m_pKernel = 0;
	m_SourceError = false;
	m_SourceDepth = 0;
	m_SourceDirDepth = 0;
	m_pLogFile = 0;
	m_KernelVersion.major = m_KernelVersion.minor = 0;
	m_LastError = CLIError::kNoError;
	gSKI::ClearError(&m_gSKIError);
	m_Initialized = true;
	m_PrintEventToResult = false;
	m_EchoResult = false ;
	m_pAgentSML = 0 ;
}

EXPORT CommandLineInterface::~CommandLineInterface() {
	if (m_pLogFile) {
		(*m_pLogFile) << "Log file closed due to shutdown." << std::endl;
		delete m_pLogFile;
	}
}

/*************************************************************
* @brief Returns true if the given command should always be echoed (to any listeners)
*        The current implementation doesn't support aliases or short forms of the commands.
* @param pCommandLine	The command line being tested
*************************************************************/
EXPORT bool CommandLineInterface::ShouldEchoCommand(char const* pCommandLine)
{
	if (!pCommandLine)
		return false ;

	std::string command = pCommandLine ;

	char const* pSpace = strchr(pCommandLine, ' ') ;
	if (pSpace)
	{
		// Trim everything from space on
		command.erase(pSpace-pCommandLine, command.length()) ;
	}

	// See if there's an entry in the echo map for this command
	// BADBAD: This won't work for short forms of the command or aliases; but making this test
	// happen later in the command line processing causes too many re-entrancy problem within the command line module.
	return (m_EchoMap.find(command) != m_EchoMap.end()) ;
}

/*************************************************************
* @brief Process a command.  Give it a command line and it will parse
*		 and execute the command using gSKI or system calls.
* @param pConnection The connection, for communication to the client
* @param pAgent The pointer to the gSKI agent interface
* @param pCommandLine The command line string, arguments separated by spaces
* @param echoResults If true send a copy of the result to the echo event
* @param pResponse Pointer to XML response object
*************************************************************/
EXPORT bool CommandLineInterface::DoCommand(Connection* pConnection, gSKI::IAgent* pAgent, const char* pCommandLine, bool echoResults, ElementXML* pResponse) {
	// No way to return data
	if (!pConnection) return false;
	if (!pResponse) return false;

	// Log input
	if (m_pLogFile) {
		if (pAgent) (*m_pLogFile) << pAgent->GetName() << "> ";
		(*m_pLogFile) << pCommandLine << endl;
	}

	m_EchoResult = echoResults ;
	m_pAgentSML = m_pKernelSML->GetAgentSML(pAgent) ;

	// Process the command, ignoring its result (errors detected with m_LastError)
	DoCommandInternal(pAgent, pCommandLine);

	GetLastResultSML(pConnection, pResponse);

	// Always returns true to indicate that we've generated any needed error message already
	return true;
}

void CommandLineInterface::GetLastResultSML(sml::Connection* pConnection, sml::ElementXML* pResponse) {
	assert(pConnection);
	assert(pResponse);

	// Handle source error output
	if (m_SourceError) {
		//SetError(CLIError::kSourceError);
		SetErrorDetail(m_SourceErrorDetail);
		m_SourceError = false;
	}

	if (m_LastError == CLIError::kNoError) {
        // Log output
        if (m_pLogFile) (*m_pLogFile) << m_Result.str() << endl;

        // The command succeeded, so return the result if raw output
		if (m_RawOutput) {
			pConnection->AddSimpleResultToSMLResponse(pResponse, m_Result.str().c_str());
			EchoString(pConnection, m_Result.str().c_str()) ;
		} else {
			// If there are tags in the response list, add them and return
			if (m_ResponseTags.size()) {
				TagResult* pTag = new TagResult();

				ElementXMLListIter iter = m_ResponseTags.begin();
				while (iter != m_ResponseTags.end()) {
					pTag->AddChild(*iter);
					m_ResponseTags.erase(iter);
					iter = m_ResponseTags.begin();
				}

				pResponse->AddChild(pTag);

			} else {
				// Otherwise, return result as simple result if there is one
				if (m_Result.str().size()) {
					pConnection->AddSimpleResultToSMLResponse(pResponse, m_Result.str().c_str());
					EchoString(pConnection, m_Result.str().c_str()) ;
				} else {
					// Or, simply return true
					pConnection->AddSimpleResultToSMLResponse(pResponse, sml_Names::kTrue);
				}
			}
		}
	} else {
		// The command failed, add the error message
		string errorDescription = CLIError::GetErrorDescription(m_LastError);
		if (m_LastErrorDetail.size()) {
			errorDescription += "\nError detail: ";
			errorDescription += m_LastErrorDetail;
		}
		if (m_Result.str().size()) {
			errorDescription += "\nResult before error happened:\n";
			errorDescription += m_Result.str();
		}
		if (gSKI::isError(m_gSKIError)) {
			errorDescription += "\ngSKI Error code: ";
			char buf[kMinBufferSize];
			Int2String(m_gSKIError.Id, buf, kMinBufferSize);
			errorDescription += buf;
			errorDescription += "\ngSKI Error text: ";
			errorDescription += m_gSKIError.Text;
			errorDescription += "\ngSKI Error details: ";
			errorDescription += m_gSKIError.ExtendedMsg;
		}
		pConnection->AddErrorToSMLResponse(pResponse, errorDescription.c_str(), m_LastError);
		EchoString(pConnection, errorDescription.c_str()) ;

        // Log error
        if (m_pLogFile) (*m_pLogFile) << errorDescription << endl;
	}

	// reset state
	m_Result.str("");
	m_ResponseTags.clear();	
	m_LastError = CLIError::kNoError;	
	m_LastErrorDetail.clear();			
	gSKI::ClearError(&m_gSKIError);
}

/************************************************************* 	 
* @brief Echo the given string through the smlEVENT_ECHO event
*		 if the call requested that commands be echoed.
*************************************************************/ 	 
void CommandLineInterface::EchoString(sml::Connection* pConnection, char const* pString)
{
	if (!m_EchoResult)
		return ;

	// BUGBUG: We may need to support this for kernel level commands without an agent
	if (m_pAgentSML)
		m_pAgentSML->FireEchoEvent(pConnection, pString) ;
}

/*************************************************************
* @brief Takes a command line and expands any aliases and returns
*		 the result.  The command is NOT executed.
* @param pConnection The connection, for communication to the client
* @param pCommandLine The command line string, arguments separated by spaces
* @param pResponse Pointer to XML response object
*************************************************************/
EXPORT bool CommandLineInterface::ExpandCommand(sml::Connection* pConnection, const char* pCommandLine, sml::ElementXML* pResponse)
{
	SetError(CLIError::kNoError);

	std::string result ;
	vector<string> argv;

	// 1) Parse command
	if (Tokenize(pCommandLine, argv) == -1)
		return false ;

	// 2) Translate aliases
	if (!argv.empty())
	{
		m_Aliases.Translate(argv);
		
		// 3) Reassemble the command line
		for (unsigned int i = 0 ; i < argv.size() ; i++)
		{
			result += argv[i] ;
			if (i != argv.size()-1) result += " " ;
		}
	}

	pConnection->AddSimpleResultToSMLResponse(pResponse, result.c_str());

	return true ;
}

bool CommandLineInterface::DoCommandInternal(gSKI::IAgent* pAgent, const std::string& commandLine) {
	vector<string> argv;
	// Parse command:
	if (Tokenize(commandLine, argv) == -1)  return false;	// Parsing failed

	// Execute the command
	return DoCommandInternal(pAgent, argv);
}

bool CommandLineInterface::DoCommandInternal(gSKI::IAgent* pAgent, vector<string>& argv) {
	if (!argv.size()) return true;

	// Check for help flags
	bool helpFlag = false;
	if (CheckForHelp(argv)) {
		helpFlag = true;
	}

	// Expand aliases
	if (m_Aliases.Translate(argv)) {
		// Is the alias target implemented?
		if (m_CommandMap.find(argv[0]) == m_CommandMap.end()) {
			SetErrorDetail("(No such command: " + argv[0] + ")");
			return SetError(CLIError::kCommandNotImplemented);
		}

	} else {
		// Not an alias, check for partial match
		std::list<std::string> possibilities;
		std::list<std::string>::iterator liter;
		bool exactMatch = false;

		for(unsigned index = 0; index < (argv[0].size()); ++index) {

			if (index == 0) {
				// Bootstrap the list of possibilities
				CommandMapConstIter citer = m_CommandMap.begin();

				while (citer != m_CommandMap.end()) {
					if (citer->first[index] == argv[0][index]) {
						possibilities.push_back(citer->first);
					}
					++citer;
				}

			} else {
				// Update the list of possiblities

				// A more efficient search here would be nice.
				liter = possibilities.begin();
				while (liter != possibilities.end()) {
					if ((*liter)[index] != argv[0][index]) {
						// Remove this possibility from the list
						liter = possibilities.erase(liter);
					} else {
						// check for exact match
						if (argv[0] == (*liter)) {
							// Exact match, we're done
							argv[0] = (*liter);
							exactMatch = true;
							break;
						}
						++liter;
					}
				}
				if (exactMatch) break;
			}

			if (!possibilities.size()) {
				// Not implemented
				SetErrorDetail("(No such command: " + argv[0] + ")");
				return SetError(CLIError::kCommandNotImplemented);

			} 
		}

		if (!exactMatch) {
			if (possibilities.size() != 1) {
				// Ambiguous
				std::string detail = "Ambiguous command, possibilities: ";
				liter = possibilities.begin();
				while (liter != possibilities.end()) {
					detail += (*liter) + ' ';
					++liter;
				}
				SetErrorDetail(detail);
				return SetError(CLIError::kAmbiguousCommand);

			} else {
				// We have a partial match
				argv[0] = (*(possibilities.begin()));
			}
		}
	}

	// Show help if requested
	if (helpFlag) {
		std::string helpFile = m_LibraryDirectory + "/help/" + argv[0];
		return GetHelpString(helpFile);
	}

	// Process command
	CommandFunction pFunction = m_CommandMap[argv[0]];
	assert(pFunction);
	
	// Initialize option parsing each call
	ResetOptions();

	// Make the Parse call
	return (this->*pFunction)(pAgent, argv);
}

int CommandLineInterface::Tokenize(string cmdline, vector<string>& argumentVector) {
	int argc = 0;
	string::iterator iter;
	string arg;
	bool quotes = false;
	int brackets = 0;
	int parens = 0;

	// Trim leading whitespace and comments from line
	if (!Trim(cmdline)) return -1;

	for (;;) {

		// Is there anything to work with?
		if(cmdline.empty()) break;

		// Remove leading whitespace
		iter = cmdline.begin();
		while (isspace(*iter)) {
			cmdline.erase(iter);

			if (!cmdline.length()) break; //Nothing but space left
			
			// Next character
			iter = cmdline.begin();
		}

		// Was it actually trailing whitespace?
		if (!cmdline.length()) break;// Nothing left to do

		// We have an argument
		++argc;
		arg.clear();
		// Use space as a delimiter unless inside quotes or brackets (nestable)
		while (!isspace(*iter) || quotes || brackets || parens) {
			if (*iter == '"') {
				// Flip the quotes flag
				quotes = !quotes;

			} else {
				if (*iter == '{') {
					++brackets;
				} else if (*iter == '}') {
					--brackets;
					if (brackets < 0) {
						SetErrorDetail("An extra '}' was found.");
						SetError(CLIError::kExtraClosingParen);
						return -1;
					}
				}
				if (*iter == '(') {
					++parens;
				} else if (*iter == ')') {
					--parens;
					if (parens < 0) {
						SetErrorDetail("An extra ')' was found.");
						SetError(CLIError::kExtraClosingParen);
						return -1;
					}
				}
			}

			// Add to argument (if we eat quotes, this has to be moved into the else above
			arg += (*iter);

			// Delete the character and move on on
			cmdline.erase(iter);
			iter = cmdline.begin();

			// Are we at the end of the string?
			if (iter == cmdline.end()) {

				// Did they close their quotes or brackets?
				if (quotes || brackets || parens) {
					std::string errorDetail = "These quoting characters were not closed: ";
					if (quotes) errorDetail += "\"";
					if (brackets) errorDetail += "{";
					if (parens) errorDetail += "(";
					SetErrorDetail(errorDetail);
					SetError(CLIError::kUnmatchedBracketOrQuote);
					return -1;
				}
				break;
			}
		}

		// Store the arg
		argumentVector.push_back(arg);
	}

	// Return the number of args found
	return argc;
}

bool CommandLineInterface::CheckForHelp(std::vector<std::string>& argv) {

	// Standard help check if there is more than one argument
	if (argv.size() > 1) {
		// Is one of the two help strings present?
		if (argv[1] == "-h" || argv[1] == "--help") {
			return true;
		}
	}
	return false;
}

EXPORT void CommandLineInterface::SetKernel(gSKI::IKernel* pKernel, gSKI::Version kernelVersion, sml::KernelSML* pKernelSML) {
	m_pKernel = pKernel;
	m_KernelVersion = kernelVersion;
	m_pKernelSML = pKernelSML;

	// Now that we have the kernel, set the home directory to the location of SoarKernelSML
#ifdef WIN32
	char dllpath[256];
	GetModuleFileName(static_cast<HMODULE>(m_pKernelSML->GetModuleHandle()), dllpath, 256);
	m_LibraryDirectory = dllpath;
	m_LibraryDirectory = m_LibraryDirectory.substr(0, m_LibraryDirectory.find_last_of("\\"));
#else // WIN32
	GetCurrentWorkingDirectory(m_LibraryDirectory);
#endif
}

bool CommandLineInterface::GetCurrentWorkingDirectory(string& directory) {
	// Pull an arbitrary buffer size of 1024 out of a hat and use it
	char buf[1024];
	char* ret = getcwd(buf, 1024);

	// If getcwd returns 0, that is bad
	if (!ret) return SetError(CLIError::kgetcwdFail);

	// Store directory in output parameter and return success
	directory = buf;
	return true;
}

bool CommandLineInterface::IsInteger(const string& s) {
	string::const_iterator iter = s.begin();
	
	// Allow negatives
	if (s.length() > 1) {
		if (*iter == '-') {
			++iter;
		}
	}

	while (iter != s.end()) {
		if (!isdigit(*iter)) {
			return false;
		}
		++iter;
	}
	return true;
}

bool CommandLineInterface::RequireAgent(gSKI::IAgent* pAgent) {
	// Requiring an agent implies requiring a kernel
	if (!RequireKernel()) return false;
	if (!pAgent) return SetError(CLIError::kAgentRequired);
	return true;
}

bool CommandLineInterface::RequireKernel() {
	if (!m_pKernel) return SetError(CLIError::kKernelRequired);
	return true;
}

void CommandLineInterface::AppendArgTag(const char* pParam, const char* pType, const char* pValue) {
	TagArg* pTag = new TagArg();
	pTag->SetParam(pParam);
	pTag->SetType(pType);
	pTag->SetValue(pValue);
	m_ResponseTags.push_back(pTag);
}

void CommandLineInterface::AppendArgTagFast(const char* pParam, const char* pType, const char* pValue) {
	TagArg* pTag = new TagArg();
	pTag->SetParamFast(pParam);
	pTag->SetTypeFast(pType);
	pTag->SetValue(pValue);
	m_ResponseTags.push_back(pTag);
}

void CommandLineInterface::PrependArgTag(const char* pParam, const char* pType, const char* pValue) {
	TagArg* pTag = new TagArg();
	pTag->SetParam(pParam);
	pTag->SetType(pType);
	pTag->SetValue(pValue);
	m_ResponseTags.push_front(pTag);
}

void CommandLineInterface::PrependArgTagFast(const char* pParam, const char* pType, const char* pValue) {
	TagArg* pTag = new TagArg();
	pTag->SetParamFast(pParam);
	pTag->SetTypeFast(pType);
	pTag->SetValue(pValue);
	m_ResponseTags.push_front(pTag);
}

void CommandLineInterface::AddListenerAndDisableCallbacks(gSKI::IAgent* pAgent) {
	if (m_pKernelSML) m_pKernelSML->DisablePrintCallback(pAgent);
	m_PrintEventToResult = true;
	if (!m_pLogFile && pAgent) pAgent->AddPrintListener(gSKIEVENT_PRINT, this);
}

void CommandLineInterface::RemoveListenerAndEnableCallbacks(gSKI::IAgent* pAgent) {
	if (!m_pLogFile && pAgent) pAgent->RemovePrintListener(gSKIEVENT_PRINT, this);
	m_PrintEventToResult = false;
	if (m_pKernelSML) m_pKernelSML->EnablePrintCallback(pAgent);
}

bool CommandLineInterface::SetError(cli::ErrorCode code) {
	m_LastError = code;
	return false;
}
bool CommandLineInterface::SetErrorDetail(const std::string detail) {
	m_LastErrorDetail = detail;
	return false;
}

void CommandLineInterface::ResultToArgTag() {
	AppendArgTagFast(sml_Names::kParamMessage, sml_Names::kTypeString, m_Result.str().c_str());
	m_Result.str("");
}

bool CommandLineInterface::StripQuotes(std::string& str) {
    if ((str.size() >= 2) && (str[0] == '"') && (str[str.length() - 1] == '"')) {
        str = str.substr(1, str.length() - 2);
        return true;
    }
    return false;
}

void CommandLineInterface::ResetOptions() {
	m_Argument = 0;
	m_NonOptionArguments = 0;	
}

bool CommandLineInterface::ProcessOptions(std::vector<std::string>& argv, Options* options) {

	// default to indifference
	m_Option = 0;
	m_OptionArgument = "";

	// increment current argument and check bounds
	while (static_cast<unsigned>(++m_Argument) < argv.size()) {

		// args less than 2 characters cannot mean anything to us
		if (argv[m_Argument].size() < 2) {
			++m_NonOptionArguments;
			continue;
		}

		if (argv[m_Argument][0] == '-') {
			if (argv[m_Argument][1] == '-') {
				// possible long m_Option
				if (argv[m_Argument].size() > 2) {
					// long m_Option, save it
					std::string longOption = argv[m_Argument].substr(2);

					// check for partial match
					std::list<Options> possibilities;
					std::list<Options>::iterator liter;

					for(unsigned index = 0; index < longOption.size(); ++index) {

						if (index == 0) {
							// Bootstrap the list of possibilities
							for (int i = 0; options[i].shortOpt != 0; ++i) {
								if (options[i].longOpt[index] == longOption[index]) {
									possibilities.push_back(options[i]);
								}
							}

						} else {
							// Update the list of possiblities

							// A more efficient search here would be nice.
							liter = possibilities.begin();
							while (liter != possibilities.end()) {
								if ((*liter).longOpt[index] != longOption[index]) {
									// Remove this possibility from the list
									liter = possibilities.erase(liter);
								} else {
									// check for exact match
									if (longOption == (*liter).longOpt) {
										// exact match, we're done
										m_Option = liter->shortOpt;
										MoveBack(argv, m_Argument, m_NonOptionArguments);
										if (!HandleOptionArgument(argv, liter->longOpt, liter->argument)) return false;
										return true;
									}
									++liter;
								}
							}
						}

						if (!possibilities.size()) {
							SetErrorDetail("No such m_Option: " + longOption);
							return SetError(CLIError::kUnrecognizedOption);
						} 
					}

					if (possibilities.size() != 1) {
						// Ambiguous
						std::string detail = "Ambiguous m_Option, possibilities: ";
						liter = possibilities.begin();
						while (liter != possibilities.end()) {
							detail += (*liter).longOpt + ' ';
							++liter;
						}
						SetErrorDetail(detail);
						return SetError(CLIError::kAmbiguousOption);

					}
					// We have a partial match
					m_Option = (*(possibilities.begin())).shortOpt;
					MoveBack(argv, m_Argument, m_NonOptionArguments);
					if (!HandleOptionArgument(argv, (*(possibilities.begin())).longOpt, (*(possibilities.begin())).argument)) return false;
					return true;
				}

				// end of options special flag '--'
				// FIXME: remove -- argument?
				m_Option = -1; // done
				return true; // no error
			}

			// short m_Option(s)
			for (int i = 0; options[i].shortOpt != 0; ++i) {
				if (argv[m_Argument][1] == options[i].shortOpt) {
					if (argv[m_Argument].size() > 2) {
						std::vector<std::string>::iterator target = argv.begin();
						target += m_Argument;

						std::string original = *target;
						*target = (*target).substr(0,2);
						++target;

						argv.insert(target, "-" + original.substr(2));
					}
					m_Option = options[i].shortOpt;
					MoveBack(argv, m_Argument, m_NonOptionArguments);
					if (!HandleOptionArgument(argv, options[i].longOpt, options[i].argument)) return false;
					return true;
				}
			}
			SetErrorDetail("No such m_Option: " + argv[m_Argument][1]);
			return SetError(CLIError::kUnrecognizedOption);
		}
		++m_NonOptionArguments;
	}

	// out of arguments
	m_Option = -1;	// done
	return true;	// no error
}

void CommandLineInterface::MoveBack(std::vector<std::string>& argv, int what, int howFar) {

	assert(what >= howFar);
	assert(what > 0);
	assert(howFar >= 0);

	if (howFar == 0) {
		return;
	}

	std::vector<std::string>::iterator dest = argv.begin();
	std::vector<std::string>::iterator target = argv.begin();

	dest += howFar;
	target += what;

	argv.insert(dest, *target);

	target = argv.begin();
	target += what + 1;

	argv.erase(target);
}

bool CommandLineInterface::HandleOptionArgument(std::vector<std::string>& argv, const char* option, int arg) {
	switch (arg) {
		case 0:
			break;
		case 1:
			// required argument
			if (static_cast<unsigned>(++m_Argument) >= argv.size()) {
				std::string detail(option);
				SetErrorDetail("Option '" + detail + "' requires an argument.");
				return SetError(CLIError::kMissingOptionArg);
			}
			m_OptionArgument = argv[m_Argument];
			MoveBack(argv, m_Argument, m_NonOptionArguments);
			break;
		case 2:
		default:
			// optional argument
			if (static_cast<unsigned>(++m_Argument) < argv.size()) {
				if (argv[m_Argument].size()) {
					if (argv[m_Argument][0] != '-') {
						m_OptionArgument = argv[m_Argument];
						MoveBack(argv, m_Argument, m_NonOptionArguments);
					} 
				}
			}
			if (!m_OptionArgument.size()) {
				--m_Argument;
			}
			break;
	}
	return true;
}