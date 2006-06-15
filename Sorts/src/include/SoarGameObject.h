#ifndef SoarGameObject_H
#define SoarGameObject_H

#include"FSM.h"
#include "Rectangle.h"

#include<stack>
#include<map>

using namespace std;

class PerceptualGroup;
class InternalGroup;

class SoarGameObject{
 public:
	SoarGameObject 
  ( GameObj*       _gob,
    bool           _friendly, 
    bool           _world, 
    int            _id );

	~SoarGameObject();

	void identifyBehaviors();

	void registerBehavior(FSM *);
	void removeBehavior(ObjectActionType);

	//template<class T>
	void issueCommand(ObjectActionType name, Vector<sint4> p);
  void endCommand();
	void update();

	void setPerceptualGroup(PerceptualGroup *g);
	PerceptualGroup *getPerceptualGroup();
	//void setInternalGroup(InternalGroup *g);
	//InternalGroup *getInternalGroup();
	int getStatus();
  void removeFromGame();

  int  getOwner()     { return *gob->sod.owner; }
 	bool isFriendly()   { return friendly; }
  bool isWorld()      { return world; }
  int getID()         { return id; }
  coordinate getLocation();

  Rectangle getBoundingBox();

  GameObj* getGob()   { return gob; }

  void setLastAttacked(int id) { lastAttackedId = id; }
  int  getLastAttacked()       { return lastAttackedId; }
  string getName() { return name; }

private:
  GameObj* gob;

  sint4 sat_loc;

	map<ObjectActionType, FSM *> behaviors;
  FSM* assignedBehavior;
  list<FSM*> defaultBehaviors;

  ObjectActionType currentCommand;
	PerceptualGroup* pGroup;
	//InternalGroup* iGroup;
	bool friendly;
  bool world;
  int id;

  int frameOfLastUpdate;

  int status;
  string name;

  bool friendlyWorker;
  // these are only updated for friendly workers!
  coordinate lastLocation;
  int motionlessFrames;

  int lastAttackedId;
};

#define OBJ_IDLE 0
#define OBJ_RUNNING 1
#define OBJ_SUCCESS 2
#define OBJ_FAILURE 3
#define OBJ_STUCK 4

#endif
