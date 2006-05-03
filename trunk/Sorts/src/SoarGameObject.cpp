#include<iostream>
#include<assert.h>
#include<string>

#include "SPathFinder.h"
#include "MineFSM.h"
#include "MoveFSM.h"

#include "Sorts.h"
#include "SoarGameObject.h"
#include "PerceptualGroup.h"
#include "InternalGroup.h"

void SoarGameObject::identifyBehaviors() {
  string name = gob->bp_name();
  if (friendly && name == "worker") {
    FSM* mineBehavior = new MineFSM(sorts, gob);
    registerBehavior(mineBehavior);
    FSM* moveBehavior = new MoveFSM(sorts, gob);
    registerBehavior(moveBehavior);
  }
}

SoarGameObject::SoarGameObject(
  GameObj*       g, 
  const Sorts*   _sorts,
  bool           _friendly, 
  bool           _world, 
  int            _id )
: gob(g), sorts(_sorts), 
  friendly(_friendly), world(_world), id(_id)
{
  status = OBJ_IDLE;
  frameOfLastUpdate = -1;
  identifyBehaviors();
}

SoarGameObject::~SoarGameObject()
{
  while(!memory.empty()) {
    memory.pop();
  }

  for(map<ObjectActionType, FSM*>::iterator i = behaviors.begin(); 
      i != behaviors.end(); i++) 
  {
    delete i->second;
  }
}

void SoarGameObject::registerBehavior(FSM *b)
{
  assert (behaviors.find(b->getName()) == behaviors.end());
  behaviors[b->getName()] = b;
}


void SoarGameObject::removeBehavior(ObjectActionType name)
{
  map<ObjectActionType, FSM*>::iterator i = behaviors.find(name);
  if (i != behaviors.end()) {
    delete i->second;
    behaviors.erase(i);
  }
}


//template<class T>
void SoarGameObject::issueCommand(ObjectActionType cmd, Vector<sint4> prms)
{
  cout << "initting: " << (int)this << endl;
  //Whether we really want this is up for analysis
  while(!memory.empty())
    memory.pop();

  map<ObjectActionType, FSM*>::iterator i = behaviors.find(cmd);
  assert(i != behaviors.end());

  i->second->init(prms);
  memory.push(i->second);
  update();
}


void SoarGameObject::update()
{
  cout << "upd: " << (int)this << " grp " << (int)pGroup << endl;
  int fsmStatus;
  iGroup->setHasStaleMembers();
  pGroup->setHasStaleMembers();
  
  int currentFrame = sorts->OrtsIO->getViewFrame();
  if (currentFrame == frameOfLastUpdate) {
    cout << "ignoring repeated update.\n";
    sorts->OrtsIO->updateNextCycle(this);
    return;
  }
  frameOfLastUpdate = currentFrame;
  
  if(!memory.empty())
  {
    cout << "fsm: " << memory.top()->getName() << endl;
    fsmStatus = memory.top()->update();
    // if we get a done status, remove the action
    // stuck result means that the FSM is hung up, but may continue
    // if things get out of the way (don't give up on it)
    if((fsmStatus != FSM_RUNNING) 
       && (fsmStatus != FSM_STUCK)) {
      cout << "pop!\n";
      memory.pop();
      cout << "memsize: " << memory.size() << endl;
    //  if(memory.empty())
        //currentCommand = SA_IDLE;
    }

    if (fsmStatus == FSM_SUCCESS) {
      status = OBJ_SUCCESS;
    }
    else if (fsmStatus == FSM_FAILURE) {
      status = OBJ_FAILURE;
    }
    else if (fsmStatus == FSM_STUCK) {
      status = OBJ_STUCK;
    }
    else {
      status = OBJ_RUNNING;
    }
    sorts->OrtsIO->updateNextCycle(this);
  }
  else {
    cout << "empty memory\n";
  }
}


void SoarGameObject::setPerceptualGroup(PerceptualGroup *g)
{
 pGroup = g;
}

PerceptualGroup *SoarGameObject::getPerceptualGroup(void)
{
 return pGroup;
}

void SoarGameObject::setInternalGroup(InternalGroup *g)
{
 iGroup = g;
}

InternalGroup *SoarGameObject::getInternalGroup(void)
{
 return iGroup;
}
/*ObjectActionType SoarGameObject::getCurrentCommand()
{
 return currentCommand;
}*/

int SoarGameObject::getStatus()
{
  return status;
}

Rectangle SoarGameObject::getBoundingBox() {
  Rectangle r;
  r.xmin = *gob->sod.x - *gob->sod.radius;
  r.xmax = *gob->sod.x + *gob->sod.radius;
  r.ymin = *gob->sod.y - *gob->sod.radius;
  r.ymax = *gob->sod.y + *gob->sod.radius;
  return r;
}
