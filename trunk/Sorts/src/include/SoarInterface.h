#ifndef soarinterface_h
#define soarinterface_h

#include<utils>
#include<list>
#include<map>
#include<hash_map>
#include<pthread.h>

#include "sml_Client.h"

using namespace std;

typedef list<pair<string, int> > groupPropertyList;

typedef struct {
  int groupId;
  bool added;
  Identifier* WMEptr;
  // change this later into hash_map<string, IntElement*> and write a hash
  // function for strings
  map<string, IntElement*> properties;
} SoarIOGroupRep;

class SoarInterface{
  public:
    SoarInterface(pthread_mutex_t* _actionQueueMutex);
    ~SoarInterface();
    void getNewActions(list<SoarAction*> &actions);
    void addGroup(SoarGameGroup* group);
    void removeGroup(SoarGameGroup* group);
    void refreshGroup(SoarGameGroup* group, groupPropertyList& gpl);
    
  private:
    int groupIdCounter;
    sml::Agent *agent;
    sml::Identifier *inputLink;

    pthread_mutex_t* actionQueueMutex;

    // these are the maps that keep track of input link - middleware objects
    
    hash_map<SoarGameGroup*, SoarIOGroupRep> mwToSoarGroups;
    hash_map<int, SoarGameGroup*>            gIdToMwGroups;
    
    hash_map<sml::Identifier*, SoarAction>  soarActions;
    // a list of actions that are currently unprocessed
    // there is a race condition on accessing this list
    list<SoarAction*> actionQueue;
};


/* The GroupManager will have a pointer to this structure, and can call the public functions to get new actions and change what groups Soar can "see". Whatever needs to be done to initialize Soar or do higher level things like pause for debugging will be done by the main loop, which can call other (not yet defined) functions inside the SoarInterface.

SoarInterface will be responsible for creating and organizing the WMEs for the groups, but not for determining what information is in the structure (for example, we don't want a "health" WME for a tree, but SoarInterface shouldn't need to figure that out). There will be a list of visible properties inside the SoarGameGroups that will be set by the GroupManager and read by the SoarInterface to determine that.

Note: addGroup should not actually add the group to the input_link until that group is refreshed! Initially, the stats will not be set. */
#endif
