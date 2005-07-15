
#include "Reader.h"


using namespace std;

Reader::Reader(QuickLink* bringIn)
{
	QL = bringIn;

	//******CONSTANTS FOR PARSER******
	_QUIT = "QUIT";	_CLEAR = "CLEAR";	_SAVE = "SAVE";	_SAVES = "S";	_LOAD = "LOAD";
	_LOADS = "L";	_ADD = "ADD";	_ADDS = "A";	_CHANGE = "CHANGE";	_CHANGES = "C";
	_DELETE = "DELETE";	_DELETES = "D";	_DONE = "DONE";	_NEWP = "CLEARP";	_NEWPS = "CP";
	_LOADP = "LOADP";	_LOADPS = "LP";	_SAVEP = "SAVEP";	_SAVEPS = "SP";	_ENDP = "ENDP";
	_ENDPS = "EP";	_RUNCS = "RC";	_RUNC = "RUNCYCLE";	_RUN = "GO";	_RUNS = "G";
	_LASTOS = "LO";	_LASTO = "LASTOUTPUT"; _CMDLIN = "CMDLIN"; _CL = "CL"; _SOAR_FORM = "SOAR-FORM";
	_SF = "SF"; _TREE_FORM = "TREE-FORM"; _TF = "TF";
}

string
Reader::ReadMe(istream* in)
{
	string toReturn;
	
	if (QL->first == _CLEAR)  //clear current input-link structure
	{
		QL->clearAll();
		toReturn = QL->first;
		return toReturn;
	}
	else if (QL->first == _SAVE || QL->first == _SAVES)  //save current input-link structure
	{
		QL->locFinder();  //gets location of file
		ofstream tempFile;
		tempFile.open(QL->loc.c_str());
		QL->saveInput(true,tempFile);
		tempFile.close();
		tempFile.clear();
		toReturn = "***VOIDRETURN***";
		return toReturn;
	}
	else if (QL->first == _LOAD || QL->first == _LOADS)  //load a saved input-link structure
	{
		QL->userInput = false;
		QL->locFinder();
		ifstream tmpFile;
		tmpFile.open(QL->loc.c_str());
		QL->loadInput(tmpFile);
		tmpFile.close();
		tmpFile.clear();
		toReturn = "***VOIDRETURN***";
		return toReturn;
	}
	else if (QL->first == _ADD || QL->first == _ADDS)  //add something to il structure
	{
		*in >> QL->second >> QL->third >> QL->fourth;  //used to make parser changes easier
		QL->parent = QL->second;
		QL->makeUpper(QL->parent);
		QL->path = QL->third.substr(1, QL->third.size()-1); //gets rid of ^
		if(QL->fourth[0] == '/')  //IDENTIFER
		{				
			QL->uniqid = QL->fourth.substr(1, QL->fourth.size()-1); //gets rid of /
			QL->makeUpper(QL->uniqid);
			QL->createID();
		}
		else //VALUE_BASED
		{
			QL->value = QL->fourth;
			QL->advValue();
		}
		toReturn = QL->first + " " + QL->second + " " + QL->third + " " + QL->fourth;
		return toReturn;
	}
	else if (QL->first == _CHANGE || QL->first == _CHANGES) //change the value of something
	{
		*in >> QL->second >> QL->third >> QL->fourth >> QL->fifth;
		QL->parent = QL->second;
		QL->makeUpper(QL->parent);
		QL->path = QL->third.substr(1, QL->third.size()-1);  //gets rid of ^
		QL->OldVal = QL->fourth;
		QL->NewVal = QL->fifth;
		QL->advAlter();
		toReturn = QL->first + " " + QL->second + " " + QL->third + " " + QL->fourth + " " + QL->fifth;
		return toReturn;
	}
	else if (QL->first == _DELETE || QL->first == _DELETES) //delete an element
	{
		*in >> QL->second >> QL->third >> QL->fourth;
		QL->parent = QL->second;
		QL->makeUpper(QL->parent);
		QL->path = QL->third.substr(1,QL->third.size()-1);  //gets rid of ^
		if(QL->fourth[0] == '/') //IDENTIFIER
		{
			QL->uniqid = QL->fourth.substr(1,QL->fourth.size()-1);  //gets rid of /
			QL->makeUpper(QL->uniqid);
			QL->advDelInd();
		}
		else //VALUE_BASE
		{
			QL->value = QL->fourth;
			QL->advDelVal();
		}
		toReturn = QL->first + " " + QL->second + " " + QL->third + " " + QL->fourth;
		return toReturn;
	}
	else if (QL->first == _QUIT || QL->first == "EXIT")  //quit program
	{
		QL->promptToSave();
		QL->pKernel->Shutdown();
		delete QL->pKernel;
		exit(0);
	}
	else if (QL->first == _NEWP || QL->first == _NEWPS)  //clears process memory
	{
		QL->promptToSave();
		QL->commandStore.resize(0);
		QL->StuffToSave = false;
		QL->clearAll();	
		return toReturn;
	}
	else if (QL->first == _LOADP || QL->first == _LOADPS) //load a process
	{
		QL->loadingStep = true;
		QL->clearAll();
		QL->pCommand = "LOAD";  //flags used to indicate loading a process for other events
		QL->first = "DONE";
		QL->locFinder();
		cout << endl;
		QL->inFile.open(QL->loc.c_str());
		QL->printStep = true;
		QL->loadProcess();
		//QL->counter++;
		toReturn = "***VOIDRETURN***";
		return toReturn;
	}
	else if (QL->first == _SAVEP || QL->first == _SAVEPS) //save process
	{
		QL->saveProcChanges();
		toReturn = "***VOIDRETURN***";
		return toReturn;
	}
	else if (QL->first == _ENDPS || QL->first == _ENDP) //end loaded process
	{
		if(QL->pCommand == "LOAD")
			QL->endProcess();
		toReturn = "***VOIDRETURN***";
		return toReturn;
	}
	else if (QL->first == _RUNS || QL->first == _RUN)  //run soar til output
	{
		QL->userInput = false;
		QL->StuffToSave = true;
		QL->SC.resize(0);   //clears output storage
		QL->Icycle = false;  //gets out of outer loop
		QL->pAgent->RunSelfTilOutput(15);
		toReturn = "EndOfStep";
		return toReturn;
	}
	else if(QL->first == _RUNC || QL->first == _RUNCS)  //run soar for n cycles
	{
		QL->userInput = false;
		QL->StuffToSave = true;
		QL->SC.resize(0); //clears output storage
		int amount;
		*in >> amount;
		if(amount > 15)
			amount = 15;
		QL->Icycle = false;
		QL->pAgent->RunSelf(amount);
		toReturn = "EndOfStep";
		return toReturn;
	}
	else if(QL->first == _LASTO || QL->first == _LASTOS)  //re-print last output
	{
		cout << endl;
		QL->printOutput();
		return toReturn;
	}
	else if(QL->first == _CMDLIN || QL->first == _CL)  //execute command line command
	{
		char cmd[1000] ;  //used to get entire line including spaces
		in->getline(cmd,1000) ;
		string strcmd = cmd;
		cout << endl << QL->pKernel->ExecuteCommandLine(strcmd.c_str(), QL->pAgent->GetAgentName()) << endl;
		toReturn = "***VOIDRETURN***";
		return toReturn;
	}
	else if(QL->first == _TF || QL->first == _TREE_FORM)  //print structures in tree-form
	{
		cout << endl << "***Structures will now be printed in Tree-form***" << endl;
		QL->printTree = true;
		toReturn = QL->first;
		return toReturn;
	}
	else if (QL->first == _SF || QL->first == _SOAR_FORM)  //print structures in soar-form
	{
		cout << endl << "***Structures will now be printed in Soar-form***" << endl;
		QL->printTree = false;
		toReturn = QL->first;
		return toReturn;
	}
	else if(QL->first == "ENDOFSTEP")
	{
		QL->counter++;
		QL->loadingStep = false;
		QL->userInput = true;
		toReturn = "***VOIDRETURN***";
		return toReturn;
	}
	else if (QL->first == "ENDOFFILE")
	{
		QL->counter++;
		QL->loadingStep = false;
		QL->userInput = true;
		toReturn = "***VOIDRETURN***";
		return toReturn;
	}
	else if(QL->first[0] == '#')
	{
		char elget;
		in->get(elget);
		toReturn = "# ";
		while (elget != '\n')
		{
			toReturn += elget;
			in->get(elget);
		}
		return toReturn;
	}
	else 
	{
		char cmd[1000] ;  //used to get entire line including spaces
		in->getline(cmd,1000) ;
		string strcmd = cmd;
		cout << endl << QL->pKernel->ExecuteCommandLine(strcmd.c_str(), QL->pAgent->GetAgentName()) << endl;
		toReturn = "***VOIDRETURN***";
		return toReturn;
	}		


		

}