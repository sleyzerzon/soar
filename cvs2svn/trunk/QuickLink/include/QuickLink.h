/*****************************************************
*  Taylor Lafrinere
*  
*  Soar Quick Link
*
*  QuickLink.h
*
*  The purpose of this application is to be able to 
*  control the input-link on soar from a command line
*  and to read its output to another window.
*
*  Start Date: 05.17.2005
*
*****************************************************/

#ifndef QUICKLINK_H	
#define QUICKLINK_H

#include <vector>
#include <fstream>
#include <string>
#include "sml_Client.h"
//#include "Reader.h"

using namespace std;

class QuickLink
{
	friend class Reader;
public:

	//******CONSTRUCTORS******

	QuickLink();
	//default constructor

	//******PUBLIC METHODS******

	void Run();
	//Runs the program

private:

	//******MEMBER STORAGE******

	vector<sml::Identifier*> IDs;
	vector<string> IDnames;
	vector<string> IDparent;
	vector<string> IDsoar;
	vector<bool> IDprint;
	vector<sml::StringElement*> SEs;
	vector<string> SEnames;
	vector<string> SEparent;
	vector<string> SEvalue;
	vector<bool> SEprint;
	vector<sml::IntElement*> IEs;
	vector<string> IEnames;
	vector<string> IEparent;
	vector<int> IEvalue;
	vector<bool> IEprint;
	vector<sml::FloatElement*> FEs;
	vector<string> FEnames;
	vector<string> FEparent;
	vector<float> FEvalue;
	vector<bool> FEprint;

	//******OUTPUT STORAGE******

	struct triple
	{
		string name;
		string att;
		string val;
		bool printed;
	};

	vector<triple> storeO;

	//*******SOAR-FORM OUTPUT SPACING TRICK******

	struct spaceControl
	{
		string indent;
		string iden;
	};

	vector<spaceControl> SC;

	//*******MULTIPLE LOAD FILE TRICK******

	struct pair
	{
		ifstream* str;
		string title;
	};

	vector<pair> loadPair;

	//******ALTERING PROCESS FILES******

	vector<string> commandStore;

	//*******MEMBER VARIABLES******

	sml::Kernel* pKernel;
	sml::Agent* pAgent;
	sml::Identifier* pInputLink;
	sml::Identifier* advHold;
	float Fvalue;
	int Ivalue;
	string command, title, garbage, parent, path, uniqid;
	string OldVal, NewVal, value, processExt, pCommand;
	string first, second, third, fourth, fifth, OS, loc, toStore;
	ifstream inFile;
	ofstream outFile;
	bool toClose, pOnce, printStep, askToSave, Icycle, printTree, loadingStep, userInput, StuffToSave;
	char endFind;
	int counter;

	


	
	//******MEMBER METHODS******

	void SoarSetup();
	//Launch Soar and Load Productions

	void InputCycle();
	//Create and Alter working memory

	void OutputCycle();
	//Capture and print Output from Soar

	void PrintWorkingMem();
	//Prints the Working Memory in tree form

	void WMrecurse(string father, string indent, bool flag);
	//Recursively prints the WM tree

	void makeUpper(string& tosmall);
	//change a string to all lower case letters

	void saveInput(bool toClose, ofstream& oFile);
	//saves input to a file

	void loadInput(ifstream& iFile);
	//loads input from a file

	void deleteWME();
	//deletes specified WME

	void deleteChilds(string parent);
	//deletes all children of deleted WME

    void delID(int ind);
	//deletes identifier

	void delIE(int ind);
	//deletes IntElelment

	void delSE(int ind);
	//deletes StringElement

	void delFE(int ind);
	//deletes FloatElement

	void createID();
	//creates identifier

	void createEL();
	//creates element

	bool displayTrips(string lookfor, string indent);
	//prints output nicely

	void advMode();
	//mode for advanced users

	void advValue();
    //figures out what type a value is

	void WhenReady();
	//asks user when ready

	void advAlter();
	//controls the alter function for advanced mode

	void advDelVal();
	//controls the delete function for advanced mode for values

	void advDelInd();
	//controls the delete function for advanced mode for Identifiers

	void clearAll();
	//clears input link structure

	void spawnDebug();
	//autostart debugger

	void loadProcess();
	//loads in old process

	void saveProcChanges();
	//saves Procedure Changes

	void promptToSave();
	//prompts user to save

	void endProcess();
	//ends a load early

	void printOutput();
	//prints output

	void printSoarOutForm();
	//prints output in soar form

	void printSoarInForm();
	//prints input in soar form

	void locFinder();
	//sets location for storage

	void OSFinder();

	void processLoader(ifstream & pFile);

	void CallParser(istream* in);
};


#endif