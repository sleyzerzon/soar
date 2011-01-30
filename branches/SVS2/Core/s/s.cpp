/* Minimal Soar CLI suitable for scripting
   Last modified Aug 30 2010
*/

#include <string>
#include <iostream>
#include <sstream>
#include <signal.h>
#include "sml_Client.h"

using namespace std;
using namespace sml;

Kernel* kernel = NULL;
Agent* agent = NULL;
string lastcmd;

string strip(string s, string lc, string rc) {
	size_t b, e;
	b = s.find_first_not_of(lc);
	e = s.find_last_not_of(rc);
	return s.substr(b, e - b + 1);
}

bool isidentifier(const string &s) {
	int i;
	if (s.empty() || !isupper(s[0]))
		return false;
	for (i = 1; i < s.size(); ++i) {
		if (!isdigit(s[i]))
			return false;
	}
	return true;
}

/*
 * Determine how many levels of brace nesting this line adds. For
 * example, the line "sp {" will return +1, and the line "{(somthing)}}"
 * will return -1. Will avoid counting braces in quoted strings.
 */
bool totalnesting(string line, int &result) {
	int nesting = 0;
	size_t p = line.find_first_of("{}|");
	
	while (p != string::npos) {
		switch (line[p]) {
			case '{':
				++nesting;
				break;
			case '}':
				--nesting;
				break;
			case '|':
				// skip over quoted string
				while (true) {
					p = line.find_first_of('|', p+1);
					if (p == string::npos) {
						// error, no closing quote pipe on line
						return false;
					}
					if (line[p-1] != '\\') {
						break;
					}
				}
				break;
		}
		p = line.find_first_of("{}|", p+1);
	}
	result = nesting;
	return true;
}

/* Read a command, spanning newlines if the command contains unclosed braces */
bool readcmd(string &result) {
	int nestlvl, i, n;
	string line;
	stringstream cmd;
	
	nestlvl = 0;
	while(getline(cin, line)) {
		if (!totalnesting(line, n)) {
			return false;
		}
		nestlvl += n;
		cmd << line << endl;
		if (nestlvl < 0) {
			return false;
		} else if (nestlvl == 0) {
			break;
		}
	}
	
	if (nestlvl > 0) {
		return false;
	}
	result = cmd.str();
	if (!result.empty() && result[result.size()-1] == '\n') {
		result.erase(result.size()-1);
	}
	return true;
}

void printcb(smlPrintEventId id, void *d, Agent *a, char const *m) {
	cout << strip(m, "\n", "\n\t ") << endl;
}

void execcmd(string c) {
	string out = agent->ExecuteCommandLine(c.c_str());
	if (out.size() > 0) {
		cout << strip(out, "\n", "\n\t ") << endl;
	}
}

void repl() {
	string cmd;
	
	while (cin) {
		cout << endl << "# ";
		if (!readcmd(cmd)) {
			cout << "?" << endl;
			continue;
		}
		if (!cin) {
			return;
		}
		if (cmd.empty()) {
			if (lastcmd.empty()) {
				continue;
			}
			cmd = lastcmd;
		} else {
			if (isidentifier(cmd)) {
				cmd.insert(0, "print ");
			}
			lastcmd = cmd;
		}
		execcmd(cmd);
	}
}

void siginthandler(int sig) {
	if (agent) {
		agent->StopSelf();
	}
	signal(SIGINT, siginthandler);
}

string exit_handler(smlRhsEventId id, void *pUserData, Agent *pAgent, char const *pFunctionName, char const *pArgument) {
	int code = atoi(pArgument);
	exit(code);
}

int main(int argc, char *argv[]) {
	string src = "source ";
	
	kernel = Kernel::CreateKernelInCurrentThread(Kernel::kDefaultLibraryName, true, 0);
	kernel->AddRhsFunction("exit", exit_handler, NULL);
	
	agent = kernel->CreateAgent("soar1");
	agent->RegisterForPrintEvent(smlEVENT_PRINT, printcb, NULL);
	agent->SetOutputLinkChangeTracking(false);
	
	if (signal(SIGINT, SIG_IGN) != SIG_IGN) {
		signal(SIGINT, siginthandler);
	}
	
	for(int i = 1; i < argc; ++i) {
		execcmd(src + argv[i]);
	}
		
	repl();
	return 0;
}
