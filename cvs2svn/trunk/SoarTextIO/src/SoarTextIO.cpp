/*****************************************************
*  Author: Taylor Lafrinere
*  
*  Soar SoarTextIO
*
*  SoarTextIO.cpp
*
*  The purpose of this application is to allow the user
*  to freely type random words and sentences and have them
*  appear on the input-link.
*
*  Start Date: 06.23.2005
*
*****************************************************/

#include "SoarTextIO.h"
#include "stdlib.h"
#include <iostream>
#include <process.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include <string>

using namespace sml;

void MyUpdateEventHandler(sml::smlUpdateEventId id, void* pUserData, sml::Kernel* pKernel, sml::smlRunFlags runFlags);
void MyStartSystemEventHandler(smlSystemEventId id, void* pUserData, Kernel* pKernel);
void MyStopSystemEventHandler(smlSystemEventId id, void* pUserData, Kernel* pKernel);
void MyAgentEventHandler(smlAgentEventId id, void* pUserData, Agent* pAgent) ;
int callBackId;

SoarTextIO::SoarTextIO()
{
	init_soar = false;
	RemoteConnect();
	cout << endl;	
	
	
}

SoarTextIO::~SoarTextIO()
{
	pKernel->Shutdown();
	delete pKernel;
}

void
SoarTextIO::init()
{
	//******INITIALIZE VARIABLES******
	sentenceNum = 1; 
	wordNum = 0;
	if(!init_soar)
	{
		pInputLink = pAgent->GetInputLink();
		pOutputLink = pAgent->GetOutputLink();
	}
	
	

	loadPlease = false;
	subtractOne = false;

	agentName = pAgent->GetAgentName();
	m_IsRunning = false;
	m_StopNow = false;
	initiateRes = false;
	initiateRem = false;
	printNow = true;
	ShouldPrintNow = false;
	PrintNothing = false;
	
	if(!init_soar)
	{
		
		WhenReady();
		callBackId = pKernel->RegisterForUpdateEvent(sml::smlEVENT_AFTER_ALL_OUTPUT_PHASES,MyUpdateEventHandler,this);
		callBackId = pKernel->RegisterForSystemEvent(smlEVENT_SYSTEM_START,MyStartSystemEventHandler, this );
		callBackId = pKernel->RegisterForSystemEvent(smlEVENT_SYSTEM_STOP, MyStopSystemEventHandler, this);
		callBackId = pKernel->RegisterForAgentEvent(smlEVENT_BEFORE_AGENT_REINITIALIZED, MyAgentEventHandler, this);
		inFile.open("!@#$%^&*");

		run();
	}

	init_soar = false;
	
}



void
SoarTextIO::run()
{
	while(checker != "--QUIT")
	{
		sentStore.resize(0);
		if(initiateRem)
		{
			initiateRem = false;
			//KillKernel();
			RemoteConnect();
		}
		else if(initiateRes)
		{
			initiateRes = false;
			//KillKernel();
			ResetConnect();
		}
		WriteCycle(&cin);
		if(checker != "--STEP" && checker != "--RESET" && checker != "--REMOTE" && checker != "--CMDLIN" && checker != "--DEBUG" && checker != "--RUN" && checker != "--STOP" && checker != "--QUIT" && checker != "--SAVE" && checker != "--LOAD")
		{
			if(NextWord.size() > 0)
			{
				pAgent->DestroyWME(NextWord[NextWord.size()-1]); //gets rid of null-pointing next-word identifier
				pAgent->CreateStringWME(NextWord[NextWord.size()-2],"next","nil");
			}
			sentenceNum++;		
		}		
	}
}

void
SoarTextIO::runner()
{
	_beginthread( RunForever, 0, this); //thread for the RunForever() command
}

void
SoarTextIO::RunForever( void* info )
{
	SoarTextIO* STIO = (SoarTextIO*)info;
	if(!STIO->pKernel->IsSoarRunning())
	{
		STIO->m_IsRunning = true;
		STIO->pKernel->RunAllAgentsForever();
	}
	STIO->m_StopNow = false;
	
}


void
MyUpdateEventHandler(smlUpdateEventId id, void* pUserData, Kernel* pKernel, smlRunFlags runFlags)
{
	SoarTextIO* STIO = (SoarTextIO*)pUserData;
	if(STIO->m_StopNow == true)
	{
		pKernel->StopAllAgents();
		STIO->m_StopNow = false;
		STIO->m_IsRunning = false;
	}
	if(STIO->pAgent->GetNumberOutputLinkChanges()!=0)
	{
		STIO->RespondCycle();
		STIO->pAgent->ClearOutputLinkChanges();
	}	
	STIO->pAgent->Commit();
}

void
MyStopSystemEventHandler(smlSystemEventId id, void* pUserData, Kernel* pKernel)
{
	SoarTextIO* STIO = (SoarTextIO*)pUserData;
	STIO->m_IsRunning = false;
}

void
MyStartSystemEventHandler(smlSystemEventId id, void* pUserData, Kernel* pKernel)
{
	SoarTextIO* STIO = (SoarTextIO*)pUserData;
	STIO->m_IsRunning = true;
}

void
MyAgentEventHandler(smlAgentEventId id, void* pUserData, Agent* pAgent)
{
	SoarTextIO* STIO = (SoarTextIO*)pUserData;
	if(STIO->LastSent.size() > 0)
	{
		STIO->pAgent->DestroyWME(STIO->LastSent[0]);
		STIO->LastSent.resize(0);
	}
//	if(STIO->pTextInput != NULL)
//		STIO->pAgent->DestroyWME(STIO->pTextInput);
//	STIO->init_soar = true;
}

void
SoarTextIO::WriteCycle(istream* getFrom)
{
	wordNum = 0;
	NextWord.resize(0);
	bool printRead = false;
	bool created = false;	

	word = "", forMem = "";
	checker = "";
	while(!printNow) { Sleep(1); }
	if(*getFrom == cin)
        cout << endl << endl << endl << endl << "> ";
	if(ShouldPrintNow)
	{
		cout << "READ FROM FILE: ";
		if(!PrintNothing)
			cout << memory[memory.size() - 1] << endl;
		else
			cout << "<NOTHING>" << endl;
		ShouldPrintNow = false;
	}
	else
		printRead = true; 

	
	while(checker != "--STEP" && checker != "--RESET" && checker != "--REMOTE" && checker != "--CMDLIN" && checker != "--DEBUG" && checker != "--RUN" && checker != "--STOP" && checker != "--QUIT" && checker != "--SAVE" && checker != "--LOAD" && getFrom->peek() != '\n' && getFrom->peek() != EOF && checker != "#&#&" )
	{
		wordNum++;
		word = "";
		*getFrom >> word;
		if(init_soar)
			init();  //reintializes SoarTextIO
		while(word == "")
		{
			cout << "> ";
			if(!*getFrom)
				getFrom = &cin;
			*getFrom >> word;
		}
		if(pKernel)
			checker = word;
		makeUpper(checker);
		if(checker == "--NEXTLINE")
		{
			ShouldPrintNow = true;
			getFrom = &inFile;
			GetFileInput(word);
			if(!*getFrom)
			{
				CloseFile();
				checker = "#&#&";  // this just gets us out of next if statement, this is bad and needs to be changed
			}
		}
		if(checker != "--STEP" && checker != "--RESET" && checker != "--REMOTE" && checker != "--CMDLIN" && checker != "--DEBUG" && checker != "--RUN" && checker != "--QUIT" && checker != "--SAVE" && checker != "--LOAD" && checker != "#&#&" && checker != "--STOP")
		{
			if(!created)
			{
				createSentId();	
				sml::IntElement* tmp2 = pAgent->CreateIntWME(sentStore[0],"text-input-number", sentenceNum);
				sml::Identifier* tmp3 = pAgent->CreateIdWME(NextWord[0], "next");
				NextWord.push_back(tmp3);
				sentStore.push_back(tmp3);

				created = true;
			}
			CreateWord();
			forMem += (word + " ");
		}
		else if(checker == "--SAVE")
			saveMem();
		else if(checker == "--LOAD")
			loadMem();
		else if(checker == "--STOP")
			m_StopNow = true;
		else if(checker == "--RUN")
			runner();
		else if(checker == "--DEBUG")
			spawnDebugger();
		else if(checker == "--CMDLIN")
		{
			char command[100];
			getFrom->getline(command,100);
			pAgent->ExecuteCommandLine(command, true);
		}
		else if(checker == "--NEWTHREAD")
		{
			initiateRes = true;
		}
		else if(checker == "--REMOTE")
		{
			initiateRem = true;
		}
		else if(checker == "--STEP")
			step();
	}
	if(checker != "--STEP" && checker != "--RESET" && checker != "--REMOTE" && checker != "--CMDLIN" && checker != "--DEBUG" && checker != "--RUN" && checker != "--SAVE" && checker != "--LOAD" && !loadPlease && checker != "--QUIT" && checker != "--STOP")
	{
		if(getFrom != cin)
		{
			wordNum--;
		}
		if(forMem != "")
			memory.push_back(forMem.substr(0, forMem.size() - 1));
		if(sentStore.size() >0)
			sml::IntElement* tmp3 = pAgent->CreateIntWME(sentStore[0], "length", wordNum); //add counter for num words
	}
	char garbage;
	if(checker != "--CMDLIN")
		getFrom->get(garbage);
	
}

bool
SoarTextIO::GetFileInput(string& word)
{
	if(!inFile)
		return false;
	while(true)
	{
		inFile >> word;
		if(word[0] != '#')  // not a comment
			break;
		getline(inFile, word);  // clears the line of comments
	}
	return true;
}

void
SoarTextIO::CloseFile()
{
	inFile.clear();
	inFile.close();
	cout << "You do not have a file open, or the file has ended." << endl;
	PrintNothing = true;
}

void
SoarTextIO::RespondCycle()
{
	top_level = "";
	int numberCommands2 = pAgent->GetNumberCommands();
	int numberCommands = pAgent->GetNumberOutputLinkChanges() ;
	for(int i = 0; i < numberCommands; i++)
	{
		if(pAgent->IsOutputLinkChangeAdd(i))
		{
			triple trip;
			sml::WMElement* tmp =pAgent->GetOutputLinkChange(i) ;
			trip.name = tmp->GetIdentifierName() ;
			trip.att = tmp->GetAttribute();
			trip.val = tmp->GetValueAsString();
			if(trip.att == "text")
				top_level = trip.val;
			trip.printed = false;
			storeO.push_back(trip);
			//cout << "(" << storeO[i].name << "   " << storeO[i].att << "   " << storeO[i].val << ")" << endl;
		}		
	}
	if(storeO.size()>0)
		PrintOutput();

	for(int i = 0; i< numberCommands2; i++)  //add's status complete
	{
		sml::Identifier* tmp2 = pAgent->GetCommand(i);
		tmp2->AddStatusComplete();
	}
	storeO.resize(0);
}

void
SoarTextIO::PrintOutput()
{
	string toPrint = "";
	string NextName = "";
	bool found = false;
	for(unsigned int i = 0; i < storeO.size() && !found ; i++)
	{
		if(storeO[i].att == "value")// && storeO[i].name == top_level)
		{
			toPrint += storeO[i].val;
			NextName = storeO[i].name;
			NextName = FindNextParent(NextName);
			found = true;
		}
	}
	bool wordLeft = true;
	while(wordLeft)
	{
		wordLeft = false;
		for(unsigned int j = 0; j < storeO.size(); j++)
		{
			if(storeO[j].name == NextName && storeO[j].att == "value")
			{
				toPrint += (" " + storeO[j].val);
				NextName = FindNextParent(NextName);
				wordLeft = true;
			}
		}		
	}	
	if(toPrint.size()>0)
	{
		printNow = false;
		CONSOLE_SCREEN_BUFFER_INFO info;
		HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
		if(!GetConsoleScreenBufferInfo( hStdout , &info ))
			cout << "GetInfo failed: " << GetLastError() << endl;
		COORD place;
		place.X = 0;
		place.Y = (info.dwCursorPosition.Y - 3);
		SetConsoleCursorPosition(hStdout,place);
		cout << ": ";
		cout << toPrint << endl << endl;
		char buffer[100];
		place.X = 0;
		place.Y = info.dwCursorPosition.Y;
		DWORD dummy;
		ReadConsoleOutputCharacter( hStdout , buffer , 100 , place , &dummy );
		string holder = buffer;
		cout << "                                                                              " << endl ;
		cout << "                                                                              " << endl ;
		cout << "                                                                              " << endl ;

		cout << GetRelevant( holder );
		
		printNow = true;
	}
	
}

string
SoarTextIO::FindNextParent(string name)
{
	for(unsigned int k = 0 ; k < storeO.size(); k++)
	{
		if(storeO[k].name == name && storeO[k].att == "next")
			return storeO[k].val;
	}
	return "error";
	
}

void
SoarTextIO::createSentId()
{
	if(LastSent.size() > 0)
	{
		pAgent->DestroyWME(LastSent[0]);
		LastSent.resize(0);
	}
	pTextInput = pAgent->CreateIdWME(pInputLink,"text");
	sentStore.push_back(pTextInput);
	NextWord.push_back(pTextInput);
	LastSent.push_back(pTextInput);
}

void
SoarTextIO::CreateWord()
{
	
	//Create word			
	sml::StringElement* tmp4 = pAgent->CreateStringWME(NextWord[NextWord.size()-1],"value",word.c_str());
	dontlose.push_back(tmp4);
	
	//Create next-word identifier
	sml::Identifier* tmp = pAgent->CreateIdWME(NextWord[NextWord.size()-1], "next");
	sentStore.push_back(tmp);
	NextWord.push_back(tmp);
}

void
SoarTextIO::saveMem()  
{
//	locFinder();
	cin >> loc;
	ofstream outFile;
	outFile.open(loc.c_str());

	if(!outFile)
	{
		cout << "***FILE FAILED TO OPEN***";
		WhenReady();
	}
	else
	{
		for(unsigned int i = 0; i < memory.size(); i++)
		{
			outFile << memory[i];
			outFile << endl;
		}
		//outFile << "&&&&" ; //used as delimeter for file
		cout << "***YOUR FILE HAS BEEN SAVED***";
		outFile.close();
	//	WhenReady();
	}	
}

void
SoarTextIO::loadMem()
{
	
	cin >> loc;
	inFile.close();
	inFile.clear();
	inFile.open(loc.c_str());

	if(!inFile)
		cout << "***FILE FAILED TO OPEN***";
	else
		PrintNothing = false;


}



void
SoarTextIO::ResetConnect()
{
	pKernel = sml::Kernel::CreateKernelInNewThread("SoarKernelSML");
	if (pKernel->HadError())
	{
		cout << pKernel->GetLastErrorDescription() << endl;
		return ;
	}
	pAgent = pKernel->CreateAgent("SoarTextIO");
	if (pKernel->HadError())
	{
		cout << pKernel->GetLastErrorDescription() << endl;
		return ;
	}
	spawnDebugger();
	init();
}

void
SoarTextIO::RemoteConnect()
{
	pKernel = sml::Kernel::CreateRemoteConnection(true,NULL,12121);
	while (pKernel->HadError())
	{
		cout << endl << pKernel->GetLastErrorDescription() << endl;
		cout << endl << "There is no kernel on port 12121 to connect to, please create a kernel." << endl;
		cout << "When you are ready, press any non white-space key + enter: ";
		string stuff;
		cin >> stuff;
		pKernel = sml::Kernel::CreateRemoteConnection( true , NULL , 12121 );
	}
	cout << endl << "A connection to the kernel has been made." << endl;
	if(pKernel->GetNumberAgents() == 1)
	{
		pAgent = pKernel->GetAgentByIndex(0);
		cout << "Your are connected to agent " << pAgent->GetAgentName() << endl;
	}
	else
	{
		cout << endl << "What is the name of the agent you are trying to connect to: ";
		string name;
		cin >> name;
		pAgent = pKernel->GetAgent(name.c_str());
		while(!pKernel->IsAgentValid(pAgent))
		{
			cout << "That agent name is invalid, please enter a new one: ";
			cin >> name;
			pAgent = pKernel->GetAgent(name.c_str());
		}
		if (pKernel->HadError())
		{
			cout << pKernel->GetLastErrorDescription() << endl;
			return ;
		}	
		char junk;
		cin.get(junk);
	}
	init();

	
}

void
SoarTextIO::KillKernel()
{
	pKernel->Shutdown();
	delete pKernel;
}

void
SoarTextIO::spawnRunner()
{
	// spawn the debugger asynchronously
	int ret = _spawnlp(_P_DETACH, "Runner.exe", "Runner.exe" , NULL);
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

void
SoarTextIO::spawnDebugger()
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

void
SoarTextIO::step()
{
	pKernel->RunAllAgents(1);
}

void
SoarTextIO::makeUpper(string & tosmall)
{
	for(unsigned int ii = 0; ii < tosmall.size(); ii++)
		tosmall[ii]=toupper(tosmall[ii]);
	return;
}

void
SoarTextIO::WhenReady()
{
	cout << endl << "Please make sure your productions are loaded before giving Soar input.";
/*	string hold;
	while(!_kbhit()){ Sleep(10); };
	Sleep(10);
	cin >> hold;*/
	cout << endl << endl;
}

int main()
{
	SoarTextIO STIO;
}

string
SoarTextIO::GetRelevant( string toShorten )
{
	int index = 0;
	int count = 0;
	while( index < 100 && count < 6 )
	{
		if( toShorten[index] == ' ' )
			count++;
		else
			count = 0;
		index++;
	}
	string toReturn = "";
	for(int i = 0; i < index - 5; i++)
	{
		toReturn += toShorten[i];
	}

	if( toReturn.size() > 2 )
		toReturn = toReturn.substr( 0 , toReturn.size() - 1 );

	return toReturn;
}

