#include "include/MineFSM.h"
#include "include/SoarGameGroup.h"
#include "general.h"

#define DISTANCE_EPSILON 25

MineFSM::MineFSM(OrtsInterface* oi, GroupManager* gm, GameObj* go) 
         : FSM(oi,gm,go) {
  name = OA_MINE;
}

int MineFSM::update() {
  SoarGameGroup* sgg;

  switch (state) {
    case IDLE:
      //cout << "MINEFSM: start!\n";
      if (gob->get_int("minerals") > 1) {
        state = MOVING_TO_BASE_ZONE;
        gob->set_action("move", moveToBaseZoneParams);
      }
      else {
        state = MOVING_TO_MINE_ZONE;
        gob->set_action("move", moveToMineZoneParams);
      }
      break;
    case MOVING_TO_MINE_ZONE:
      // once we get in range, look for a mineral
      //cout << "MINEFSM: mmz\n";
      if ((squaredDistance(*gob->sod.x, *gob->sod.y, mineZoneX, mineZoneY) 
          < DISTANCE_EPSILON) 
          or (*gob->sod.speed == 0)) { 
        sgg = groupMan->getGroupNear("mineral", worldId, 
                                     *gob->sod.x, *gob->sod.y);
        if (sgg != NULL) {
          // there is a mineral patch nearby, head right to a mineral
          mineralObj = sgg->getNextMember();
          mineralX = *mineralObj->gob->sod.x;
          mineralY = *mineralObj->gob->sod.y;
          mineralId = mineralObj->getID();
          
          tempParams.clear();
          tempParams.push_back(mineralX);
          tempParams.push_back(mineralY);
          gob->set_action("move", tempParams);
          state = MOVING_TO_MINERAL; 
          //cout << "MINEFSM: moving to mineral\n";
          
          if (*gob->sod.speed == 0) {
            return FSM_STUCK;
          }
        }  
        else if (*gob->sod.speed == 0) {
          // move ended, no mineral found, fail.
          return FSM_FAILURE;
       
        }
      } 
      // else keep moving
      break;
          
    case MOVING_TO_MINERAL:
      if (not ORTSIO->isAlive(mineralId)) {
        // minerals gone!
        //cout << "MINEFSM: minerals disappeared! looking for more" << endl;
        state = MOVING_TO_MINE_ZONE;
        gob->set_action("move", moveToMineZoneParams);
      }
      else if (ORTSIO->getOrtsDistance(mineralObj->gob, gob) <= 2) {
        // 2 is  defined in tool blueprint file for the distance needed to mine
        if (*gob->sod.speed > 0) {
          //cout << "MINEFSM: at the mineral, but speed > 0.\n";
        }
        else {
          tempParams.clear();
          tempParams.push_back(mineralId);
          gob->component("pickaxe")->set_action("mine", tempParams); 
          //cout << "MINEFSM: mining commencing!\n";
          state = MINING;
        }
      }
      else if (*gob->sod.speed > 0) {
        //cout << "MINEFSM: in motion to mineral.\n";
      }
      else {
        //cout << "MINEFSM: can't get to the mineral I chose. Trying again.\n";
        gob->set_action("move", moveToMineZoneParams);
        state = MOVING_TO_MINE_ZONE;
      } 
      break;
    case MINING:
      if (gob->component("pickaxe")->get_int("active") == 0 &&
          gob->get_int("is_mobile") == 1) 
      {
        // finished mining
        if (gob->get_int("minerals") == 0) {
          //cout << "MINEFSM: Mining failed for some reason! Trying again..\n";
          state = MOVING_TO_MINE_ZONE;
          gob->set_action("move", moveToMineZoneParams);
        }
        else {
          //cout << "MINEFSM: mining finished successfully" << endl;
          if (baseId != -1) {
            // go right to the base
            tempParams.clear();
            tempParams.push_back(baseX);
            tempParams.push_back(baseY);
            gob->set_action("move", tempParams);
            state = MOVING_TO_BASE;  
          }
          else {
            state = MOVING_TO_BASE_ZONE;
            gob->set_action("move", moveToBaseZoneParams);
          }
        }
      }
      else {
        //cout << "MINEFSM: still mining..\n";
      } 
      break;
    case MOVING_TO_BASE_ZONE:
      // once we get in range, look for a command center
      if ((squaredDistance(*gob->sod.x, *gob->sod.y, baseZoneX, baseZoneY) 
          < DISTANCE_EPSILON) or (*gob->sod.speed == 0)) { 
        sgg = groupMan->getGroupNear("controlCenter", myId, 
                                     *gob->sod.x, *gob->sod.y);
        if (sgg != NULL) {
          // there is a base nearby, head right to it 
          baseObj = sgg->getNextMember();
          baseX = *baseObj->gob->sod.x;
          baseY = *baseObj->gob->sod.y;
          baseId = baseObj->getID();

          tempParams.clear();
          tempParams.push_back(baseX);
          tempParams.push_back(baseY);
          gob->set_action("move", tempParams);
          state = MOVING_TO_BASE;  
          if (*gob->sod.speed == 0) {
            return FSM_STUCK;
          }
        }  
        else if (*gob->sod.speed == 0) {
          // move ended, no base found, fail.
          //cout << "MINEFSM: at the zone, no base.\n";
          return FSM_FAILURE;
        }
      } 
      // else keep moving
      break;
          
    case MOVING_TO_BASE:
      if (not ORTSIO->isAlive(baseId)) {
        // base gone!
        //cout << "MINEFSM: base disappeared! looking for more" << endl;
        state = MOVING_TO_BASE_ZONE;
        gob->set_action("move", moveToBaseZoneParams);
      }
      else if (ORTSIO->getOrtsDistance(baseObj->gob, gob) <= 3) {
        // 3 is defined in tool blueprint file for the distance needed
        
        tempParams.clear();
        tempParams.push_back(baseId);
        gob->set_action("return_resources", tempParams);
        state = SEND_MOVE_TO_MINE_COMMAND;
        
        //cout << "MINEFSM: resources returned!\n";
      }
      break;
    case SEND_MOVE_TO_MINE_COMMAND:
      // need an extra state here, since the action on the 
      // return is used up by return_resources
      if (mineralId != -1) {
        // go right back where you were mining
        tempParams.clear();
        tempParams.push_back(mineralX);
        tempParams.push_back(mineralY);
        gob->set_action("move", tempParams);
        state = MOVING_TO_MINERAL; 
        //cout << "MINEFSM: moving to mineral\n";
      }
      else {
        gob->set_action("move", moveToMineZoneParams);
        state = MOVING_TO_MINE_ZONE;
      }
      break;
  } // switch (state)

  return FSM_RUNNING;
}
          
void MineFSM::init(vector<signed long> p) {
  FSM::init(p);

  state = IDLE;

  assert (p.size() == 4);
  
  mineZoneX  = p[0];
  mineZoneY  = p[1];
  baseZoneX  = p[2];
  baseZoneY  = p[3];

  // set up the parameter vectors for later use
  moveToMineZoneParams.push_back(mineZoneX);
  moveToMineZoneParams.push_back(mineZoneY);
  moveToBaseZoneParams.push_back(baseZoneX);
  moveToBaseZoneParams.push_back(baseZoneY);

  worldId = ORTSIO->getWorldId();
  myId = ORTSIO->getMyId();
  mineralObj = (SoarGameObject*) NULL;
  baseObj = (SoarGameObject*) NULL;
  mineralId = -1;
  baseId = -1;
}
