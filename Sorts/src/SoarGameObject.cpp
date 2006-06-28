#include<iostream>
#include<assert.h>
#include<string>

#include "MineFSM.h"
#include "PersistentMoveFSM.h"
#include "IdleFSM.h"
#include "AttackFSM.h"
#include "AttackNearFSM.h"
#include "BuildFSM.h"
#include "TrainFSM.h"

#include "Sorts.h"
#include "SoarGameObject.h"
#include "PerceptualGroup.h"

#define msg cout << Sorts::frame << " SGO(" << this << "): "
#define PANIC_FRAMES 30

using namespace std;

void SoarGameObject::identifyBehaviors() {
  name = gob->bp_name();
  FSM* idleBehavior = new IdleFSM(gob);
  registerBehavior(idleBehavior);
  if (friendly) {
    if (name == "worker") {
      FSM* mineBehavior = new MineFSM(gob);
      ((MineFSM*)mineBehavior)->setSoarGameObject(this);
      registerBehavior(mineBehavior);
      FSM* moveBehavior = new PersistentMoveFSM(gob);
      registerBehavior(moveBehavior);
      FSM* buildBehavior = new BuildFSM(gob);
      ((BuildFSM*)buildBehavior)->setSoarGameObject(this);
      registerBehavior(buildBehavior);
      FSM* attackBehavior = new AttackFSM(this);
      registerBehavior(attackBehavior);
      friendlyWorker = true;
    }
    else if (name == "marine") {
      FSM* moveBehavior = new PersistentMoveFSM(gob);
      registerBehavior(moveBehavior);
      FSM* attackBehavior = new AttackFSM(this);
      registerBehavior(attackBehavior);
      FSM* attackNear = new AttackNearFSM(this);
      registerBehavior(attackNear);
      defaultBehaviors.push_back(attackNear);
     }
    else if (name == "tank") {
      FSM* moveBehavior = new PersistentMoveFSM(gob);
      registerBehavior(moveBehavior);
      FSM* attackBehavior = new AttackFSM(this);
      registerBehavior(attackBehavior);
      FSM* attackNear = new AttackNearFSM(this);
      registerBehavior(attackNear);
      defaultBehaviors.push_back(attackNear);
    }
    else if (name == "controlCenter") {
      FSM* trainBehavior = new TrainFSM(gob);
      registerBehavior(trainBehavior);
    }
    else if (name == "barracks") {
      FSM* trainBehavior = new TrainFSM(gob);
      registerBehavior(trainBehavior);
    }
    else if (name == "factory") { 
      FSM* trainBehavior = new TrainFSM(gob);
      registerBehavior(trainBehavior);
    }
  }
}

SoarGameObject::SoarGameObject(
  GameObj*       g, 
  bool           _friendly, 
  bool           _world, 
  int            _id )
: gob(g), 
  friendly(_friendly), world(_world), id(_id)
{
  status = OBJ_IDLE;
  frameOfLastUpdate = -1;
  lastLocation = getLocation();
  friendlyWorker = false; // determined in identifyBehaviors
  motionlessFrames = 0;
  
  identifyBehaviors();
  pGroup = NULL;
  lastAttackedId = -1;
  assignedBehavior = NULL;
  
  sat_loc = Sorts::spatialDB->addObject(gob);
}

SoarGameObject::~SoarGameObject()
{
  removeFromGame();
}

void SoarGameObject::removeFromGame() {
  for(map<ObjectActionType, FSM*>::iterator 
      i  = behaviors.begin(); 
      i != behaviors.end(); 
      ++i) 
  {
    delete i->second;
  }
  Sorts::spatialDB->removeObject(gob,sat_loc);
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


void SoarGameObject::assignAction(ObjectActionType cmd, Vector<sint4> prms)
{
  msg << "assigning action: " << (int)cmd << endl;

  if (assignedBehavior != NULL) {
    assignedBehavior->stop();
  }
  
  if (cmd == OA_STOP) {
    assignedBehavior = NULL;
  }
  else {
    map<ObjectActionType, FSM*>::iterator i = behaviors.find(cmd);
    assert(i != behaviors.end());
    i->second->init(prms);
    assignedBehavior = i->second;
  }
  motionlessFrames = 0;
  update();
}


void SoarGameObject::update()
{
  msg << "updating sgo " << this << " in group " << pGroup << endl;
  if (gob->is_pending_action()) {
    msg << "ignored update, there is a pending action.\n";
    Sorts::OrtsIO->updateNextCycle(this);
    return;
  }
  
  int fsmStatus;

  sat_loc = Sorts::spatialDB->updateObject(gob,sat_loc);
    
  pGroup->setHasStaleMembers();
  
  int currentFrame = Sorts::OrtsIO->getViewFrame();
  if (currentFrame == frameOfLastUpdate) {
    msg << "ignoring repeated update.\n";
    Sorts::OrtsIO->updateNextCycle(this);
    return;
  }

  // first carry out assigned behavior
  if (assignedBehavior != NULL) {
    fsmStatus = assignedBehavior->update();

    // if we get a done status, remove the action
    // stuck result means that the FSM is hung up, but may continue
    // if things get out of the way (don't give up on it)
    if((fsmStatus != FSM_RUNNING) && (fsmStatus != FSM_STUCK)) {
      assignedBehavior = NULL;
    }

    if (fsmStatus == FSM_SUCCESS) {
      status = OBJ_SUCCESS;
    }
    else if (fsmStatus == FSM_FAILURE) {
      status = OBJ_FAILURE;
      msg << "got obj_failure\n";
    }
    else if (fsmStatus == FSM_STUCK) {
      status = OBJ_STUCK;
    }
    else {
      status = OBJ_RUNNING;
    }
    Sorts::OrtsIO->updateNextCycle(this);
  }
  else {
    //status = OBJ_IDLE;
    // don't set status to idle.. we need the last FSM status to stick
    // around when commands end
  }

  // do default behaviors (if action has not yet been assigned)
  for(list<FSM*>::iterator
      i  = defaultBehaviors.begin();
      i != defaultBehaviors.end();
      ++i)
  {
    if (gob->is_pending_action()) {
      break;
    }
    (*i)->update(); // don't care about the status for now
  }

  // spit out a warning if the object sits still for a long time
  if (friendlyWorker and assignedBehavior != NULL) {
    if (getLocation() == lastLocation) {
      motionlessFrames++;
    }
    else {
      motionlessFrames = 0;
    }
    if (motionlessFrames > PANIC_FRAMES) {
      msg << "worker has been still for " 
          << motionlessFrames << " frames, time to panic.\n";
      assignedBehavior->panic();
      motionlessFrames = 0;
    }
    lastLocation = getLocation();
  }
  frameOfLastUpdate = currentFrame;

  if (lastAttackedId >= 0) {
    ScriptObj* weapon = gob->component("weapon");
    assert(weapon != NULL); 
    // lastAttackedId should never be set if the object has no weapon
    if (weapon->get_int("shooting") == 0) {
      lastAttackedId = -1;
    }
  }
}


void SoarGameObject::setPerceptualGroup(PerceptualGroup *g) {
  pGroup = g;
}

PerceptualGroup *SoarGameObject::getPerceptualGroup(void)
{
 return pGroup;
}


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

coordinate SoarGameObject::getLocation() {
 // if this gets used a lot, we should make the coord a member
  coordinate c;
  c.x = *gob->sod.x;
  c.y = *gob->sod.y;

  return c;
}

void SoarGameObject::endCommand() {
  if (assignedBehavior != NULL) {
    assignedBehavior->stop();
  }
  assignedBehavior = NULL;
  status = OBJ_IDLE;
}
