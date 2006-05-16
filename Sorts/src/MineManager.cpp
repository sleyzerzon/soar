#include "MineManager.h"
#include "Sorts.h"
#include "MineFSM.h"

/*
   MineManager.cpp
   Sorts project
   Sam Wintermute, 2006
*/

MineManager::MineManager() {
}

MineManager::~MineManager() {
  for (set<MiningRoute*>::iterator it=routes.begin();
       it != routes.end();
       it++) { delete *it; }
  for (list<StationInfo*>::iterator it=allStations.begin();
       it != allStations.end();
       it++) { delete *it; }
  for (list<MiningRoute*>::iterator it=invalidRoutes.begin();
       it != invalidRoutes.end();
       it++) { delete *it; }
  for (list<CCenterInfo*>::iterator it=cCenters.begin();
       it != cCenters.end();
       it++) { delete *it; }
  for (set<MineralInfo*>::iterator it=minerals.begin();
       it != minerals.end();
       it++) { delete *it; }
}

void MineManager::prepareRoutes(list<SoarGameObject*>& miners) {
  list<MiningRoute*> bestRoutes;
  int numAssignments = miners.size();
  MiningRoute* best;

  cout << "MM: preparing " << numAssignments << " routes.\n";

  // if this fails, not every route that was prepared last time was requested 
  // by the FSM.
  assert(assignments.size() == 0);
  
  // true if the corresponding miner has a route
  vector<bool> assigned;

  for (int i=0; i<numAssignments; i++) {
    best = getBestRoute();
    assert (best->stage == PF_DIST); 
    bestRoutes.push_back(best);
    addCostToRoute(best);
    
    assigned.push_back(false);
  }
  double minDistance;
  int minMinerIdx;
  SoarGameObject* minMinerPtr;
  double currentDistance;

  list<SoarGameObject*>::iterator minerIt;
  for (list<MiningRoute*>::iterator it = bestRoutes.begin();
       it != bestRoutes.end();
       it++) {
    minDistance = 99999999;
    minMinerIdx = -1;
    minMinerPtr = NULL;
    minerIt = miners.begin();
    for (int i=0; i<numAssignments; i++) {
      if (not assigned[i]) {
        currentDistance = coordDistanceSq((*it)->miningLoc, 
                                          (*minerIt)->getLocation()); 
        if (currentDistance < minDistance) {
          minMinerIdx = i;
          minMinerPtr = *minerIt;
          minDistance = currentDistance;
        }
      }
      minerIt++;
    }
    assert(minerIt == miners.end());
    // we now have the closest miner to the given route.
    assigned[minMinerIdx] = true;
    assignments[minMinerPtr] = *it;
  }
  
}
MiningRoute* MineManager::getMiningRoute(MineFSM* fsm) {
  map<SoarGameObject*, MiningRoute*>::iterator route 
      = assignments.find(fsm->getSoarGameObject());
  assert(route != assignments.end());
  MiningRoute* ret = (*route).second;
  assignments.erase(route);
  addFSMToRoute(ret, fsm);
  return ret;
}
MiningRoute* MineManager::reportMiningResults(int time, MiningRoute* route, 
                                              bool atBase, MineFSM* fsm) {
  return NULL;
}
void MineManager::addMineral(SoarGameObject* mineral) {
  MineralInfo* mi = new MineralInfo;
  mi->mineral = mineral;
  mi->stationsValid[0] = false;
  mi->stationsValid[1] = false;
  mi->stationsValid[2] = false;
  mi->stationsValid[3] = false;
  
  for (list<CCenterInfo*>::iterator it = cCenters.begin();
       it != cCenters.end();
       it++) {
    addRoute(*it, mi);
  }
    
  minerals.insert(mi);
}
void MineManager::addControlCenter(SoarGameObject* center) {
  CCenterInfo* cci = new CCenterInfo;
  cci->cCenter = center;
  cci->stationsValid[0] = false;
  cci->stationsValid[1] = false;
  cci->stationsValid[2] = false;
  cci->stationsValid[3] = false;
  
//  list<MineralInfo*> mis;
  for (set<MineralInfo*>::iterator it = minerals.begin();
       it != minerals.end();
       it++) {
    // can't iterate and modify at the same time..
    addRoute(cci, *it);
    //mis.push_back(*it);
  }/*
  for (list<MineralInfo*>::iterator it = mis.begin();
       it != mis.end();
       it++) {
    addRoute(&cci, *it);
  }*/
  cCenters.push_back(cci);
}

void MineManager::removeMineral(SoarGameObject* mineral) {
  // if anyone is mining this mineral, we need to interrupt them
  // and re-route

  MineralInfo* dummy;
  dummy->mineral = mineral;

  SoarGameObject* sgo;
  
  set<MineralInfo*>::iterator it = minerals.find(dummy);
  assert(it != minerals.end());
  MineralInfo* mi = *it;
 
  list <SoarGameObject*> miners; // need to prepare routes for miners
  list<MiningRoute*>::iterator it2;
  list<MineFSM*>::iterator it3;
  list<MineFSM*> fsmsToAbort;
  
  for(it2 = mi->routes.begin();
      it2 != mi->routes.end();
      it2++) {
    if ((*it2)->valid) {
      for (it3 = (*it2)->fsms.begin();
          it3 != (*it2)->fsms.end();
          it3++) {
        sgo = (*it3)->getSoarGameObject();
        miners.push_back(sgo);
        fsmsToAbort.push_back(*it3);

        // this frees the resources (dropoff point matters, in this case)
        // and deletes the MineFSM from the route
        removeFromRoute(*it2, it3);
      }
      (*it2)->valid = false;
      invalidRoutes.push_back(*it2);
      // route will still appear at the other end
      routes.erase(*it2);
    } 
  }
  // delete the mineral
  minerals.erase(it);
  // don't worry about de-allocating the stations, that is all done in the
  // allStations list.. 
  prepareRoutes(miners);
  
  for (it3 = fsmsToAbort.begin();
       it3 != fsmsToAbort.end();
       it3++) {
    (*it3)->abortMining();
    // FSM will request the new route calculated above
  }
}    

void MineManager::removeControlCenter(SoarGameObject* center) {
  // if anyone is using this, we need to interrupt them
  list<SoarGameObject*> miners; // need to prepare routes for miners
  list<MineFSM*> fsmsToAbort;
  list<MineFSM*>::iterator it3;
  SoarGameObject* sgo;
  
  for (list<CCenterInfo*>::iterator it = cCenters.begin();
       it != cCenters.end();
       it++) {
    if ((*it)->cCenter == center) {
      for(list<MiningRoute*>::iterator it2 = (*it)->routes.begin();
          it2 != (*it)->routes.end();
          it2++) {
        if ((*it2)->valid) {
          for (it3 = (*it2)->fsms.begin();
              it3 != (*it2)->fsms.end();
              it3++) {
            sgo = (*it3)->getSoarGameObject();
            miners.push_back(sgo);
            fsmsToAbort.push_back(*it3);
            // this frees the resources (dropoff point matters, in this case)
            // and deletes the MineFSM from the route
            removeFromRoute(*it2, it3);
          }
          // remove the route 
          (*it2)->valid = false;
          invalidRoutes.push_back(*it2);
          routes.erase(*it2);
        }
      }
    }
    // remove cCenter
    cCenters.erase(it);
    it = cCenters.end();
  }
  
  prepareRoutes(miners);
  
  for (it3 = fsmsToAbort.begin();
       it3 != fsmsToAbort.end();
       it3++) {
    (*it3)->abortMining();
    // FSM will request the new route calculated above
  }
}

void MineManager::removeWorker(MiningRoute* route, MineFSM* fsm) {
  removeFromRoute(route, fsm);
}

void MineManager::removeFromRoute(MiningRoute* route, MineFSM* fsm) {
  for (list<MineFSM*>::iterator it= route->fsms.begin();
       it != route->fsms.end();
       it++) {
    if (*it == fsm) {
      removeFromRoute(route, it);
    }
  }
}

void MineManager::removeFromRoute(MiningRoute* route, 
                                  list<MineFSM*>::iterator it) {
  //assignments.erase(*it->getSoarGameObject());
  route->fsms.erase(it);
  //route->mineStation->optimality--;
  //route->dropoffStation->optimality--;
  adjustOptimality(route);
}

    
void MineManager::addFSMToRoute(MiningRoute* route, MineFSM* fsm) {
  // the cost must have been added!
  route->fsms.push_back(fsm);
}

void MineManager::addCostToRoute(MiningRoute* route) {
  assert(route->mineStation != NULL);
  assert(route->dropoffStation != NULL);
  route->mineStation->optimality += 1000;
  route->dropoffStation->optimality += 1000;
  
  for (list<MiningRoute*>::iterator it = route->mineStation->routes.begin();
       it != route->mineStation->routes.end();
       it++) {
    adjustOptimality(*it);
  }
  for (list<MiningRoute*>::iterator it = route->dropoffStation->routes.begin();
       it != route->dropoffStation->routes.end();
       it++) {
    adjustOptimality(*it);
  }
}

void MineManager::adjustOptimality(MiningRoute* route) {
  set<MiningRoute*>::iterator it = routes.find(route);
  assert(it != routes.end());
  routes.erase(it);
  calculateOptimality(route);
  routes.insert(route);
}

void MineManager::calculateOptimality(MiningRoute* route) {
  // for now, just make it infeasible to use the same station as another route
  if (route->mineStation != NULL) {
    route->optimality = route->pathlength 
                        + 100 * (route->mineStation->optimality)
                        + 100 * (route->dropoffStation->optimality);
  }
  else {
    route->optimality = route->pathlength;
  }
}

void MineManager::addRoute(CCenterInfo* cci, MineralInfo* mi) {
  coordinate miningLoc = mi->mineral->getLocation();
  coordinate ccLoc = cci->cCenter->getLocation();
  MiningRoute* route;
 
  // make a new route for each set of mining point / dropoff point pairs
  route = new MiningRoute;
  //route->mineral = mi->mineral;
  //route->cCenter = cci->cCenter;
  route->mineralInfo = mi;
  route->cCenterInfo = cci;
  route->miningLoc = miningLoc;
  route->dropoffLoc = ccLoc;
  route->valid = true;
  
  route->pathlength = coordDistance(route->miningLoc, route->dropoffLoc)
                     - CCENTER_MAXRADIUS
                     - MINERAL_RADIUS; 
  route->stage = STRAIGHT_LINE_DIST;
  route->mineStation = NULL;
  route->dropoffStation = NULL;
  calculateOptimality(route);
  routes.insert(route);
  mi->routes.push_back(route);
  cci->routes.push_back(route);
  cout << "MM added SLD route: " << (int)cci->cCenter << " to " << (int)mi->mineral << " opt: " << route->optimality <<  endl;
  cout << route->miningLoc.x << "," << route->miningLoc.y 
    << " -> " << route->dropoffLoc.x << "," << route->dropoffLoc.y << endl;
} 

MiningRoute* MineManager::getBestRoute() {
  // keep expanding routes until the best (top of the set)
  // is a point-to-point path
  MiningRoute* topRoute = *routes.begin();
  if (topRoute == *routes.end()) {
    cout << "error: trying to mine w/o any routes (is at least one mineral and cc in view?\n";
  assert(false);
  }
  RouteHeuristicStage topStage = topRoute->stage;
  
  while (topStage != PF_DIST) {
    cout << "MM: looking at route "; 
    cout << "opt: " << topRoute->optimality << endl;
    switch (topStage) {
      case STRAIGHT_LINE_DIST:
        expandSLD(topRoute);
        break;
      case OBJ_OBJ_PF_DIST:
        // expand to 16 edge-edge routes
        expandObjObj(topRoute); 
        break;
      case EDGE_EDGE_PF_DIST:
        // expand to point-to-point routes
        expandEdgeEdge(topRoute);
        break;
      case PF_DIST:
        // this will never happen (while condition)
        break;
    
    }
    topRoute = *routes.begin();
    assert(topRoute->valid);
    topStage = topRoute->stage;
  }

  cout << "MM: top route is ";
  cout << topRoute->dropoffLoc.x << "," << topRoute->dropoffLoc.y << " to ";
  cout << topRoute->miningLoc.x << "," << topRoute->miningLoc.y << endl;

  return topRoute;
}

void MineManager::expandSLD(MiningRoute* route) {
  cout << "MM: expandSLD\n";
  double oldPath = route->pathlength;
  route->pathlength = pathFindDist(route->mineralInfo->mineral, 
                                   route->cCenterInfo->cCenter);
  assert (route->pathlength >= oldPath-1);
  route->stage = OBJ_OBJ_PF_DIST;
  if (route->pathlength == -1) {
    route->valid = false;
    routes.erase(route);
    invalidRoutes.push_back(route);
    //delete route;
  }
  else {
    route->pathlength -= CCENTER_MAXRADIUS;
    route->pathlength -= MINERAL_RADIUS;
    adjustOptimality(route);
    cout << "opt is now: " << route->optimality << endl;
  }
}
void MineManager::expandObjObj(MiningRoute* route) {
  cout << "MM: expandOO\n";
  // make 16 edge-edge routes from an obj-obj route
  // the existing locations in the route are the centers of the objects
  
  MiningRoute* newRoute;
  // temporary stations for this expansion stage
  StationInfo* dropoffStation;
  StationInfo* miningStation;
  
  for (int i=0; i<4; i++) {
    for (int j=0; j<4; j++) {
      newRoute = new MiningRoute(*route);
      switch (i) {
        case 0:
          // need to be w/in 2 to mine..
          newRoute->miningLoc.x -= (WORKER_RADIUS + MINERAL_RADIUS + 1);
          break;
        case 1:
          newRoute->miningLoc.x += (WORKER_RADIUS + MINERAL_RADIUS + 1);
          break;
        case 2:
          newRoute->miningLoc.y -= (WORKER_RADIUS + MINERAL_RADIUS + 1);
          break;
        case 3:
          newRoute->miningLoc.y += (WORKER_RADIUS + MINERAL_RADIUS + 1);
          break;
      }
      switch (j) {
        case 0:
          // need to be w/in 3 to dropoff..
          newRoute->dropoffLoc.x -= (WORKER_RADIUS + CCENTER_MINRADIUS + 2);
          break;
        case 1:
          newRoute->dropoffLoc.x += (WORKER_RADIUS + CCENTER_MINRADIUS + 2);
          break;
        case 2:
          newRoute->dropoffLoc.y -= (WORKER_RADIUS + CCENTER_MINRADIUS + 2);
          break;
        case 3:
          newRoute->dropoffLoc.y += (WORKER_RADIUS + CCENTER_MINRADIUS + 2);
          break;
      }
      newRoute->pathlength = pathFindDist(newRoute->dropoffLoc, 
                                          newRoute->miningLoc);
      if (newRoute->pathlength == -1) {
        cout << "deleting unreachable edge-edge route\n";
        delete newRoute;
      }
      else {
        calculateOptimality(newRoute);
        cout << "adding route: " << newRoute->optimality << endl;
        routes.insert(newRoute);
        newRoute->mineralInfo->routes.push_back(newRoute);
        newRoute->cCenterInfo->routes.push_back(newRoute);
        newRoute->stage = EDGE_EDGE_PF_DIST;
      }
    }
  }
  // remove the original route
  routes.erase(route);
  //delete route;
  route->valid = false;
  invalidRoutes.push_back(route);
}

void MineManager::expandEdgeEdge(MiningRoute* route) {
  cout << "MM: expandEE\n";
  
  // make 22 point-point routes from an edge-edge route
  // the existing locations in the route are the centers of the edges
  
  Direction mineralDir 
    = getRelDirection(route->mineralInfo->mineral->getLocation(), 
                      route->miningLoc);
  Direction cCenterDir 
    = getRelDirection(route->cCenterInfo->cCenter->getLocation(), 
                      route->dropoffLoc);

  bool ccHasStations = route->cCenterInfo->stationsValid[cCenterDir];
  bool mineralHasStations = route->mineralInfo->stationsValid[mineralDir];
 
  MiningRoute* newRoute;
  
  if (not mineralHasStations) {
    allocateMiningStations(route->mineralInfo, mineralDir);
  }
  if (not ccHasStations) {
    allocateDropoffStations(route->cCenterInfo, cCenterDir);
  }
  
  StationInfo* mineStation;
  StationInfo* dropoffStation;
  
  for (int i=0; i<MINERAL_EDGE_STATIONS; i++) {
    for (int j=0; j<CC_EDGE_STATIONS; j++) {
      switch (mineralDir) {
        case NORTH:
          mineStation = route->mineralInfo->northStations[i];
          break;
        case SOUTH:
          mineStation = route->mineralInfo->southStations[i];
          break;
        case EAST:
          mineStation = route->mineralInfo->eastStations[i];
          break;
        case WEST:
          mineStation = route->mineralInfo->westStations[i];
          break;
      }
      switch (cCenterDir) {
        case NORTH:
          dropoffStation = route->cCenterInfo->northStations[j];
          break;
        case SOUTH:
          dropoffStation = route->cCenterInfo->southStations[j];
          break;
        case EAST:
          dropoffStation = route->cCenterInfo->eastStations[j];
          break;
        case WEST:
          dropoffStation = route->cCenterInfo->westStations[j];
          break;
      }

      if (mineStation->optimality != -1
          and dropoffStation->optimality != -1) {
        newRoute = new MiningRoute(*route);
        newRoute->mineStation = mineStation;
        newRoute->dropoffStation = dropoffStation;
        newRoute->stage = PF_DIST;

        // yes, this is redundant, 
        // (route->location == route->station->location)
        // but the location at the top level must exist, since some routes
        // don't have stations (as they are not fully expanded)
        newRoute->miningLoc = mineStation->location;
        newRoute->dropoffLoc = dropoffStation->location;

        if (dropoffStation->optimality > 500
            or mineStation->optimality > 500) {
          cout << "immediately rejecting EE route, station is full.\n";
          delete newRoute;
        }
        else {

          newRoute->pathlength = pathFindDist(newRoute->dropoffStation->location, 
                                              newRoute->mineStation->location);

          if (newRoute->pathlength == -1) {
            cout << "deleting unreachable point-point route\n";
            delete newRoute;
          }
          else {
            // register the route with everyone who needs to know about it
            mineStation->routes.push_back(newRoute);
            dropoffStation->routes.push_back(newRoute);
            newRoute->mineralInfo->routes.push_back(newRoute);
            newRoute->cCenterInfo->routes.push_back(newRoute);
            
            calculateOptimality(newRoute);
            cout << "adding route: " << newRoute->optimality << endl;
            routes.insert(newRoute);
          }
        }
      }
      else {
        cout << "not adding route, a station is unreachable.\n";
      }
    }
  }
  route->valid = false;
  routes.erase(route);
  invalidRoutes.push_back(route);
}
      
void MineManager::allocateMiningStations(MineralInfo* mi, Direction d) {
  StationInfo* newStation;
  coordinate mineralCenter = mi->mineral->getLocation();
  
  assert (MINERAL_EDGE_STATIONS == 2);
  for (int i=0; i<2; i++) {
    newStation = new StationInfo;
    allStations.push_back(newStation);
    switch (d) {
      case NORTH:
        mi->northStations[i] = newStation;
        newStation->location.x = mineralCenter.x - 4 + i*7;
        newStation->location.y 
          = mineralCenter.y + WORKER_RADIUS + MINERAL_RADIUS + 1;
        break;
      case SOUTH:
        mi->southStations[i] = newStation;
        newStation->location.x = mineralCenter.x - 4 + i*7;
        newStation->location.y 
          = mineralCenter.y - (WORKER_RADIUS + MINERAL_RADIUS + 1);
        break;
      case EAST:
        mi->eastStations[i] = newStation;
        newStation->location.x 
          = mineralCenter.x + WORKER_RADIUS + MINERAL_RADIUS + 1;
        newStation->location.y = mineralCenter.y - 4 + i*7;
        break;
      case WEST:
        mi->westStations[i] = newStation;
        newStation->location.x 
          = mineralCenter.x - (WORKER_RADIUS + MINERAL_RADIUS + 1);
        newStation->location.y = mineralCenter.y - 4 + i*7;
        break;
    }
    if (stationBlocked(newStation->location)) {
      newStation->optimality = -1; // code for blocked
    }
    else {
      newStation->optimality = 0;
    }
  }
  mi->stationsValid[d] = true;
}

void MineManager::allocateDropoffStations(CCenterInfo* cci, Direction d) {
  StationInfo* newStation;
  coordinate ccCenter = cci->cCenter->getLocation();
  
  assert (CC_EDGE_STATIONS == 9);
  for (int i=0; i<9; i++) {
    newStation = new StationInfo;
    switch (d) {
      case NORTH:
        cci->northStations[i] = newStation;
        newStation->location.x = ccCenter.x - 33 + i*7;
        newStation->location.y 
          = ccCenter.y + WORKER_RADIUS + CCENTER_MINRADIUS + 2;
        break;
      case SOUTH:
        cci->southStations[i] = newStation;
        newStation->location.x = ccCenter.x - 33 + i*7;
        newStation->location.y 
          = ccCenter.y - (WORKER_RADIUS + CCENTER_MINRADIUS + 2);
        break;
      case EAST:
        cci->eastStations[i] = newStation;
        newStation->location.x 
          = ccCenter.x + WORKER_RADIUS + CCENTER_MINRADIUS + 2;
        newStation->location.y = ccCenter.y - 33 + i*7;
        break;
      case WEST:
        cci->westStations[i] = newStation;
        newStation->location.x 
          = ccCenter.x - (WORKER_RADIUS + CCENTER_MINRADIUS + 2);
        newStation->location.y = ccCenter.y - 33 + i*7;
        break;
    }
    if (stationBlocked(newStation->location)) {
      newStation->optimality = -1; // code for blocked
    }
    else {
      newStation->optimality = 0;
    }
  }
  cci->stationsValid[d] = true;
}

double MineManager::pathFindDist(SoarGameObject* obj1, SoarGameObject* obj2) {
  TerrainBase::Path path;
  TerrainBase::Loc prevLoc;
  cout << "finding path:\n";
  cout << *(obj1->gob->sod.x) << ","<< *(obj1->gob->sod.y);
  cout << " -> " << *(obj2->gob->sod.x) << ","<< *(obj2->gob->sod.y);
  cout << endl;
  Sorts::terrainModule->findPath(obj1->gob, obj2->gob, path);
  double result = 0;
  if (path.locs.size() == 0) {
    result = -1;
  }
  else {
    // path is obj1->obj2, waypoints in reverse order
    prevLoc.x = *obj1->gob->sod.x;
    prevLoc.y = *obj1->gob->sod.y;
    cout << "loc: " << prevLoc.x << "," << prevLoc.y << endl;
    for (int i=path.locs.size()-1; i>=0; i--) {
      result += path.locs[i].distance(prevLoc);
      cout << "loc: " << path.locs[i].x << "," << path.locs[i].y << endl;
      prevLoc = path.locs[i];
    }
    cout << "then: " <<*obj2->gob->sod.x << "," << *obj2->gob->sod.y << endl;
  }
  return result;
}

double MineManager::pathFindDist(coordinate loc1, coordinate loc2) {
  TerrainBase::Path path;
  TerrainBase::Loc prevLoc;
  TerrainBase::Loc tloc1;
  TerrainBase::Loc tloc2;
  tloc1.x = loc1.x;
  tloc1.y = loc1.y;
  tloc2.x = loc2.x;
  tloc2.y = loc2.y;
  Sorts::terrainModule->findPath(tloc1, tloc2, path);
  double result = 0;
  if (path.locs.size() == 0) {
    result = -1;
  }
  else {
    // path is obj1->obj2, waypoints in reverse order
    prevLoc.x = loc1.x;
    prevLoc.y = loc1.y;
    cout << "loc: " << loc1.x << "," << loc1.y << endl;
    for (int i=path.locs.size()-1; i>=0; i--) {
      cout << "loc: " << path.locs[i].x << "," << path.locs[i].y << endl;
      result += path.locs[i].distance(prevLoc);
      prevLoc = path.locs[i];
    }
  }
  return result;
}

bool MineManager::stationBlocked(coordinate c) {
  return false;
}

Direction MineManager::getRelDirection(coordinate first, coordinate second) {
  // return relative direction of second to first
  
  assert(second.x == first.x || second.y == first.y);
  if (second.x > first.x) {
    return EAST;
  }
  if (second.y > first.y) {
    return NORTH;
  }
  if (second.x < first.x) {
    return WEST;
  }
  else {
    assert(second.y < first.y);
    return SOUTH;
  }
}
