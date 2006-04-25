#ifndef FEATUREMAP_H
#define FEATUREMAP_H
/*
  FeatureMap.h

  Sam Wintermute, 2006
*/

#include <list>
#include "SoarGameGroup.h"
using namespace std;


//class SoarGameGroup;

class FeatureMap {
  public:
    FeatureMap();
    ~FeatureMap() { }
    SoarGameGroup* getGroup(int sector);
    // return a group from the given sector
    // cycle through the qualifying groups

    void addGroup(SoarGameGroup* group, int sector, int strength);
    // add the group to the map
    // note that we don't care about looking for the features
    // since the map itself doesn't know what the features are
  
    void removeGroup(SoarGameGroup* group, int sector, int strength);  
    // remove the group (if it exists)

    int getCount(int);

    void clear();
    // remove all objects

    void setIsPresent(bool);
    bool getIsPresent();

    void setIsStale(bool);
    bool getIsStale();
  private:
    vector<set<SoarGameGroup*> > fmSectors;
    vector<set<SoarGameGroup*>::iterator> fmSectorIterators;
    vector<int> fmCounts;
    bool isPresent; // true if FM is in Soar
    bool isStale;
};
#endif
