#ifndef _WMAttribValueSet_h_
#define _WMAttribValueSet_h_

#include <sml_Client.h>

using namespace sml;

class WMAttribValueSet {
public:
  virtual ~WMAttribValueSet() { }
  virtual bool contains(WMElement* attrib) = 0;
};

#endif
