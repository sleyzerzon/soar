#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

// Use Visual C++'s memory checking functionality
#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // _MSC_VER

#include <assert.h>
#include <string>
#include <iostream>
#include "sml_Client.h"
#include "sml_Connection.h"

using namespace std;
using namespace sml;

class StatsTracker {
public:
	vector<double> realtimes;
	vector<double> proctimes;
	vector<double> kerneltimes;
	vector<double> totaltimes;

	double GetAverage(vector<double> numbers) {
		assert(numbers.size() > 0 && "GetAverage: Size of set must be non-zero");

		double total = 0.0;
		for(unsigned int i=0; i<numbers.size(); i++) {
			total += numbers[i];
		}
		return total/(double)numbers.size();
	}

	double GetHigh(vector<double> numbers) {
		assert(numbers.size() > 0 && "GetHigh: Size of set must be non-zero");

		double high = numbers[0];
		for(unsigned int i=0; i<numbers.size(); i++) {
			if(numbers[i] > high) high = numbers[i];
		}
		return high;
	}

	double GetLow(vector<double> numbers) {
		assert(numbers.size() > 0 && "GetLow: Size of set must be non-zero");

		double low = numbers[0];
		for(unsigned int i=0; i<numbers.size(); i++) {
			if(numbers[i] < low) low = numbers[i];
		}
		return low;
	}

	void ReportResults() {
		cout << "OS Real: Avg: " << GetAverage(realtimes) << " Low: " << GetLow(realtimes) << " High: " << GetHigh(realtimes) << endl;
		cout << "OS Proc: Avg: " << GetAverage(proctimes) << " Low: " << GetLow(proctimes) << " High: " << GetHigh(proctimes) << endl;
		cout << "Soar Kernel: Avg: " << GetAverage(kerneltimes) << " Low: " << GetLow(kerneltimes) << " High: " << GetHigh(kerneltimes) << endl;
		cout << "Soar Total: Avg: " << GetAverage(totaltimes) << " Low: " << GetLow(totaltimes) << " High: " << GetHigh(totaltimes) << endl;
	}
};


void MyPrintEventHandler(smlPrintEventId id, void* pUserData, Agent* pAgent, char const* pMessage) {
	cout << pMessage << endl;
}

void Test1(int numTrials) {
	cout << "This test creates a kernel, runs the test suite once, and destroys the kernel." << endl;
	cout << "This is repeated " << numTrials << " times to measure average performance." << endl;
	cout << "Importantly, since the kernel is destroyed between each run, memory needs to be reallocated each time." << endl;

	StatsTracker st;

	for(int i = 0; i < numTrials; i++) {

		Kernel* kernel = Kernel::CreateKernelInNewThread("SoarKernelSML");
		Agent* agent = kernel->CreateAgent("Soar1");
		
		agent->RegisterForPrintEvent(smlEVENT_PRINT, MyPrintEventHandler, NULL);

		agent->ExecuteCommandLine("rete-net -l count-test-no-write.soarx");
		agent->ExecuteCommandLine("watch 0");
		agent->ExecuteCommandLine("max-elaborations 50000");
		AnalyzeXML response;
		agent->ExecuteCommandLineXML("time run", &response);
		
		//FIXME: can't actually differentiate these results yet due to bug 523
		st.realtimes.push_back(atof(response.GetArgValue("seconds")));
		st.proctimes.push_back(atof(response.GetArgValue("seconds")));

		agent->ExecuteCommandLineXML("stats", &response);
		
		st.kerneltimes.push_back(atof(response.GetArgValue("statskernelcputime")));
		st.totaltimes.push_back(atof(response.GetArgValue("statstotalcputime")));
		
		kernel->Shutdown();
		delete kernel;
	}

	st.ReportResults();

}

void Test2() {
}

void main() {

	// When we have a memory leak, set this variable to
	// the allocation number (e.g. 122) and then we'll break
	// when that allocation occurs.
	//_crtBreakAlloc = 73 ;

	{ // create local scope to allow for local memory cleanup before we check at end

		Test1(1);

		cout << endl << endl << "Press any key to continue";
		cin.get();

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