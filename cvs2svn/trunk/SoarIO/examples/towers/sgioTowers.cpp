#include "Towers.h"

//standard directives
#include <cassert>
#include <iostream>
#include <string>
#include <stdlib.h>

using std::cout;
using std::cin;
using std::endl;
using std::string;

//SGIO directives
#include "sgio_wmemem.h"
#include "sgio_soar.h"
#include "sgio_siosoar.h"
#include "sgio_apisoar.h"

using sgio::Soar;
using sgio::SIOSoar;
using sgio::APISoar;
using sgio::Agent;
using sgio::WorkingMemory;
using sgio::SoarId;
using sgio::StringElement;
using sgio::IntElement;

//
#include "sgioTowersSoarAgent.h"
//using sgio_towers::SoarAgent;

//namespace sgio_towers
//{

/*************************************************************
* @brief	The TowerInputLinkProfile class contains all of the  
*			wmes corresponding to the Tower
*************************************************************/
class TowerInputLinkProfile
{
public:

	/*************************************************************
	* @returns	The Iwme* of the identifier that all tower child
	*			Iwmes hang off of
	*************************************************************/
	//IWme* GetTowerIdentifier() const { return m_pPegIdentifier;}

	/*************************************************************
	* @returns	The IWMObject* of the identifier that all tower
	*			child Iwmes use as a parent
	*************************************************************/
	//IWMObject* GetTowerIdentifierObject() const {return m_pPegIdentifier->GetValue()->GetObject();}

	/*************************************************************
	* @brief	Release reference to wmes actually owned by this
	*			class, set pointers to zero
	*************************************************************/

private:
	/*************************************************************
	* @brief	Initialize private data to zero.  This constructor
	*			can only be called by friends
	*************************************************************/
	TowerInputLinkProfile(WorkingMemory* inWorkingMemory, char inName) : pWorkingMemory(inWorkingMemory)
	{
		//m_pPegIdentifier = 0;
		string tempName;
		tempName += inName;

		m_pPegName = pWorkingMemory->CreateStringWME(pWorkingMemory->GetILink(), k_worldPegString, tempName);
	}

	~TowerInputLinkProfile()
	{
		//Release the top WME, all the rest go automatically
		pWorkingMemory->DestroyWME(m_pPegName);

		//this objectec doesn't own WM, so can't delete it
		pWorkingMemory = 0;
	}

	WorkingMemory* pWorkingMemory; //working memory is not owned by this object
	//SoarId* m_pPegIdentifier; //might be un necessary. may just have names hang off of ILink
	StringElement* m_pPegName;

	friend class Tower;
};

class DiskInputLinkProfile
{
public:

	~DiskInputLinkProfile()
	{
		pWMemory->DestroyWME(m_pHoldsIdentifier);
		m_pHoldsIdentifier	= 0;
			m_pDisk			= 0;
			m_pDiskBeneath	= 0;
	}

	void SetDiskBeneath(Disk* inDisk, Tower* inTower)
	{
		pActualDiskBeneath = inDisk;

		char buffer[33];
		currentPeg = itoa(inTower->GetSize(), buffer, 10);
		holdsNeedsToBeUpdated = true;
	}

	void Update()
	{
		if(!holdsNeedsToBeUpdated)
			return;

		pWMemory->Update(m_pPeg, currentPeg);
		pWMemory->Update(m_pDiskBeneath, pActualDiskBeneath->GetSize());
		pWMemory->Commit();
	}

private:

	DiskInputLinkProfile(WorkingMemory* inWMemory, Disk* inDisk, Disk* inDiskBeneath) : pWMemory(inWMemory), 
																						pActualDiskBeneath(inDiskBeneath)
	{
		if(inDiskBeneath)
			m_size = inDiskBeneath->GetSize();

		currentPeg = inDisk->GetTower()->GetName();
		
		m_pHoldsIdentifier	= pWMemory->CreateIdWME(pWMemory->GetILink(), k_holdsIdentifierString);
			m_pPeg			= pWMemory->CreateStringWME(m_pHoldsIdentifier, k_worldPegString, currentPeg);
			m_pDisk			= pWMemory->CreateIntWME(m_pHoldsIdentifier, k_diskIdentifierString, m_size);
			m_pDiskBeneath	= pWMemory->CreateIntWME(m_pHoldsIdentifier, k_holdsAboveString, inDiskBeneath->GetSize());

		holdsNeedsToBeUpdated = true;
	}

	WorkingMemory* pWMemory;

	SoarId* m_pHoldsIdentifier;
		StringElement* m_pPeg;		//name of peg that the disk is on
		IntElement* m_pDisk;		// size and name of disk
		IntElement* m_pDiskBeneath;	// size/name of disk beneath this on, if any

	int m_size;
	Disk* pActualDiskBeneath;
	string currentPeg;
	bool holdsNeedsToBeUpdated;

	friend class Disk;
};

/*************************************************************
* @brief	The IOManager class contains SGIO WorkingMemory
*			and Soar pointers, which it is responsible for 
*			cleaning up, and has wrapper functions for 
*			manipulating the input link
*************************************************************/
class IOManager
{
public:
	Agent* GetAgent() const
	{
		return pAgent;
	}

	WorkingMemory* GetWorkingMemory() const
	{
		return pWMemory;
	}
private:
	IOManager(Soar* inSoar) : pSoar(inSoar)
	{
		pAgent = pSoar->CreateAgent("sgioTowersAgent");
		assert(pAgent);

		//Load agent's productions
		if(! pAgent->LoadProductions("towers-of-hanoi-SGIO.soar"))
		{
			cout << "Program ending because productions failed to load. Press any key followed by 'enter' " << endl;
			string foo;
			cin >> foo;
			exit(1);
		}

		pWMemory = new WorkingMemory(pAgent);
		assert(pWMemory);
	}

	~IOManager()
	{
		delete pWMemory;
		delete pSoar;
	}

	Soar* pSoar;		//this object assumes ownership of Soar, and must clean it up
	Agent* pAgent;		//this object does not own the agent and does not clean it up
	WorkingMemory* pWMemory;	//this object assumes ownership of WM, and must clean it up

	friend class HanoiWorld;
};

//}//closes namespace


//using sgio_towers::DiskInputLinkProfile;

//======================================================
//============ Disk Function Definitions ===============

Disk::Disk(Tower* inTower, int inSize, Disk* inDiskBeneath) : pTower(inTower), m_size(inSize)
{
	m_iLinkProfile = new DiskInputLinkProfile(pTower->GetWorld()->GetIOManager()->GetWorkingMemory(), this, inDiskBeneath);
	assert(m_iLinkProfile);
}

void Disk::Detach()
{
	delete m_iLinkProfile;
}


void Disk::Update(Disk* diskBeneathMe, Tower* tower)
{
	assert(tower);
	pTower = tower;
	m_iLinkProfile->SetDiskBeneath(diskBeneathMe, tower);
	m_iLinkProfile->Update();
}

//======================================================
//============ Tower Function Definitions ==============

Tower::Tower(HanoiWorld* world, char name) : m_name(name)
{
	m_iLinkProfile = new TowerInputLinkProfile(world->GetIOManager()->GetWorkingMemory(), m_name);
	assert(m_iLinkProfile);
}

Tower::~Tower()
{
	for(diskItr_t diskItr = m_disks.begin(); diskItr != m_disks.end(); ++diskItr)
	{
		(*diskItr)->Detach();
		delete (*diskItr);
	}
	m_disks.clear();

	delete m_iLinkProfile;
	pWorld = 0;
}

//will always add a smaller disk than the top, so new disk must on at end of container
//disks that have just been created already have their disk beneath initialized, don't reset it
void Tower::AddDisk(Disk* newDisk, bool justCreated)
{
	assert(newDisk);
	if(!justCreated)
	{
		if(!m_disks.empty())
			newDisk->Update(m_disks.back(), this);
		else
			newDisk->Update(0, this);
	}

	m_disks.push_back(newDisk);
}

void Tower::RemoveTopDisk()
{
	if(m_disks.size() != 0)
		m_disks.erase(--m_disks.end());
}

Disk* Tower::GetTopDisk() const
{
	if(m_disks.size() != 0)
	{
		assert(m_disks.back());
		return m_disks.back();
	}
	return 0;
}


void Tower::PrintDiskAtRow(int row) const
{
	if(static_cast<int>(m_disks.size()) <= row)
		cout<<"--";
	else
		cout << m_disks[row]->GetSize();
}


//======================================================
//=========== Hanoi Function Definitions ===============

HanoiWorld::HanoiWorld(bool graphicsOn, int inNumTowers, int inNumDisks) : drawGraphics(graphicsOn)
{
	//create Soar and agent
	Soar* soar = 0;

	#ifdef SGIO_API_MODE
		soar = new APISoar();
	#elif defined SGIO_TSI_MODE
		soar = new SIOSoar("127.0.0.1", 6969, true);
	#endif
	assert(soar);

	ioManager = new IOManager(soar);
	assert(ioManager);

	//wrap up the Soar agent
	m_agent = new SoarAgent(ioManager->GetAgent(), ioManager->GetWorkingMemory(), this);
	assert(m_agent);

	//create Towers

	//Name each tower and store for later
	for(int towerNum = 0; towerNum < inNumTowers; ++towerNum)
	{
		//==============
		//"Left" tower
		//==============
		if(towerNum == 0)
		{
			Tower* tower = new Tower(this, 'A');
			assert(tower);

			//Create disks
			for(int currentDiskSize = maxNumDisks; currentDiskSize > 0; --currentDiskSize)
			{
				//The disk currently at the front of the container is the "bottom" disk.  New, smaller, disks
				//are inserted in back

				Disk* towerTopDisk = 0;

				if(tower->GetSize() != 0)
					towerTopDisk = tower->GetTopDisk();

				Disk* disk = new Disk(tower, currentDiskSize, towerTopDisk);
				assert(disk);

				tower->AddDisk(disk, true);
			}

			m_towers.push_back(tower);
		}
		//==============
		//Middle tower
		//==============
		else if(towerNum == 1)
		{
			Tower* tower = new Tower(this, 'B');
			assert(tower);
			m_towers.push_back(tower);
		}

		//===============
		//"Right" tower
		//===============
		else
		{
			Tower* tower = new Tower(this, 'C');
			assert(tower);
			m_towers.push_back(tower);
		}
	}//end-for-loop

}

HanoiWorld::~HanoiWorld()
{
	for(towerItr_t towerItr = m_towers.begin(); towerItr != m_towers.end(); ++towerItr)
		delete *towerItr;

	m_towers.clear();

	delete m_agent;
	m_agent = 0;
	delete ioManager; //This will clean up Soar and working memory
}

void HanoiWorld::Run()
{
	m_agent->MakeMove();
}

//remove from the source tower, add to the destination tower
bool HanoiWorld::MoveDisk(int sourceTower, int destinationTower)
{
	Disk* movingDisk = m_towers[sourceTower]->GetTopDisk();
	if(!movingDisk)
		return false;

	assert(movingDisk);
	m_towers[sourceTower]->RemoveTopDisk();

	m_towers[destinationTower]->AddDisk(movingDisk, false);
	return true;
}

void HanoiWorld::Print()
{
	for(int row = maxNumDisks - 1; row >= 0; --row)
	{
		for(unsigned int towerCounter = 0; towerCounter < m_towers.size(); ++towerCounter)
		{
			cout << "(";
			m_towers[towerCounter]->PrintDiskAtRow(row);
			cout << ")    ";
		}
		cout << endl;
	}
	cout<<"======================" << endl << endl;
}

bool HanoiWorld::AtGoalState()
{
	if(m_towers[2]->GetSize() == maxNumDisks)
		return true;
	return false;
}

void  HanoiWorld::EndGameAction()
{
	//Run();
//#ifdef GSKI_DAG_TOWERS_USE_TIMER
//	cout.precision(6);
//	cout <<  "Total run time: " <<  gSKITowersDAGTimer / static_cast<double>(CLK_TCK) << endl;
//#endif
}

