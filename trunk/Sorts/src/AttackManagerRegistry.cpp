#include "AttackManagerRegistry.h"

#include "SoarGameObject.h"
#include "PerceptualGroup.h"

int AttackManagerRegistry::assignManager
( const list<PerceptualGroup*>& targets )
{
  set<SoarGameObject*> allTargets;
  for(list<PerceptualGroup*>::const_iterator
      i =  targets.begin();
      i != targets.end();
      i++)
  {
    list<SoarGameObject*> members;
    (*i)->getMembers(members);
    for(list<SoarGameObject*>::iterator
        i =  members.begin();
        i != members.end();
        i++)
    {
      allTargets.insert(*i);
    }
  }

  for(list<ManagerRecord>::iterator
      i =  managers.begin();
      i != managers.end();
      i++)
  {
    // this check can take a while
    if (*(*i).manager->getTargets() == allTargets) { 
      return (*i).id;
    }
  }

  // create a new one
  ManagerRecord newRecord;
  AttackManager* m = new AttackManager(allTargets);
  newRecord.id = uid++;
  newRecord.manager = m;
  managers.push_back(newRecord);
  return newRecord.id;
}

AttackManager* AttackManagerRegistry::getManager(int id) {
  for(list<ManagerRecord>::iterator
      i =  managers.begin();
      i != managers.end();
      i++)
  {
    if ((*i).id == id) {
      return (*i).manager;
    }
  }
  return NULL;
}

void AttackManagerRegistry::removeManager(AttackManager* m) {
  for(list<ManagerRecord>::iterator
      i =  managers.begin();
      i != managers.end();
      i++)
  {
    if ((*i).manager == m) {
      managers.erase(i);
      return;
    }
  }
}
