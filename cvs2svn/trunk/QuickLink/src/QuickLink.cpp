/*****************************************************
 *  Taylor Lafrinere
 *  
 *  Soar Quick Link
 *
 *  QuickLink.cpp
 *
 *  The purpose of this application is to be able to 
 *  control the input-link on soar from a command line
 *  and to read its output.
 *
 *  Start Date: 05.17.2005
 *
 *****************************************************/

#include "sml_Client.h"
#include "QuickLink.h"
#include <string>
#include <iostream>
#include <vector>
#include <cctype>
#include <fstream>
#include <process.h>
#include <windows.h>

using namespace std;


#if defined _WIN64 || defined _WIN32
 
void
QuickLink::OSFinder()
{
	OS = "windows";
}

#else

void 
QuickLink::OSFinder()
{
	OS = "not windows";
}
#endif




QuickLink::QuickLink()
{
	//OSFinder(); //needed for spawn debugger
	
	//Create an instance of Soar kernel
	pKernel = sml::Kernel::CreateKernelInNewThread("SoarKernelSML");


	//Error checking to see if kernel loaded correctly
	if (pKernel->HadError())
	{
		cout << pKernel->GetLastErrorDescription() << endl;
		return ;
	}

	//Create a Soar agent named "QuickLink"
	pAgent = pKernel->CreateAgent("QuickLink");

	//Make sure agent was successfully created
	if (pKernel->HadError())
	{
		cout << pKernel->GetLastErrorDescription() << endl;
		return ;
	}

	//*******INITIALIZE VARIABLES******
	
	garbage = ""; parent = ""; 	path = ""; command = "";
	pInputLink = pAgent->GetInputLink();
	Fvalue = 0; Ivalue = 0; counter = 0;
	
	pOnce = true, printStep = false, Icycle = true, printTree = false;

	//******CONSTANTS FOR PARSER******
	_QUIT = "QUIT";	_CLEAR = "CLEAR";	_SAVE = "SAVE";	_SAVES = "S";	_LOAD = "LOAD";
	_LOADS = "L";	_ADD = "ADD";	_ADDS = "A";	_CHANGE = "CHANGE";	_CHANGES = "C";
	_DELETE = "DELETE";	_DELETES = "D";	_DONE = "DONE";	_NEWP = "CLEARP";	_NEWPS = "CP";
	_LOADP = "LOADP";	_LOADPS = "LP";	_SAVEP = "SAVEP";	_SAVEPS = "SP";	_ENDP = "ENDP";
	_ENDPS = "EP";	_RUNCS = "RC";	_RUNC = "RUNCYCLE";	_RUN = "RUN";	_RUNS = "R";
	_LASTOS = "LO";	_LASTO = "LASTOUTPUT"; _CMDLIN = "CMDLIN"; _CL = "CL"; _SOAR_FORM = "SOAR-FORM";
	_SF = "SF"; _TREE_FORM = "TREE-FORM"; _TF = "TF";

	SoarSetup();
}

void
QuickLink::SoarSetup()
{
	/*if(OS == "windows")
		spawnDebug();  
	else	
	{
		cout << "Launch the debugger and connect it to QuickLink" << endl;
		WhenReady();
	}*/

	cout << endl << "Launch the debugger and connect it to QuickLink" << endl;
	WhenReady();

	cout << endl << "Load all productions into Soar using the debugger" << endl;
	WhenReady();

	pCommand = "NEW"; //pCommand is a flag used to identify process commands

	return;
}

void 
QuickLink::Run()
{
	while (true)
	{
		printStep = false;  //used to print step of process
		if (pCommand == "NEW" || pCommand == "RUN") //not running a process
			advMode();
		else if (inFile) //process file is open and has things left to load
		{			
			counter++;  //used to print step of process
			printStep = true;  //used to print step of process
			loadProcess();
			advMode();	
			pOnce = true;  //flag set so that end process message will be printed
		}
		else if (!inFile && pOnce)
			endProcess();
		else
			advMode();

		OutputCycle();
		pKernel->CheckForIncomingCommands();
	}
	pKernel->DestroyAgent(pAgent);
	delete pKernel;

	return;
}



void 
QuickLink::advMode()
{
	Icycle = true;
	//******INPUT******
	cout << "******INPUT****** " << endl << endl;

	while (Icycle)
	{
		PrintWorkingMem();

		cout << "> ";
		cin >> first;
		makeUpper(first);

		if (first == _CLEAR)  //clear current input-link structure
			clearAll();
		else if (first == _SAVE || first == _SAVES)  //save current input-link structure
		{
			locFinder();  //gets location of file
			ofstream tempFile;
			tempFile.open(loc.c_str());
			saveInput(true,tempFile);
			tempFile.close();
			tempFile.clear();
		}
		else if (first == _LOAD || first == _LOADS)  //load a saved input-link structure
		{
			locFinder();
			ifstream tmpFile;
			tmpFile.open(loc.c_str());
			loadInput(tmpFile);
			tmpFile.close();
			tmpFile.clear();
		}
		else if (first == _ADD || first == _ADDS)  //add something to il structure
		{
			cin >> second >> third >> fourth;  //used to make parser changes easier
			parent = second;
			path = third.substr(1, third.size()-1); //gets rid of ^
			if(fourth[0] == '#')  //IDENTIFER
			{				
				uniqid = fourth.substr(1, fourth.size()-1); //gets rid of #
				createID();
			}
			else //VALUE_BASED
			{
				value = fourth;
				advValue();
			}
		}
		else if (first == _CHANGE || first == _CHANGES) //change the value of something
		{
			cin >> second >> third >> fourth >> fifth;
			parent = second;
			path = third.substr(1, third.size()-1);  //gets rid of ^
			OldVal = fourth;
			NewVal = fifth;
			advAlter();
		}
		else if (first == _DELETE || first == _DELETES) //delete an element
		{
			cin >> second >> third >> fourth;
			parent = second;
			path = third.substr(1,third.size()-1);  //gets rid of ^
			if(fourth[0] == '#') //IDENTIFIER
			{
				uniqid = fourth.substr(1,fourth.size()-1);  //gets rid of #
				advDelInd();
			}
			else //VALUE_BASE
			{
				value = fourth;
				advDelVal();
			}
		}
		else if (first == _QUIT)  //quit program
		{
			promptToSave();
			pKernel->Shutdown();
			delete pKernel;
			exit(0);
		}
		else if (first == _NEWP || first == _NEWPS)  //clears process memory
		{
			promptToSave();
			aProc.resize(0);
			clearAll();			
		}
		else if (first == _LOADP || first == _LOADPS) //load a process
		{
			clearAll();
			pCommand = "LOAD";  //flags used to indicate loading a process for other events
			first = "DONE";
			locFinder();
			cout << endl;
			inFile.open(loc.c_str());
			printStep = true;
			loadProcess();
			counter++;
		}
		else if (first == _SAVEP || first == _SAVEPS) //save process
			saveProcChanges();
		else if (first == _ENDPS || first == _ENDP) //end loaded process
		{
			if(pCommand == "LOAD")
				endProcess();
		}
		else if (first == _RUNS || first == _RUN)  //run soar til output
		{
			SC.resize(0);   //clears output storage
			Icycle = false;  //gets out of outer loop
			forAlterProc();  //saves current input-link structure to process memory
			pAgent->RunSelfTilOutput(15);
		}
		else if(first == _RUNC || first == _RUNCS)  //run soar for n cycles
		{
			SC.resize(0); //clears output storage
			int amount;
			cin >> amount;
			if(amount > 15)
				amount = 15;
			Icycle = false;
			forAlterProc();  //saves current input-link structure to process memory
			pAgent->RunSelf(amount);
		}
		else if(first == _LASTO || first == _LASTOS)  //re-print last output
		{
			cout << endl;
			printOutput();
		}
		else if(first == _CMDLIN || first == _CL)  //execute command line command
		{
			char cmd[1000] ;  //used to get entire line including spaces
			cin.getline(cmd,1000) ;
			string strcmd = cmd;
			cout << endl << pKernel->ExecuteCommandLine(strcmd.c_str(), pAgent->GetAgentName()) << endl;
		}
		else if(first == _TF || first == _TREE_FORM)  //print structures in tree-form
		{
			cout << endl << "***Structures will now be printed in Tree-form***" << endl;
			printTree = true;
		}
		else if (first == _SF || first == _SOAR_FORM)  //print structures in soar-form
		{
			cout << endl << "***Structures will now be printed in Soar-form***" << endl;
			printTree = false;
		}
		else 
		{
			cout << "***INVALID COMMAND***" << endl;
			WhenReady();
		}		
	}
	pAgent->Commit();
}

void
QuickLink::loadProcess()
{
	loadInput(inFile);
	pAgent->Commit();
}

void
QuickLink::advValue()  //figure out what type the value is
{
	bool mFloat = false;
	for(unsigned int i = 0; i < value.size(); i++)
	{
		if (value[i] == '.')  //if it has a period it might be a float, not and int
			mFloat = true;
	}
	if(mFloat && (isdigit(value[0]) || isdigit(value[1]))) //value is float
	{
		command = "FLOAT";  //flag for createEL
		Fvalue = static_cast<float>(atof(value.c_str()));
		createEL();			
	}
	else if(isdigit(value[0])) //value is int
	{
		command = "INT";  //flag for createEL
        Ivalue = atoi(value.c_str());
		createEL();
	}
	else //value is string
	{
		command = "STRING";  //flag for createEL
		createEL();
	}
}

void
QuickLink::createID()
{
	if(parent == "il")
	{
		sml::Identifier* temp1;
		temp1 = pAgent->CreateIdWME(pInputLink, path.c_str());
		IDnames.push_back(path);
		IDparent.push_back(parent);
		IDsoar.push_back(uniqid);
		IDs.push_back(temp1);
	}
	else
	{
		int iI = -1;
		for(unsigned int i = 0; i < IDs.size(); i++)
		{
			if(IDsoar[i] == parent)
				iI = i;
		}
		if(iI == -1)  //parent doesn't exist
		{
			cout << endl << "ERROR: Parent name not found!" << endl << endl;
			WhenReady();
		}
		else
		{
			sml::Identifier* temp1;
			temp1 = pAgent->CreateIdWME(IDs[iI], path.c_str());
			IDnames.push_back(path);
			IDparent.push_back(parent);
			IDsoar.push_back(uniqid);
			IDs.push_back(temp1);
		}					
	}
}

void
QuickLink::createEL()
{
	int pI = -1;

	for(unsigned int i = 0; i < IDs.size(); i++)  //find parent
	{
		if(IDsoar[i] == parent)
			pI = i;
	}
	if(pI == -1 && parent != "il") //parent doesn't exist
	{
		cout << endl << "ERROR: Parent name not found!" << endl << endl;
		WhenReady();
	}
	else
	{
		if (command == "STRING")
		{
			SEnames.push_back(path);
			SEvalue.push_back(value);
			if(parent == "il")  //create on input-link
			{
				SEparent.push_back(parent);
				sml::StringElement* temp2 = pAgent->CreateStringWME(pInputLink, path.c_str(), value.c_str());
				SEs.push_back(temp2);
			}
			else
			{
				SEparent.push_back(parent);
				sml::StringElement* temp2 = pAgent->CreateStringWME(IDs[pI], path.c_str(), value.c_str());
				SEs.push_back(temp2);
			}

		}
		else if (command == "INT")
		{
            IEnames.push_back(path);
			IEvalue.push_back(Ivalue);
			if(parent == "il")
			{
				IEparent.push_back(parent);
				sml::IntElement* temp3 = pAgent->CreateIntWME(pInputLink, path.c_str(), Ivalue);
				IEs.push_back(temp3);
			}
			else
			{
				IEparent.push_back(parent);
				sml::IntElement* temp3 = pAgent->CreateIntWME(IDs[pI], path.c_str(), Ivalue);
				IEs.push_back(temp3);
			}						
		}
		else
		{
			FEnames.push_back(path);
			FEvalue.push_back(Fvalue);
			if(parent == "il")
			{
				FEparent.push_back(parent);
				sml::FloatElement* temp4 = pAgent->CreateFloatWME(pInputLink, path.c_str(), Fvalue);
				FEs.push_back(temp4);
			}
			else
			{
				FEparent.push_back(parent);
				sml::FloatElement* temp4 = pAgent->CreateFloatWME(IDs[pI], path.c_str(), Fvalue);
				FEs.push_back(temp4);
			}	

		}
	}
}

void
QuickLink::PrintWorkingMem()
{
	cout << endl << "******CURRENT INPUT-LINK STRUCTURE******" << endl << endl;
	if (printStep)
	{
		cout << "******Step " << counter << " of Process " << processExt << "******" << endl << endl;
	}
	//Resize all to size of respective element number
	IDprint.resize(IDs.size());
	FEprint.resize(FEs.size());
	IEprint.resize(IEs.size());
	SEprint.resize(SEs.size());

	//set all to false
	for(unsigned int i = 0; i < IDs.size(); i++)
		IDprint[i] = false;
	for(unsigned int i = 0; i < IEs.size(); i++)
		IEprint[i] = false;
	for(unsigned int i = 0; i < SEs.size(); i++)
		SEprint[i] = false;
	for(unsigned int i = 0; i < FEs.size(); i++)
		FEprint[i] = false;
	if(printTree)
	{
		cout << "^input-link [il]" << endl;
		WMrecurse("il","  ",true);
	}
	else
		printSoarInForm();	
	cout << endl;
	
}

void 
QuickLink::WMrecurse(string father, string indent, bool flag)
{
	unsigned int i;
	for(i = 0; i < IDs.size(); i++)
	{
		if(IDparent[i] == father && IDprint[i] == false)
		{
			cout << indent << "^" << IDnames[i] << " [" << IDsoar[i] <<"] " << endl;
			IDprint[i] = true;
            WMrecurse(IDsoar[i], indent + "  ",true);
		}
		if(flag)  
		{
			for(unsigned int s = 0; s < SEs.size(); s++)
			{
				if(SEparent[s] == father && SEprint[s] == false)
				{
					cout << indent << "^" << SEnames[s] << "  " << SEvalue[s] << endl;
					SEprint[s] = true;
				}
			}
			for(unsigned int f = 0; f < FEs.size(); f++)
			{
				if(FEparent[f] == father && FEprint[f] == false)
				{
					cout << indent << "^" << FEnames[f] << "  " << FEvalue[f] << endl;
					FEprint[f] = true;
				}
			}
			for(unsigned int n = 0; n < IEs.size(); n++)
			{
				if(IEparent[n] == father && IEprint[n] == false)
				{
					cout << indent << "^" << IEnames[n] << "  " << IEvalue[n] << endl;
					IEprint[n] = true;
				}
			}
			flag = false;
		}		
	}
	if(i == 0)
	{
		for(unsigned int s = 0; s < SEs.size(); s++)
		{
			if(SEparent[s] == father)
			{
				cout << indent << "^" << SEnames[s] << "  " << SEvalue[s] << endl;
			}
		}
		for(unsigned int f = 0; f < FEs.size(); f++)
		{
			if(FEparent[f] == father)
			{
				cout << indent << "^" << FEnames[f] << "  " << FEvalue[f] << endl;
			}
		}
		for(unsigned int n = 0; n < IEs.size(); n++)
		{
			if(IEparent[n] == father)
			{
				cout << indent << "^" << IEnames[n] << "  " << IEvalue[n] << endl;
			}
		}
		flag = false;
	}
}

void 
QuickLink::printSoarInForm()
{	
	vector<spaceControl> toPrint;  //queue of identifiers to print
	spaceControl tmp;  //controls indent level
	tmp.iden = "il";
	tmp.indent = "";
	toPrint.push_back(tmp);
	for(unsigned int j = 0; j < toPrint.size(); j++)
	{
		if(IDs.size() > 0)
			cout << toPrint[j].indent << "(" << toPrint[j].iden;
		else  //nothing in input-link structure
			cout << "(il)" << endl << endl;
		int length = (3 + toPrint[j].indent.size() + toPrint[j].iden.size());  //controls word wrapping
		for(unsigned int i = 0; i < IDs.size(); i++)
		{
			if(IDparent[i] == toPrint[j].iden)
			{
				int tmp = (length + IDnames[i].size() + IDsoar[i].size() +3);
				if(tmp < 78)//controls word wrapping
				{	cout << " ^" << IDnames[i] << " " << IDsoar[i];
					length = tmp;
				}
				else
				{	cout << endl << toPrint[j].indent << "    " << " ^" << IDnames[i] << " " << IDsoar[i];
					length = (4 + toPrint[j].indent.size());  //controls word wrapping
				}
				IDprint[i] = true;
				spaceControl tmp2;
				tmp2.iden = IDsoar[i];
				tmp2.indent = ("  " + toPrint[j].indent);
				toPrint.push_back(tmp2);
			}
		}
		for(unsigned int i = 0; i < SEs.size(); i++)
			if(SEparent[i] == toPrint[j].iden)
			{
				int tmp = (length + SEnames[i].size() + SEvalue[i].size() +3);
				if(tmp < 78)  //controls word wrapping
				{	cout << " ^" << SEnames[i] << " " << SEvalue[i];
					length = tmp;  //controls word wrapping 
				}
				else
				{	cout << endl << toPrint[j].indent << "    " << " ^" << SEnames[i] << " " << SEvalue[i];
					length = (4 + toPrint[j].indent.size());  //controls word wrapping
				}
			}
		for(unsigned int i = 0; i < IEs.size(); i++)
			if(IEparent[i] == toPrint[j].iden)
			{
				int tmp = (length + IEnames[i].size() + 8);
				if(tmp < 78)  //controls word wrapping
				{	cout << " ^" << IEnames[i] << " " << IEvalue[i];
					length = tmp;  //controls word wrapping
				}
				else
				{	cout << endl << toPrint[j].indent << "    " << " ^" << IEnames[i] << " " << IEvalue[i];
					length = (4 + toPrint[j].indent.size());  //controls word wrapping
				}
			}
		for(unsigned int i = 0; i < FEs.size(); i++)
			if(FEparent[i] == toPrint[j].iden)
			{
				int tmp = (length + FEnames[i].length() + 9);
				if(tmp < 78)  //controls word wrapping
				{	cout << " ^" << FEnames[i] << " " << FEvalue[i]; 
					length = tmp;  //controls word wrapping
				}
				else
				{	cout << endl << toPrint[j].indent << "    " << " ^" << FEnames[i] << " " << FEvalue[i];
					length = (4 + toPrint[j].indent.size());  //controls word wrapping
				}
			}
		if(IDs.size() > 0)
			cout << ")" << endl;
		
	}
}

void
QuickLink::makeUpper(string & tosmall)
{
	for(unsigned int ii = 0; ii < tosmall.size(); ii++)
		tosmall[ii]=toupper(tosmall[ii]);
	return;
}

void
QuickLink::OutputCycle()
{
	storeO.resize(0);
	
	//******GET OUTPUT******	

	int numberCommands = pAgent->GetNumberOutputLinkChanges() ;
	int numberCommands2 = pAgent->GetNumberCommands();
	for(int i = 0; i < numberCommands; i++)
	{
		if(pAgent->IsOutputLinkChangeAdd(i))
		{
			triple trip;
			sml::WMElement* tmp =pAgent->GetOutputLinkChange(i) ;
			trip.name = tmp->GetIdentifierName() ;
			trip.att = tmp->GetAttribute();
			trip.val = tmp->GetValueAsString();
			trip.printed = false;
			storeO.push_back(trip);
		}		
	}

	printOutput();
	
	for(int i = 0; i< numberCommands2; i++)  //add's status complete
	{
		sml::Identifier* tmp2 = pAgent->GetCommand(i);
		tmp2->AddStatusComplete();
		pAgent->Commit();
	}
	pKernel->CheckForIncomingCommands();

	//pretty sure these next 3 lines are not needed, not sure though
	numberCommands = pAgent->GetNumberOutputLinkChanges() ;
	numberCommands2 = pAgent->GetNumberCommands();
	
	pAgent->ClearOutputLinkChanges();	

	return;
}

bool
QuickLink::displayTrips(string lookfor, string indent)  //output tree printer
{
    bool toReturn = true;
	for(unsigned int i = 0; i < storeO.size(); i++)
	{
		if(storeO[i].name == lookfor)
		{
			cout << endl << indent << "^" << storeO[i].att;
			if(displayTrips(storeO[i].val, indent + "  "))
			{
				cout << "  " << storeO[i].val ;
			}
			toReturn = false;
		}
	}
	return toReturn;
}

void
QuickLink::printSoarOutForm()
{
	for(unsigned int i = 0; i < storeO.size(); i++)
	{
		if(storeO[i].printed == false)
		{
			string indent = "";
			string iden;
			iden = storeO[i].name;
			for(unsigned int s = 0; s < SC.size(); s++)
				if(iden == SC[s].iden)
					indent = ("  " + SC[s].indent);
			spaceControl tmp;
			tmp.iden = storeO[i].val;
			tmp.indent = indent;
			SC.push_back(tmp);

			cout << indent << "(" << iden << " ^" << storeO[i].att << " " << storeO[i].val;
			storeO[i].printed = true;
			int returner = 1;  //controls word wrapping
			for(unsigned int j = 0; j < storeO.size(); j++)
			{
				if(storeO[j].name == iden && !storeO[j].printed)
				{
					if(returner > 3)  //controls word wrapping
					{
						cout << endl << indent;
						returner = 0;  //controls word wrapping
					}
					else
					{
						returner++;
						cout << " ";
					}
					cout << "^" << storeO[j].att << " " << storeO[j].val << " ";
					storeO[j].printed = true;
					spaceControl tmp2;
					tmp2.iden = storeO[j].val;
					tmp2.indent = indent;
					SC.push_back(tmp2);
				}
			}
			cout << ")" << endl;
		}
	}
	for(unsigned int i = 0; i < storeO.size(); i++)  //resets all printed flags to false
		storeO[i].printed = false;
}

void
QuickLink::loadInput(ifstream& iFile)
{
	if(!iFile)
	{
		cout << "ERROR: File failed to open!" << endl;
		iFile.clear();
		printStep = false;
		counter--; //keeps step number correct
		WhenReady();
	}
	else
	{
		int lpi = -1;
		for (unsigned int i = 0; i < loadPair.size(); i++)
			if (&iFile == loadPair[i].str)  //trick used so that multiple load files can be open at
				lpi = i;					//same time that all use this function
		if(lpi == -1)
		{
			pair tmp;
			tmp.str = &iFile;
			tmp.title = "";
			loadPair.push_back(tmp);
			lpi = loadPair.size() -1;
		}	
		if (loadPair[lpi].title == "")
            iFile >> loadPair[lpi].title;
		while(loadPair[lpi].title == "id")  //loads identifiers
		{
			iFile >> parent;
			iFile >> path;
			iFile >> uniqid;
			bool tester = true;
			for(unsigned int i = 0; i < IDs.size() && tester; i++)  //checks to see if element already exists
				if(IDparent[i] == parent && IDnames[i] == path && IDsoar[i] == uniqid)
					tester = false;
			if(tester)	
                createID();
			iFile >> loadPair[lpi].title;  
		}
		while(loadPair[lpi].title == "ie") //loads int elements
		{
			iFile >> parent;
			iFile >> path;
			iFile >> Ivalue;
			command = "INT";
			bool tester = true;
			for(unsigned int i = 0; i < IEs.size() && tester; i++)  //checks to see if element already exists
				if(IEparent[i] == parent && IEnames[i] == path && IEvalue[i] == Ivalue)
					tester = false;
			if(tester)	
				createEL();
			iFile >> loadPair[lpi].title;
		}
		while(loadPair[lpi].title == "fe") //loads float elements
		{
			iFile >> parent;
			iFile >> path;
			iFile >> Fvalue;
			command = "FLOAT";
			bool tester = true;
			for(unsigned int i = 0; i < FEs.size() && tester; i++)  //checks to see if element already exists
				if(FEparent[i] == parent && FEnames[i] == path && FEvalue[i] == Fvalue)
					tester = false;
			if(tester)	
				createEL();
			iFile >> loadPair[lpi].title;
		}
		while(loadPair[lpi].title == "se")  //loads string elements
		{
			iFile >> parent;
			iFile >> path;
			iFile >> value;
			command = "STRING";
			bool tester = true;
			for(unsigned int i = 0; i < SEs.size() && tester; i++)  //checks to see if element already exists
				if(SEparent[i] == parent && SEnames[i] == path && SEvalue[i] == value)
					tester = false;
			if(tester)	
				createEL();
			iFile >> loadPair[lpi].title;
		}
		iFile >> loadPair[lpi].title;  //gets next title, used mainly to initiate eof
	}
}



void
QuickLink::delSE(int index)
{
	int last = SEs.size() - 1;
	pAgent->DestroyWME(SEs[index]);
	SEs[index] = SEs[last];
	SEnames[index] = SEnames[last];
	SEparent[index] = SEparent[last];
	SEvalue[index] = SEvalue[last];
	SEs.pop_back();
	SEnames.pop_back();
	SEparent.pop_back();
	SEvalue.pop_back();
}

void
QuickLink::delIE(int index)
{
	int last = IEs.size() - 1;
	pAgent->DestroyWME(IEs[index]);
	IEs[index] = IEs[last];
	IEnames[index] = IEnames[last];
	IEparent[index] = IEparent[last];
	IEvalue[index] = IEvalue[last];
	IEs.pop_back();
	IEnames.pop_back();
	IEparent.pop_back();
	IEvalue.pop_back();
}

void
QuickLink::delFE(int index)
{
	int last = FEs.size() - 1;
	pAgent->DestroyWME(FEs[index]);
	FEs[index] = FEs[last];
	FEnames[index] = FEnames[last];
	FEparent[index] = FEparent[last];
	FEvalue[index] = FEvalue[last];
	FEs.pop_back();
	FEnames.pop_back();
	FEparent.pop_back();
	FEvalue.pop_back();
}

void
QuickLink::delID(int index)
{
	int last = IDs.size() - 1;
	pAgent->DestroyWME(IDs[index]);
	IDs[index] = IDs[last];
	IDnames[index] = IDnames[last];
	IDparent[index] = IDparent[last];
	IDsoar[index] = IDsoar[last];
	IDs.pop_back();
	IDnames.pop_back();
	IDparent.pop_back();
	IDsoar.pop_back();
}

void
QuickLink::deleteChilds(string father)
{
	int ind = -1;
	for(unsigned int j = 0; j < IDs.size(); j++)
	{
		if(IDparent[j] == father)
		{
			ind = j;
			deleteChilds(IDsoar[ind]);
			delID(ind);
			j--;  //needed because of way delete is made
		}
	}
	ind = -1;
	for(unsigned int j = 0; j < IEs.size(); j++)
	{
		if(IEparent[j] == father)
		{
			ind = j;
			delIE(ind);
			j--;  //needed because of way delete is made
		}
	}
	ind = -1;
	for(unsigned int j = 0; j < FEs.size(); j++)
	{
		if(FEparent[j] == father)
		{
			ind = j;
			delFE(ind);
			j--;  //needed because of way delete is made
		}
	}
	ind = -1;
	for(unsigned int j = 0; j < SEs.size(); j++)
	{
		if(SEparent[j] == father)
		{
			ind = j;
			delSE(ind);
			j--;  //needed because of way delete is made
		}
	}			
}

void 
QuickLink::forAlterProc()
{
	vector<wme> VecWme;
	for(unsigned int i =0; i< IDs.size(); i++)
	{
		wme tmpWme;
		tmpWme.title = "id";
		tmpWme.parent = IDparent[i];
		tmpWme.name = IDnames[i];
		tmpWme.value = IDsoar[i]; 
		tmpWme.num = 0;
		tmpWme.dec = 0;
		tmpWme.fl = false;
		VecWme.push_back(tmpWme);
	}
	for(unsigned int i =0; i< IEs.size(); i++)
	{
		wme tmpWme;
		tmpWme.title = "ie";
		tmpWme.parent = IEparent[i];
		tmpWme.name = IEnames[i];
		tmpWme.num = IEvalue[i];
		tmpWme.value = "";
		tmpWme.dec = 0;
		tmpWme.fl = false;
		VecWme.push_back(tmpWme);
	}
	for(unsigned int i =0; i< FEs.size(); i++)
	{
		wme tmpWme;
		tmpWme.title = "fe";
		tmpWme.parent = FEparent[i];
		tmpWme.name = FEnames[i];
		tmpWme.dec = FEvalue[i]; 
		tmpWme.num = 0;
		tmpWme.value = "";
		tmpWme.fl = true;
		VecWme.push_back(tmpWme);
	}
	for(unsigned int i =0; i< SEs.size(); i++)
	{
		wme tmpWme;
		tmpWme.title = "se";
		tmpWme.parent = SEparent[i];
		tmpWme.name = SEnames[i];
		tmpWme.value = SEvalue[i]; 
		tmpWme.num = 0;
		tmpWme.dec = 0;
		tmpWme.fl = false;
		VecWme.push_back(tmpWme);
	}
	aProc.push_back(VecWme);
}

void
QuickLink::saveProcChanges()
{
	locFinder();
	cout << endl;
	ofstream aFile;
	aFile.open(loc.c_str());

	for(unsigned int j = 0; j < aProc.size(); j++)
	{
		for(unsigned int i = 0; i < aProc[j].size(); i++)
		{
			aFile << aProc[j][i].title << " " << aProc[j][i].parent << " " << aProc[j][i].name << " " ;
			if(aProc[j][i].fl == true)
				aFile << aProc[j][i].dec << endl;
			else if (aProc[j][i].value != "")
				aFile << aProc[j][i].value << endl;
			else
				aFile << aProc[j][i].num << endl;
		}
		aFile << "&&&" << endl;
	}

	cout << endl << endl << "***Your process has been saved***" << endl;
	WhenReady();
}

void 
QuickLink::saveInput(bool toClose, ofstream& oFile)
{
	if(!oFile)
	{
		cout << endl << "***File failed to open***" << endl << endl;
		WhenReady();
	}
	else //write to file
	{	
		for(unsigned int i =0; i< IDs.size(); i++)
		{
			oFile << "id " << IDparent[i] << " " << IDnames[i] << " " << IDsoar[i] << endl;
		}
		for(unsigned int i =0; i< IEs.size(); i++)
		{
			oFile << "ie " << IEparent[i] << " " << IEnames[i] << " " << IEvalue[i] << endl;
		}
		for(unsigned int i =0; i< FEs.size(); i++)
		{
			oFile << "fe " << FEparent[i] << " " << FEnames[i] << " " << FEvalue[i] << endl;
		}
		for(unsigned int i =0; i< SEs.size(); i++)
		{
			oFile << "se " << SEparent[i] << " " << SEnames[i] << " " << SEvalue[i] << endl;
		}

		oFile << "&&&" << endl;  //prints delimeter
		if (toClose)  //only closes structure files, not process files
		{
			oFile.close();
			oFile.clear();
			cout << "***YOUR FILE HAS BEEN SAVED***" << endl << endl;
			WhenReady();
		}
	}
}

void
QuickLink::advDelInd()  //deletes identifiers
{
	int ind = -1;
	for (unsigned int i = 0; i < IDs.size(); i ++)  //find element
	{
		if(IDparent[i] == parent && IDnames[i] == path && IDsoar[i] == uniqid)
			ind = i;
	}
	if (ind == -1)  //doesn't exist
	{
		cout << "ERROR: Either the parent, name or value specified does not exist!" << endl << endl;
		WhenReady();
	}
	else
	{
		deleteChilds(IDsoar[ind]);
		delID(ind);
		cout << endl << "***WME HAS BEEN DELETED***" << endl << endl;
		WhenReady();
	}			
}

void
QuickLink::advDelVal()  //deletes value based elements
{
	int ind = -1;
	bool mFloat = false;
	for(unsigned int i = 0; i < value.size(); i++)
	{
		if (value[i] == '.')
			mFloat = true;
	}
	if(mFloat && (isdigit(value[0]) || isdigit(value[1]))) //value is float
	{
		Fvalue = static_cast<float>(atof(value.c_str()));
		for(unsigned int i = 0; i < FEs.size(); i++)
		{
			if(FEparent[i] == parent && FEnames[i] == path && FEvalue[i] == Fvalue)
				ind = i;
		}
		if (ind == -1)
		{
			cout << "ERROR: Either the parent, name or value specified does not exist!" << endl << endl;
			WhenReady();
		}
		else
		{
			delFE(ind);
			cout << endl << "***WME HAS BEEN DELETED***" << endl << endl;
			WhenReady();
		}

	}
	else if(isdigit(value[0])) //value is int
	{
		Ivalue = static_cast<int>(atoi(value.c_str()));
		for(unsigned int i = 0; i < IEs.size(); i++)
		{
			if(IEparent[i] == parent && IEnames[i] == path && IEvalue[i] == Ivalue)
				ind = i;
		}
		if (ind == -1)
		{
			cout << "ERROR: Either the parent, name or value specified does not exist!" << endl << endl;
			WhenReady();
		}
		else
		{
			delIE(ind);
			cout << endl << "***WME HAS BEEN DELETED***" << endl << endl;
			WhenReady();
		}

	}
	else //value is string
	{
		for(unsigned int i = 0; i < SEs.size(); i++)
		{
			if(SEparent[i] == parent && SEnames[i] == path && SEvalue[i] == value)
				ind = i;
		}
		if (ind == -1)
		{
			cout << "ERROR: Either the parent, name or value specified does not exist!" << endl << endl;
			WhenReady();
		}
		else
		{
			delSE(ind);
			cout << endl << "***WME HAS BEEN DELETED***" << endl << endl;
			WhenReady();
		}

	}
}

void
QuickLink::advAlter()
{
	int index = -1;
	bool mFloat = false;
	for(unsigned int i = 0; i < NewVal.size(); i++)
	{
		if (NewVal[i] == '.')
			mFloat = true;
	}
	if(mFloat && (isdigit(NewVal[0]) || isdigit(NewVal[1]))) //value is float
	{
		float OFvalue = static_cast<float>(atof(OldVal.c_str()));
		for(unsigned int i = 0; i < FEs.size(); i++)
		{
			if(FEparent[i] == parent && FEnames[i] == path && FEvalue[i] == OFvalue)
				index = i;
		}
		if (index == -1)
		{
			cout << "ERROR: Either the parent, name or value specified does not exist!" << endl << endl;
			WhenReady();
		}
		else
		{
			Fvalue = static_cast<float>(atof(NewVal.c_str()));
			pAgent->Update(FEs[index],Fvalue);
			FEvalue[index] = Fvalue;
		}					
	}
	else if(isdigit(NewVal[0])) //value is int
	{
		int OIvalue = atoi(OldVal.c_str());
		for(unsigned int i = 0; i < IEs.size(); i++)
		{
			if(IEparent[i] == parent && IEnames[i] == path && IEvalue[i] == OIvalue)
				index = i;
		}
		if (index == -1)
		{
			cout << "ERROR: Either the parent, name or value specified does not exist!" << endl << endl;
			WhenReady();
		}
		else
		{
			Ivalue = atoi(NewVal.c_str());
			pAgent->Update(IEs[index],Ivalue);
			IEvalue[index] = Ivalue;
		}					
	}
	else //value is string
	{
		for(unsigned int i = 0; i < SEs.size(); i++)
		{
			if(SEparent[i] == parent && SEnames[i] == path && SEvalue[i] == OldVal)
				index = i;
		}
		if (index == -1)
		{
			cout << "ERROR: Either the parent, name or value specified does not exist!" << endl << endl;
			WhenReady();
		}
		else
		{
			pAgent->Update(SEs[index],NewVal.c_str());
			SEvalue[index] = NewVal;
		}					
	}
}

void
QuickLink::endProcess()
{
	inFile.clear();
	inFile.close();
	loadPair.resize(0);
	cout << endl << "***Your process file has ended***" << endl << endl;
	WhenReady();
	counter = 0;
	pCommand = "NEW";
	printStep = false;
	pOnce = false;
	advMode();	
}

void 
QuickLink::printOutput()
{
	cout << endl << endl << "******OUTPUT******" << endl;
	if(printTree)  //tree-form
	{
		cout << endl << "^output-link";
		bool useless = displayTrips("I3","  ");
	}
	else
		printSoarOutForm();  //soar-form
	
	cout << endl << endl << "******END OF OUTPUT******" << endl << endl;
	WhenReady();
}

void
QuickLink::promptToSave()
{
	if(aProc.size() != 0)
	{
		string toSave;
		cout << endl << "Would you like to save the past " << aProc.size() << " steps as a process before" 
			<< endl << "they are destroyed?: ";
		cin >> toSave;
		makeUpper(toSave);
		while(toSave != "YES" && toSave != "NO")
		{
			cout << endl << endl << "Please enter YES or NO: ";
			cin >> toSave;
			makeUpper(toSave);
		}
		if (toSave == "YES")
		{
			cout << endl << "Please enter the name of the file to save your changes to: ";
			saveProcChanges();
		}
	}
}

void
QuickLink::WhenReady()
{
	cout << "Press any non white-space key + enter to continue: ";
	cout << endl;
	cin >> garbage;
	cout << endl << endl;
}

void
QuickLink::clearAll()
{
	deleteChilds("il");
}

void
QuickLink::locFinder()
{
	string fileName;
	string tmp;
	char garbage;
	cin.get(garbage);
	while(cin.peek() != '\n')
	{
		cin >> tmp;
		if(fileName != "")
			tmp = " " + tmp;
		fileName += tmp;
	}
	
	string location = "";//QuickLink\\ProcessesAndStructures\\";

	loc = location + fileName;	

}

void
QuickLink::spawnDebug()
{
	// spawn the debugger asynchronously
	int ret = _spawnlp(_P_NOWAIT, "javaw.exe", "javaw.exe", "-jar", "SoarJavaDebugger.jar", "-remote", NULL);
	if(ret == -1) {
		switch (errno) {
			case E2BIG:
				cout << "arg list too long";
				break;
			case EINVAL:
				cout << "illegal mode";
				break;
			case ENOENT:
				cout << "file/path not found";
				break;
			case ENOEXEC:
				cout << "specified file not an executable";
				break;
			case ENOMEM:
				cout << "not enough memory";
				break;
			default:
				cout << ret;
		}
	}
	Sleep(3500);
}

int main ()
{		
	
	QuickLink QL;
	QL.Run();
	
	return 0;
}


