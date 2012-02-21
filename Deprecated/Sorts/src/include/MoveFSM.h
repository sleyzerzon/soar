/*
    This file is part of Sorts, an interface between Soar and ORTS.
    (c) 2006 James Irizarry, Sam Wintermute, and Joseph Xu

    Sorts is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Sorts is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Sorts; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA    
*/
#ifndef MoveFSM_H
#define MoveFSM_H

#include "FSM.h"
#include "Sorts.h"
#include "SortsSimpleTerrain.h"
#include "general.h"

class MoveFSM: public FSM {
 public:
  MoveFSM(GameObj*);
  ~MoveFSM();

  int update();
	void init(std::vector<sint4>);
  void initNoPath(std::vector<sint4>);

  bool getMoveVector(); //returns true if there needs to be a change in direction, otherwise false
  SortsSimpleTerrain::Loc getHeadingVector(sint4 , sint4);
  

  void stop();
  void panic();
  coordinate currentLocation;
  bool moveSimple(sint4 x, sint4 y);

 private:
	enum{IDLE,WARMUP,MOVING,ALREADY_THERE,UNREACHABLE,STUCK};

  void veerRight();

  void traverse(sint4 locx, sint4 locy, ScalarPoint goal);
  Vector<sint4> move(ScalarPoint loc, sint4 speed);
  
  
  bool veerAhead(int dtt);
  bool collision(int x, int y);
  bool dynamicCollision(int x, int y);
  bool isReachableFromBuilding(SortsSimpleTerrain::Loc l);
	int state;
  int runTime;
  double heading;

  void clearWPWorkers(); 
  
  GameObj* collidingGob(int x, int y);

  SortsSimpleTerrain::Loc target;

  SortsSimpleTerrain::Path path;
  int nextWPIndex;
  Vector<sint4> moveParams;
  int counter;
  int counter_max;

  int vec_count;
  int precision;
  bool lastRight;
  int veerCount;
  coordinate lastLocation;

  sint4 direction;
  sint4 idleTime;

  SortsSimpleTerrain::Loc imaginaryWorkerWaypoint;
  bool usingIWWP;
};

#endif
