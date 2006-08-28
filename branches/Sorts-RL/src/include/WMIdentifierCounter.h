#ifndef _WMIdentifierCounter_h_
#define _WMIdentifierCounter_h_

#include <map>
#include <sml_Client.h>

#include "IdPattern.h"

using namespace sml;
using std::vector;
using std::map;
using std::string;

using boost::algorithm::split;
using boost::algorithm::is_any_of;

void updateCountsEventHandler
( smlRunEventId id,
    void*         userData,
    Agent*        agent,
    smlPhase      phase );

class WMIdentifierCounter {
public:
  WMIdentifierCounter(Agent* _agent);

  void addPattern(string name, const IdPattern& p);
  
  void setup();

  void updateCounts() const;

private:
  Agent* agent;
  Identifier* inputLink;
  Identifier* countLink;
  map<IntElement*, IdPattern> patterns;
};


#endif
