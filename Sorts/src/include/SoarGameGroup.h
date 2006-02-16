#ifndef SoarGameGroup_h
#define SoarGameGroup_h

#include <list>
#include <set>
#include <string>
#include "SoarAction.h"
#include "OrtsInterface.h"
#include "general.h"

#ifdef DEBUG_GROUPS
#include "FakeSoarGameObject.h"
#else
#include "SoarGameObject.h"
#endif

using namespace std;

//class SoarGameObject{}; // TEMPORARY

class SoarGameGroup {
  public:
    SoarGameGroup(SoarGameObject* unit, OrtsInterface* in_ORTSIO);
    void addUnit(SoarGameObject* unit);
    bool removeUnit(SoarGameObject* unit);
    void updateStats(bool saveProps);
    bool assignAction(SoarActionType type, list<int> params,
                      list<SoarGameGroup*> targets);
    bool isEmpty();

    list<SoarGameObject*> getMembers(); 
  
    void mergeTo(SoarGameGroup* target);
    bool getStale();
    void setStale();
    void setStale(bool val);
    bool getStaleInSoar();
    void setStaleInSoar(bool val);
    groupPropertyStruct getSoarData();
    //void setType(int inType);
    pair<string, int> getCategory();
    int getSize();
    SoarGameObject* getCenterMember();
    SoarGameObject* getNextMember();

    // get the player number that owns this group
    int getOwner();
    bool isWorld();
    bool isFriendly();

    bool getSticky();
    void setSticky(bool in);
  private:
    set <SoarGameObject*> members;
    groupPropertyStruct soarData;
    bool stale;
    bool staleInSoar;
    // stale means the internal statistics haven't been updated-
    // something moved, or new members were added.

    // staleInSoar means that internal statistics are correct, but Soar
    // needs a refresh-- either refresh it or remove it from Soar ASAP

    string typeName;
    
    SoarGameObject* centerMember;
    SoarGameObject* currentMember; // for getNextMember functionality

    int owner;
    bool friendly;
    bool world;
    OrtsInterface* ORTSIO;
    // sticky if grouped by Soar-
    // issuing a command makes it auto-sticky until command ends & Soar acks
    bool sticky;

    // info on the last command executed- what is it, and is it done?
    string currentCommand;
    int commandStatus;
};

#define GRP_STATUS_RUNNING 0
#define GRP_STATUS_SUCCESS 1
#define GRP_STATUS_FAILURE 2
#define GRP_STATUS_IDLE 3
#endif
