#ifndef Sorts_H
#define Sorts_H

/* Sorts class

   highest-level class for the middleware
   has pointers to the various interfaces and managers

   most lower-level objects should have a pointer to this
*/

//#define USE_CANVAS

#include "SoarInterface.h"
#include "OrtsInterface.h"
#include "PerceptualGroupManager.h"
#include "MapManager.h"
#include "FeatureMapManager.h"
#include "GameActionManager.h"
class AttackManagerRegistry;
class MineManager;

#include "SpatialDB.h"
#include "TerrainModule.H"

#ifdef USE_CANVAS
#include "SortsCanvas.h"
#endif

class Sorts {
  public:
    Sorts
    ( SoarInterface*          _SoarIO,
      OrtsInterface*          _OrtsIO,
      PerceptualGroupManager* _pGroupManager, 
      MapManager*             _mapManager, 
      FeatureMapManager*      _featureMapManager,
      TerrainModule*          _tm,
      SpatialDB *             _spatialDB,
      AttackManagerRegistry*  _amr,
      MineManager*            _mineMan,
      GameActionManager*      _gam,
      pthread_mutex_t*        _mutex )
    {
      SoarIO = _SoarIO;
      OrtsIO = _OrtsIO;
      pGroupManager = _pGroupManager;
      mapManager = _mapManager;
      featureMapManager = _featureMapManager;
      terrainModule = _tm;
      spatialDB = _spatialDB;
      amr = _amr;
      mineManager = _mineMan;
      gameActionManager = _gam;
      mutex = _mutex;
      catchup = false;
      frame = -1;
    }

    static SoarInterface*           SoarIO;
    static OrtsInterface*           OrtsIO;
    static PerceptualGroupManager*  pGroupManager;
    static MapManager*              mapManager;
    static FeatureMapManager*       featureMapManager;
    static TerrainModule*           terrainModule;
    static SpatialDB*               spatialDB;
    static AttackManagerRegistry*   amr;
    static MineManager*             mineManager;
    static GameActionManager*       gameActionManager;
 //   static TerrainManager           terrainManager;
    static pthread_mutex_t*         mutex;
    static bool                     catchup;
    static int                      cyclesSoarAhead;
    static int frame;
#ifdef USE_CANVAS
    static SortsCanvas              canvas;
#endif
};



/* The mutex in the Sorts object is now the mutex for everything. Previously, 
   we used mutexes for the shared data structures and Soar IO, but this 
   simplifies things by putting a single mutex that only allows either
   the Soar-event code or the ORTS-event code to run at the same time. I'm
   leaving the old mutexes in SoarInterface.cpp, but commented out- if we
   ever can use actual multithreading, they will allow that (where this scheme
   will not). If that is the case, a mutex needs to be added around the 
   functions in PerceptualGroupManager, since both Soar and ORTS events call them.

   -sw, 4/25/06
*/
   

#endif
