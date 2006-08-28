#ifndef RLDeltaUpdater_H_
#define RLDeltaUpdater_H_

#include <map>
#include <sml_Client.h>

using namespace sml;
using std::string;
using std::map;

struct WatchData {
  string name;
  Identifier* id;
  string attribute;
  FloatElement* delta;
  double lastVal;
  
  WatchData() {
    name = "";
    id = NULL; 
    attribute = "";
    delta = NULL;
    lastVal = 0; 
  }

  WatchData
  ( const char* _name,
    Identifier* _id, 
    const char* _attribute, 
    FloatElement* _delta, 
    double _lastVal )
  : name(_name), id(_id), attribute(_attribute), delta(_delta), 
    lastVal(_lastVal)
  { }
};

class RLDeltaUpdater {
public:
  RLDeltaUpdater(Agent* agent);
  friend void updateWatches(smlRunEventId id, void* userData, Agent* agent, smlPhase phase);

private:
  Identifier* watchInfo;
  //Identifier* watchReqs;
  map<Identifier*, WatchData> watches;
};

#endif
