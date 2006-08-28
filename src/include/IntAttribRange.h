#ifndef _IntAttribRange_h_
#define _IntAttribRange_h_

#include <boost/regex.hpp>
#include <sml_Client.h>
#include "WMAttribValueSet.h"

using namespace sml;
using std::string;

class IntAttribRange : public WMAttribValueSet {
public:
  IntAttribRange() { low = 0; high = 0; }

  IntAttribRange(int _low, int _high) : low(_low), high(_high) {}

  IntAttribRange(const string& descrip) {
    assert(operator=(descrip));
  }

  IntAttribRange(const IntAttribRange& other) { low = other.low; high = other.high; }

  void setRange(int _low, int _high) {
    low = _low; high = _high;
  }

  bool contains(WMElement* attrib) {
    IntElement* intElem = attrib->ConvertToIntElement();
    if (intElem == NULL) {
      assert(false && "Testing a non-int element against int range");
      return false;
    }
    
    return low <= intElem->GetValue() && intElem->GetValue() <= high;
  }

  bool operator=(const string& descrip) {
    boost::cmatch matches;
    boost::regex re("\\[(-?\\d+),(-?\\d+)\\]");
    if (boost::regex_match(descrip.c_str(), matches, re)) {
      string lower_match(matches[1].first, matches[1].second);
      string upper_match(matches[2].first, matches[2].second);
      int l = atoi(lower_match.c_str());
      int h = atoi(upper_match.c_str());
      if (l <= h) {
        low = l; high = h;
        return true;
      }
      else {
        return false;
      }
    }
    else {
      return false;
    }
  }

private:
  int low;
  int high;
};

#endif
