#include "RLDeltaUpdater.h"

#include <iostream>
#include <set>
#include <pthread.h>
#include <unistd.h>

#include "Sorts.h"

using std::set;
using std::cout;
using std::endl;

void updateWatches
( smlRunEventId id,
  void*         userData,
  Agent*        agent,
  smlPhase      phase ) 
{
  // if Soar spins too fast, the discount factor will drive rewards to
  // near 0.
  usleep(300000);
  pthread_mutex_lock(Sorts::mutex);

  if (agent->GetOutputLink() == NULL) {
    pthread_mutex_unlock(Sorts::mutex);
    return; 
  }
  WMElement* wrWME = agent->GetOutputLink()->FindByAttribute("watch", 0);
  if (!wrWME || !wrWME->IsIdentifier()) {
    pthread_mutex_unlock(Sorts::mutex);
    return;
  }
  Identifier* watchReqs = (Identifier*) wrWME;

  RLDeltaUpdater* updater = (RLDeltaUpdater*) userData;
  
  for(int i = 0; i < watchReqs->GetNumberChildren(); ++i) {
    Identifier* item = watchReqs->GetChild(i)->ConvertToIdentifier();
    
    if (updater->watches.find(item) == updater->watches.end()) {
      WMElement* idWME = item->FindByAttribute("id", 0);
      const char* name = item->GetParameterValue("name");
      const char* attribute = item->GetParameterValue("attribute");
      assert(idWME && idWME->IsIdentifier());
      assert(name);
      assert(attribute);
      Identifier* id = idWME->ConvertToIdentifier();

      // new watch, the delta is set to 0 by default
      const char* strVal = id->GetParameterValue(attribute);
      assert(strVal);
      FloatElement *deltaWME = agent->CreateFloatWME(updater->watchInfo, name, 0.0);
      WatchData newWatch(name, id, attribute, deltaWME, atof(strVal));
      updater->watches[item] = newWatch;
      item->AddStatusComplete();
    }
  }

  for(map<Identifier*, WatchData>::iterator
      i  = updater->watches.begin();
      i != updater->watches.end();
      ++i)
  {
    WatchData& watch = i->second;
    const char* strVal = watch.id->GetParameterValue(watch.attribute.c_str());
    assert(strVal);
    double currVal = atof(strVal);
    cout << "rlupdate: old " << watch.lastVal << " new: " << currVal 
         << " diff: " << currVal - watch.lastVal << endl;
    agent->Update(watch.delta, currVal - watch.lastVal);
    watch.lastVal = currVal;
  }

  agent->Commit();

  pthread_mutex_unlock(Sorts::mutex);
}

RLDeltaUpdater::RLDeltaUpdater(Agent* agent) {
  watchInfo = agent->CreateIdWME(agent->GetInputLink(), "watch");
  //watchReqs = agent->CreateIdWME(agent->GetOutputLink(), "watch");
  agent->Commit();
  agent->RegisterForRunEvent(smlEVENT_BEFORE_DECISION_CYCLE, updateWatches, this);
}


