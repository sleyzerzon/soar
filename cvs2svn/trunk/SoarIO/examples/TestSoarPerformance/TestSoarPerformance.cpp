#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#ifdef _MSC_VER
#define snprintf _snprintf
// Use Visual C++'s memory checking functionality
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // _MSC_VER

#include <assert.h>
#include <string>
#include <iostream>
#include <iomanip>
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

	void PrintResults() {
		PrintResultsHelper("OS Real", GetAverage(realtimes), GetLow(realtimes), GetHigh(realtimes));
		PrintResultsHelper("OS Proc", GetAverage(proctimes), GetLow(proctimes), GetHigh(proctimes));
		PrintResultsHelper("Soar Kernel", GetAverage(kerneltimes), GetLow(kerneltimes), GetHigh(kerneltimes));
		PrintResultsHelper("Soar Total", GetAverage(totaltimes), GetLow(totaltimes), GetHigh(totaltimes));
	}

	void PrintResultsHelper(string label, double avg, double low, double high) {
		cout.precision(3);
		cout << resetiosflags(ios::right);
		cout << setiosflags(ios::left);
		cout.width(12);
		cout << label;
		cout << ":";
		cout << resetiosflags(ios::left);
		cout << setiosflags(ios::right);
		cout.width(10);
		cout << "Avg: " << avg;
		cout.width(10);
		cout << "Low: " << low;
		cout.width(15);
		cout << "High: " << high;
		cout << endl;
	}
};


void MyPrintEventHandler(smlPrintEventId id, void* pUserData, Agent* pAgent, char const* pMessage) {
	//cout << pMessage << endl;
}

void PrintTest1Description(int numTrials) {
	cout << endl;
	cout << "Test1 creates a kernel, runs the test suite once, and destroys the kernel." << endl;
	cout << "This is repeated " << numTrials << " times to measure average performance." << endl;
	cout << "Importantly, since the kernel is destroyed between each run, memory needs to be reallocated each time." << endl;
}
void Test1(int numTrials, StatsTracker* pSt, int watchlevel) {
	char wl[10];
	snprintf(wl, 9, "%s %i", "watch", watchlevel);

	for(int i = 0; i < numTrials; i++) {

		Kernel* kernel = Kernel::CreateKernelInNewThread("SoarKernelSML");
		Agent* agent = kernel->CreateAgent("Soar1");
		
		agent->RegisterForPrintEvent(smlEVENT_PRINT, MyPrintEventHandler, NULL);

		agent->LoadProductions("TestSoarPerformance.soar");
		agent->ExecuteCommandLine(wl);

		AnalyzeXML response;
		agent->ExecuteCommandLineXML("time run", &response);
		
		//FIXME: can't actually differentiate these results yet due to bug 523
		pSt->realtimes.push_back(atof(response.GetArgValue(sml_Names::kParamRealSeconds)));
		pSt->proctimes.push_back(atof(response.GetArgValue(sml_Names::kParamProcSeconds)));

		agent->ExecuteCommandLineXML("stats", &response);
		
		pSt->kerneltimes.push_back(atof(response.GetArgValue(sml_Names::kParamStatsKernelCPUTime)));
		pSt->totaltimes.push_back(atof(response.GetArgValue(sml_Names::kParamStatsTotalCPUTime)));
		
		agent->ExecuteCommandLine("stats");

		kernel->Shutdown();
		delete kernel;
	}
}

void PrintTest2Description(int numTrials) {
	cout << endl;
	cout << "Test2 creates a kernel and runs the test suite once to prime the memory pools." << endl;
	cout << "The stats from this priming step are thrown away." << endl;
	cout << "Then init-soar is called and the test-suite is run again." << endl;
	cout << "This is repeated " << numTrials << " times to measure average performance." << endl;
	cout << "Importantly, since the kernel is not destroyed between each run, memory does not need to be reallocated each time." << endl;
}

void Test2(int numTrials, StatsTracker* pSt, int watchlevel) {
	char wl[10];
	snprintf(wl, 9, "%s %i", "watch", watchlevel);

	Kernel* kernel = Kernel::CreateKernelInNewThread("SoarKernelSML");
	Agent* agent = kernel->CreateAgent("Soar1");
	
	agent->RegisterForPrintEvent(smlEVENT_PRINT, MyPrintEventHandler, NULL);

	agent->LoadProductions("TestSoarPerformance.soar");
	agent->ExecuteCommandLine(wl);
	agent->ExecuteCommandLine("run");

	for(int i = 0; i < numTrials; i++) {

		agent->ExecuteCommandLine("init-soar");
		AnalyzeXML response;
		agent->ExecuteCommandLineXML("time run", &response);
		
		//FIXME: can't actually differentiate these results yet due to bug 523
		pSt->realtimes.push_back(atof(response.GetArgValue(sml_Names::kParamRealSeconds)));
		pSt->proctimes.push_back(atof(response.GetArgValue(sml_Names::kParamProcSeconds)));

		agent->ExecuteCommandLineXML("stats", &response);

		pSt->kerneltimes.push_back(atof(response.GetArgValue(sml_Names::kParamStatsKernelCPUTime)));
		pSt->totaltimes.push_back(atof(response.GetArgValue(sml_Names::kParamStatsTotalCPUTime)));
	}

	kernel->Shutdown();
	delete kernel;
}

void main() {

	// When we have a memory leak, set this variable to
	// the allocation number (e.g. 122) and then we'll break
	// when that allocation occurs.
	//_crtBreakAlloc = 73 ;

	{ // create local scope to allow for local memory cleanup before we check at end

		StatsTracker stTest1_w0, stTest2_w0, stTest1_w1, stTest2_w1;

		int numTrials = 1;
		Test1(numTrials, &stTest1_w0, 0);
		//Test2(numTrials, &stTest2_w0, 0);
		//Test1(numTrials, &stTest1_w1, 1);
		//Test2(numTrials, &stTest2_w1, 1);

		PrintTest1Description(numTrials);
		PrintTest2Description(numTrials);

		cout << endl << "Test1 watch 0" << endl;
		stTest1_w0.PrintResults();
		cout << endl << "Test2 watch 0" << endl;
		//stTest2_w0.PrintResults();
		cout << endl << "Test1 watch 1" << endl;
		//stTest1_w1.PrintResults();
		cout << endl << "Test2 watch 1" << endl;
		//stTest2_w1.PrintResults();

		cout << endl << endl << "Press enter to continue";
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