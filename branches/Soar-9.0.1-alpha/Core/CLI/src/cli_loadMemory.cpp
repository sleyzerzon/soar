/////////////////////////////////////////////////////////////////
// load-memory command file.
//
// Author: Yongjia Wang
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

//#ifdef SEMANTIC_MEMORY


#include "cli_CommandLineInterface.h"

#include "sml_Names.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <hash_map>
#include "gSKI_Agent.h"
//#include "agent.h"
//#include "print.h"
#include "gSKI_DoNotTouch.h"
#include "gSKI_Kernel.h"

//#include "../../gSKI/src/gSKI_Agent.h"
//#include "../../SoarKernel/include/agent.h"
//#include "../../SoarKernel/include/print.h"
//#include "gSKI_DoNotTouch.h"
//#include "gSKI_Kernel.h"



using namespace cli;
using namespace sml;
using namespace std;
using namespace gSKI;

class memory_elem{
public:
	memory_elem(string i, string a, string v, int t, int h){
		id = i;
		attr = a;
		value = v;
		value_type = t;
		most_recent_history = h;
	}
	string id;
	string attr;
	string value;
	int value_type;
	int most_recent_history;
};

// Preload semantic memory from file
bool CommandLineInterface::ParseLoadMemory(gSKI::Agent* pAgent, std::vector<std::string>& argv) {
	
/* In IgSKI now
	Agent* pAgent2 = (Agent*)(pAgent);
	agent* thisAgent = pAgent2->GetSoarAgent();
*/
	for(std::vector<string>::iterator itr = argv.begin(); itr != argv.end(); ++itr){
		cout << *itr << endl;
	}
	
	if(argv.size() < 2){
		cout << "no file name specified\n";
		return false;
	}

	string filename = argv[1];

	StripQuotes(filename);

    // Separate the path out of the filename if any
	// Code Copied from cli_source.cpp
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
		return SetError(CLIError::kOpenFileFail);
	}
	
	//cout << "Path: " << path << endl;
	//cout << "Filename: " << filename << endl;
	

 	m_Result << "Loading Memory\nPath: " << path << "\nFilename: " << filename << std::endl;
	//AddListenerAndDisableCallbacks(pAgent);
	//print(thisAgent, "Loading Memory\nPath: %s\nFilename: %s\n", path.c_str(), filename.c_str());
	//RemoveListenerAndEnableCallbacks(pAgent);

	std::string line;
	int lineCount = 0;
	
	
	

	// Should only hold most recent values
	// If there is a tie, could have multiple value
	stdext::hash_map<string, vector<memory_elem> > processed_memory;

	
	while (getline(soarFile, line)) {
		
		// Increment line count
		++lineCount;
		
		// This Trim function does not handle quoting mark loaded through a file correctly. e.g |some value| -- YJ
		// Trim whitespace and comments
		//if (!Trim(line)) {
		//	HandleSourceError(lineCount, filename); Interface changed
		//	if (path.length()) DoPopD();
		//	return false;
		//}
	//	mResult << line << std::endl;
		//cout << line << endl;
		//AddListenerAndDisableCallbacks(pAgent);
		//print(thisAgent, "%s\n", line.c_str());
		//RemoveListenerAndEnableCallbacks(pAgent);
		if(line.size() == 0 || line[0] == '#'){
			continue;
		}
		istringstream isstr(line);
		string id, attr, value;
		vector<int> history;
		char ch;
		int type;
		
		if(true){
			isstr >> id >> attr;
			//cout << id << " " << attr << endl;
			char val_ch = isstr.get();
			while(val_ch == ' '){
				val_ch = isstr.get();
			}
			
			//cout << val_ch << endl;
			if(val_ch == '|'){
				char val[99999];
				isstr.getline(val, 99999, '|');
				value = string(val);
			}
			else{
				isstr >> value;
				value = val_ch + value;
			}
			isstr >> type;

			//cout << id << " " << attr << " '" << value << "' " << type << " " << endl;
		}
		else{
			isstr >> id >> attr >> value >> type;
		}
		
		// id attr value type [2 3 4 ]
		if(isstr >> ch){
			if(ch == '['){
				int num;
				while(isstr >> num){
					history.push_back(num);
					//cout << num << endl;
				}
			}
		}
		if(history.size() == 0){ // no history
			history.push_back(0);
		}
		stdext::hash_map<string, vector<memory_elem> >::iterator mem_itr = processed_memory.find(id+","+attr);
		if(mem_itr == processed_memory.end()){
			vector<memory_elem> multi_value_elemes;
			multi_value_elemes.push_back(memory_elem(id, attr, value, type, history.back()));
			processed_memory.insert(pair<string, vector<memory_elem> >(id+","+attr, multi_value_elemes));
		}
		else{
			mem_itr->second.push_back(memory_elem(id, attr, value, type, history.back()));
		}


		// Attain the evil back door of doom, even though we aren't the TgD
		//gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();
		//pKernelHack->load_semantic_memory_data(pAgent, id, attr, value, type, history);
		
	}


	soarFile.close();
	if (path.length()) DoPopD();
	
	// Need to update history reference number
	// For multi valued attributes, only keep the most recent one, unless there is a tie
	for(stdext::hash_map<string, vector<memory_elem> >::iterator mem_itr = processed_memory.begin(); mem_itr != processed_memory.end(); ++ mem_itr){
		vector<memory_elem> multi_value_elems = mem_itr->second;
		if(multi_value_elems.size() == 1){ // just single value
			vector<int> h;
			h.push_back(0);
			string id = multi_value_elems[0].id;
			string attr = multi_value_elems[0].attr;
			string value = multi_value_elems[0].value;
			int type = multi_value_elems[0].value_type;
			
			gSKI::EvilBackDoor::TgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();
			pKernelHack->load_semantic_memory_data(pAgent, id, attr, value, type, h);
		}
		else{ // multiple values, keep the most recent ones (could be tie)
			vector <memory_elem> final_values;
			vector<memory_elem> multi_value_elems = mem_itr->second;
			int most_recent_history = 0;
			for(int i=0; i<multi_value_elems.size(); ++i){
				string id = multi_value_elems[i].id;
				string attr = multi_value_elems[i].attr;
				string value = multi_value_elems[i].value;
				int type = multi_value_elems[i].value_type;
				int history = multi_value_elems[i].most_recent_history;
				if(history >= most_recent_history){
					if(history > most_recent_history){
						most_recent_history = history;
						final_values.clear();
					}
					final_values.push_back(memory_elem(id, attr, value, type, history));
				}
			}
			for(int i=0; i<final_values.size(); ++i){
				string id = final_values[i].id;
				string attr = final_values[i].attr;
				string value = final_values[i].value;
				int type = final_values[i].value_type;
				vector<int> h;
				h.push_back(0);
				gSKI::EvilBackDoor::TgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();
				pKernelHack->load_semantic_memory_data(pAgent, id, attr, value, type, h);
			}
		}
	}
	

	// Quit needs no help
	return true;
}


bool CommandLineInterface::ParsePrintMemory(gSKI::Agent* pAgent, std::vector<std::string>& argv) {
	
	gSKI::EvilBackDoor::TgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();
	AddListenerAndDisableCallbacks(pAgent);
	
	string attr = "", value = "";
	string outputfile;
	if(argv.size() >= 3){
		if(argv[1] == "-f"){
			outputfile = argv[2];
			if(argv.size() >= 5){
				attr = argv[3];
				value = argv[4];
			}
			
		}
		else{
			attr = argv[1];
			value = argv[2];
		}

		
	}
	// print the chunks with the specified attr-value
	// Only support one pair, should be able to support more complex regex in the future.
	pKernelHack->print_semantic_memory(pAgent, attr, value, outputfile);
	
	int count_size = pKernelHack->semantic_memory_chunk_count(pAgent);
	int lme_size = pKernelHack->semantic_memory_lme_count(pAgent);
	m_Result << count_size << " Chunks" << endl;
	m_Result << lme_size << " Elements" << endl;

	RemoveListenerAndEnableCallbacks(pAgent);

/*
	Agent* pAgent2 = (Agent*)(pAgent);
	agent* thisAgent = pAgent2->GetSoarAgent();
		
	vector<LME> content;
	thisAgent->semantic_memory->dump(content);
	for(vector<LME>::iterator itr = content.begin(); itr != content.end(); ++itr){
		AddListenerAndDisableCallbacks(pAgent);
		print(thisAgent, "<%s, %s, %s, %d>\n",itr->id.c_str(), itr->attr.c_str(), itr->value.c_str(), itr->value_type);
		RemoveListenerAndEnableCallbacks(pAgent);
	}

*/
	// Quit needs no help
	return true;
}

bool CommandLineInterface::ParseClearMemory(gSKI::Agent* pAgent, std::vector<std::string>& argv) {
	
	gSKI::EvilBackDoor::TgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();
	RemoveListenerAndEnableCallbacks(pAgent);
	int size = pKernelHack->clear_semantic_memory(pAgent);
	m_Result << size << " elements cleared" << endl;
	
	RemoveListenerAndEnableCallbacks(pAgent);
/*
	Agent* pAgent2 = (Agent*)(pAgent);
	agent* thisAgent = pAgent2->GetSoarAgent();
	//thisAgent->semantic_memory->insert_LME(id, attr, value, type);
		
	int size = thisAgent->semantic_memory->clear();
	AddListenerAndDisableCallbacks(pAgent);
	print(thisAgent, "%d LMEs cleared\n",size);
	RemoveListenerAndEnableCallbacks(pAgent);
*/
	return true;
}

bool CommandLineInterface::ParseSummarizeMemory(gSKI::Agent* pAgent, std::vector<std::string>& argv) {
	
	gSKI::EvilBackDoor::TgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();
	RemoveListenerAndEnableCallbacks(pAgent);
	int count_size = pKernelHack->semantic_memory_chunk_count(pAgent);
	int lme_size = pKernelHack->semantic_memory_lme_count(pAgent);
	m_Result << count_size << " Chunks" << endl;
	m_Result << lme_size << " Elements" << endl;
	
	RemoveListenerAndEnableCallbacks(pAgent);

	return true;
}

bool CommandLineInterface::ParseSmemOption(gSKI::Agent* pAgent, std::vector<std::string>& argv) {
	
	gSKI::EvilBackDoor::TgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	Options optionsData[] = {
		//{'d', "disable",	0},
		//{'d', "off",		0},
		//{'e', "enable",		0},
		//{'e', "on",			0},
		{'a', "automatic",	0},
		{'d', "deliberate",		0},
		{'h', "help",			0},
		//{'w', "wrong",		0},
		{0, 0, 0}
	};
	SmemBitset options(0);
	
	
	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1){ // display info
			int current_op = pKernelHack->semantic_memory_set_parameter(pAgent, 2);
			if(current_op == 0){
				RemoveListenerAndEnableCallbacks(pAgent);
				m_Result << "Current Option: Deliberate" << endl;
				RemoveListenerAndEnableCallbacks(pAgent);
			}
			else if(current_op == 1){
				RemoveListenerAndEnableCallbacks(pAgent);
				m_Result << "Current Option: Automatic" << endl;
				RemoveListenerAndEnableCallbacks(pAgent);
			}
			break;
		}

		switch (m_Option) {
			case 'd':
				pKernelHack->semantic_memory_set_parameter(pAgent, 0);
				//options.set(0);
				break;
			case 'a':
				pKernelHack->semantic_memory_set_parameter(pAgent, 1);
				//options.set(1);
				break;
			case 'h':
				m_Result << "-a, --automatic: automatic saving, disable deliberate saving" << endl;
				m_Result << "-d, --deliberate: deliberate saving, disable automatic saving" << endl;
				//options.set(1);
				break;
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	// No non-option arguments
	//if (m_NonOptionArguments) return SetError(CLIError::kTooManyArgs);
	//RemoveListenerAndEnableCallbacks(pAgent);
	//m_Result << m_Option << endl;
	//RemoveListenerAndEnableCallbacks(pAgent);
	
	
	return true;
}

// Preload Cluster file
bool CommandLineInterface::ParseCluster(gSKI::Agent* pAgent, std::vector<std::string>& argv) {
	
	// Attain the evil back door of doom, even though we aren't the TgD
	gSKI::EvilBackDoor::TgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();
	for(std::vector<string>::iterator itr = argv.begin(); itr != argv.end(); ++itr){
		cout << *itr << endl;
	}
	
	if(argv.size() < 2){
		cout << "Printing Cluster Weights\n";
		AddListenerAndDisableCallbacks(pAgent);
		pKernelHack->clustering(pAgent, std::vector<std::vector<double> >(), true, false);
		AddListenerAndDisableCallbacks(pAgent);
		return false;
	}

	if(argv[1] == "clear"){
		pKernelHack->clustering(pAgent, std::vector<std::vector<double> >(), false, true);
	}

	// Quit needs no help
	return true;
}


bool CommandLineInterface::ParseClusterTrain(gSKI::Agent* pAgent, std::vector<std::string>& argv) {
	
	// Attain the evil back door of doom, even though we aren't the TgD
	gSKI::EvilBackDoor::TgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();
	for(std::vector<string>::iterator itr = argv.begin(); itr != argv.end(); ++itr){
		cout << *itr << endl;
	}
	
	if(argv.size() < 2){
		cout << "No filename specified\n";
		return false;
	}

	string filename = argv[1];

	StripQuotes(filename);

    // Separate the path out of the filename if any
	// Code Copied from cli_source.cpp
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
		return SetError(CLIError::kOpenFileFail);
	}
	
	//cout << "Path: " << path << endl;
	//cout << "Filename: " << filename << endl;
	

 	m_Result << "Cluster Training File\nPath: " << path << "\nFilename: " << filename << std::endl;
	//AddListenerAndDisableCallbacks(pAgent);
	//print(thisAgent, "Loading Memory\nPath: %s\nFilename: %s\n", path.c_str(), filename.c_str());
	//RemoveListenerAndEnableCallbacks(pAgent);

	std::string line;
	int lineCount = 0;
	
	
	std::vector<std::vector<std::pair<std::string, std::string> > > instances;

	while (getline(soarFile, line)) {
		
		// Increment line count
		++lineCount;

		// Trim whitespace and comments
		if (!Trim(line)) {
		//	HandleSourceError(lineCount, filename); Interface changed
			if (path.length()) DoPopD();
			return false;
		}
	//	mResult << line << std::endl;
		//cout << line << endl;
		//AddListenerAndDisableCallbacks(pAgent);
		//print(thisAgent, "%s\n", line.c_str());
		//RemoveListenerAndEnableCallbacks(pAgent);
		if(line.size() == 0 || line[0] == '#'){
			continue;
		}
		istringstream isstr(line);
		
		vector<pair<string, string> > one_instance;
		string attr, value;
		while(isstr >> attr >> value){
			one_instance.push_back(pair<string, string>(attr, value));
		}
		instances.push_back(one_instance);
	}
	int train_times = 1;
	if(argv.size() >= 3){
		istringstream isstr(argv[2]);
		isstr >> train_times;
		//if(!isstr.good()){
		//	train_times = 1;
		//}
	}
	m_Result << "Training " << train_times << " Times" << endl;
	for(int i=0; i<train_times; ++i){
		pKernelHack->cluster_train(pAgent,instances);
	}
	

	soarFile.close();
	if (path.length()) DoPopD();

	// Quit needs no help
	return true;
}

bool CommandLineInterface::ParseClusterRecognize(gSKI::Agent* pAgent, std::vector<std::string>& argv) {
	
	// Attain the evil back door of doom, even though we aren't the TgD
	gSKI::EvilBackDoor::TgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();
	for(std::vector<string>::iterator itr = argv.begin(); itr != argv.end(); ++itr){
		cout << *itr << endl;
	}
	
	if(argv.size() < 2){
		cout << "No filename specified\n";
		return false;
	}

	string filename = argv[1];

	StripQuotes(filename);

    // Separate the path out of the filename if any
	// Code Copied from cli_source.cpp
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
		return SetError(CLIError::kOpenFileFail);
	}
	
	//cout << "Path: " << path << endl;
	//cout << "Filename: " << filename << endl;
	

 	m_Result << "Cluster Training File\nPath: " << path << "\nFilename: " << filename << std::endl;
	//AddListenerAndDisableCallbacks(pAgent);
	//print(thisAgent, "Loading Memory\nPath: %s\nFilename: %s\n", path.c_str(), filename.c_str());
	//RemoveListenerAndEnableCallbacks(pAgent);

	std::string line;
	int lineCount = 0;
	
	
	std::vector<std::vector<std::pair<std::string, std::string> > > instances;

	while (getline(soarFile, line)) {
		
		// Increment line count
		++lineCount;

		// Trim whitespace and comments
		if (!Trim(line)) {
		//	HandleSourceError(lineCount, filename); Interface changed
			if (path.length()) DoPopD();
			return false;
		}
	//	mResult << line << std::endl;
		//cout << line << endl;
		//AddListenerAndDisableCallbacks(pAgent);
		//print(thisAgent, "%s\n", line.c_str());
		//RemoveListenerAndEnableCallbacks(pAgent);
		if(line.size() == 0 || line[0] == '#'){
			continue;
		}
		istringstream isstr(line);
		
		vector<pair<string, string> > one_instance;
		string attr, value;
		while(isstr >> attr >> value){
			one_instance.push_back(pair<string, string>(attr, value));
		}
		instances.push_back(one_instance);
	}
	
	std::vector<std::vector<int> > clusters = pKernelHack->cluster_recognize(pAgent,instances);
	//m_Result << clusters.size() << endl;
	for(int i=0; i<clusters.size(); ++i){
		//m_Result << clusters[i].size() << endl;
		for(int j=0; j<clusters[i].size(); ++j){
			m_Result << clusters[i][j] << ",";
		}
		m_Result << endl;
	}
	

	soarFile.close();
	if (path.length()) DoPopD();

	// Quit needs no help
	return true;
}
//#endif
