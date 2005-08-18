#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

// Use Visual C++'s memory checking functionality
#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // _MSC_VER

#include <string>
#include <iostream>
#include "sml_Client.h"
#include "sml_Connection.h"

using namespace std;
using namespace sml;

void MyPrintEventHandler(smlPrintEventId id, void* pUserData, Agent* pAgent, char const* pMessage) {
	cout << pMessage << endl;
}

void main() {

	// When we have a memory leak, set this variable to
	// the allocation number (e.g. 122) and then we'll break
	// when that allocation occurs.
	//_crtBreakAlloc = 73 ;

	{ // create local scope to allow for local memory cleanup before we check at end
		
		//
		// set up Soar
        //
		Kernel* kernel = Kernel::CreateKernelInNewThread("SoarKernelSML");
		Agent* agent = kernel->CreateAgent("Soar1");
		
		agent->RegisterForPrintEvent(smlEVENT_PRINT, MyPrintEventHandler, NULL);

		agent->ExecuteCommandLine("rete-net -l test.soarx");
		agent->ExecuteCommandLine("watch 0");
		agent->ExecuteCommandLine("max-elaborations 5000");
		AnalyzeXML response;
		agent->ExecuteCommandLineXML("time run", &response);
		
		cout << endl << response.GenerateXMLString(true) << endl;
		
		//FIXME: can't actually differentiate these results yet due to bug 523
		string realtime = response.GetArgValue("seconds");
		string proctime = response.GetArgValue("seconds");
		
		agent->ExecuteCommandLineXML("stats", &response);
		
		string kerneltime = response.GetArgValue("statskernelcputime");
		string totaltime = response.GetArgValue("statstotalcputime");
		
		kernel->Shutdown();
		delete kernel;

		//
		// Show results
		//
		cout << endl;
		cout << "OS Real: " << realtime << endl;
		cout << "OS Proc: " << proctime << endl;
		cout << "Soar Kernel: " << kerneltime << endl;
		cout << "Soar Total: " << totaltime << endl;

		string dummy;
		cout << endl << endl << "Press any key and enter to continue";
		cin >> dummy;

	} // end local scope

#ifdef _MSC_VER
	// Set the memory checking output to go to Visual Studio's debug window (so we have a copy to keep)
	// and to stdout so we can see it immediately.
	_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG );
	_CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT );

	// Now check for memory leaks.
	// This will only detect leaks in objects that we allocate within this executable and static libs.
	// If we allocate something in a DLL then this call won't see it because it works by overriding the
	// local implementation of malloc.
	_CrtDumpMemoryLeaks();
#endif // _MSC_VER
	
}