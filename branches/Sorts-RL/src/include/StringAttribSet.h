#ifndef _StringAttribSet_h_
#define _StringAttribSet_h_

#include <set>
#include <boost/algorithm/string.hpp>
#include <sml_Client.h>
#include "WMAttribValueSet.h"

using namespace sml;
using std::set;

class StringAttribSet : public WMAttribValueSet {
public:
  StringAttribSet() { }

  StringAttribSet(const string& strs) {
    vector<string> pieces;
    boost::algorithm::split(pieces, strs, is_any_of(", "));
    stringSet.insert(pieces.begin(), pieces.end());
  }

  void addString(string s) {
    stringSet.insert(s);
  }

  bool contains(WMElement* attrib) {
    string s = attrib->GetValueAsString();
    return stringSet.find(s) != stringSet.end();
  }

private:
  set<string> stringSet;
};

#endif
