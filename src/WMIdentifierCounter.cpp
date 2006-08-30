#include "WMIdentifierCounter.h"

#include <iostream>
#include <utility>
#include <map>
#include <boost/algorithm/string.hpp>
#include <sml_Client.h>

#include "IdPattern.h"

#define INPUT_STRUCT_NAME "match-counts"

using namespace sml;
using std::cout;
using std::endl;
using std::vector;
using std::map;
using std::string;
using std::pair;

using boost::algorithm::split;
using boost::algorithm::is_any_of;

WMIdentifierCounter::WMIdentifierCounter(Agent* _agent) {
  agent = _agent;
  inputLink = agent->GetInputLink();
  countLink = agent->CreateIdWME(inputLink, INPUT_STRUCT_NAME);
  agent->RegisterForRunEvent(smlEVENT_BEFORE_DECISION_CYCLE, updateCountsEventHandler, this);

  // add all patterns we want to look at
  setup();
}

void WMIdentifierCounter::addPattern(string name, const IdPattern& p) {
  IntElement* pCountLink = agent->CreateIntWME(countLink, name.c_str(), -1);
  patterns.insert(pair<IntElement*,IdPatternCached>(pCountLink, IdPatternCached(p)));
}

void WMIdentifierCounter::updateCounts() {
  for(map<IntElement*, IdPatternCached>::iterator
      i  = patterns.begin();
      i != patterns.end();
      ++i) 
  {
    int count = i->second.pattern.count(inputLink);
    if (count != i->second.lastVal) {
      agent->Update(i->first, count);
      i->second.lastVal = count;
    }
  }
}

void updateCountsEventHandler
( smlRunEventId id,
    void*         userData,
    Agent*        agent,
    smlPhase      phase )
{
  ((WMIdentifierCounter*) userData)->updateCounts();
}

