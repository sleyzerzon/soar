#ifndef _IdPattern_h_
#define _IdPattern_h_

#include <iostream>
#include <map>
#include <boost/algorithm/string.hpp>
#include <sml_Client.h>

#include "WMAttribValueSet.h"

#define INPUT_STRUCT_NAME "MatchCounts"

using namespace sml;
using std::vector;
using std::map;
using std::string;

using boost::algorithm::split;
using boost::algorithm::is_any_of;


class IdPattern {
public:
  IdPattern() { }

  IdPattern(const IdPattern& other)
  : prefix(other.prefix), restrictions(other.restrictions),
    countAttrib(other.countAttrib)
  { }

  IdPattern
  ( const vector<string>& _prefix,  
    const map<string, WMAttribValueSet*>& _restrictions,
    const string& _countAttrib)
  : prefix(_prefix), restrictions(_restrictions), countAttrib(_countAttrib)
  { }

  IdPattern
  ( const string& prefixStr,  
    const map<string, WMAttribValueSet*>& _restrictions,
    const string& _countAttrib)
  : restrictions(_restrictions), countAttrib(_countAttrib)
  {
    split(prefix, prefixStr, is_any_of("."));
  }

  void addRestriction(string attrib, WMAttribValueSet* valSet) {
    restrictions[attrib] = valSet;
  }

  // returns the number of Identifiers this pattern matches, starting with
  // the root
  int count(Identifier* root) const {
    return count_r(root, 0);
  }

private: // functions
  int count_r(Identifier* root, int prefixPart) const {
    Identifier *currPar = root;
    if (prefixPart == prefix.size()) {
      // we've reached the identifier we want to match value attributes against
      for(map<string, WMAttribValueSet*>::const_iterator
          i  = restrictions.begin();
          i != restrictions.end();
          ++i)
      {
        bool matched = false;
        int n = 0;
        WMElement* attrib = root->FindByAttribute(i->first.c_str(), n);
        while (attrib) {
          if (i->second->contains(attrib)) {
            matched = true;
            break;
          }
          ++n;
          attrib = root->FindByAttribute(i->first.c_str(), n);
        }

        if (!matched) {
          // all attributes with this name have been searched to no avail
          // this ID is not a match
          return 0;
        }
      }
      // all restrictions met, this ID is a match
      if (countAttrib.length() == 0) {
        // count each instance as 1
        return 1;
      }
      else {
        WMElement* c = root->FindByAttribute(countAttrib.c_str(), 0);
        if (c) {
          IntElement* m = c->ConvertToIntElement();
          if (m) {
            return m->GetValue();
          }
        }
        return 1;
      }
    }
    else {
      // here, we still have to traverse further down the line
      int n = 0, numMatches = 0;
      WMElement* attrib = currPar->FindByAttribute(prefix[prefixPart].c_str(), n);
      while (attrib != NULL) {
        Identifier* child = attrib->ConvertToIdentifier();
        if (child) {
          numMatches += count_r(child, prefixPart + 1);
        }
        ++n;
        attrib = currPar->FindByAttribute(prefix[prefixPart].c_str(), n);
      }
      return numMatches;
    }
  }

private: // variables
  vector<string> prefix;
  map<string, WMAttribValueSet*> restrictions;
  string countAttrib;
};

#endif
