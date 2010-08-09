
// base class for parameters
class Parameter {
public:
  Parameter();
};

class IntParameter: public Parameter {
public:
  IntParameter(string _name, int _val);
  string name;
  int val;
};

// etc. for strings, floats, and bools
// need to determine whether to use dynamic_cast, or have an enum to detect
// types

