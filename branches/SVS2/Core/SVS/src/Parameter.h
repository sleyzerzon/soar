
// base class for parameters
class Parameter {
public:
  Parameter();
};

class GroundedObjectParameter: public Parameter {
public:
  GroundedObjectParameter(string _name, GroundedObject* _val);
  string name;
  GroundedObject* val;
};

class IdentifierParameter: public Parameter {
public:
  IdentifierParameter(string _name, string _identifierStringVal, 
      string _stringVal, bool _isLongTerm);
  string name;
  string identifierStringVal; // Soar representation, e.g., I34
  string stringVal; // environment-provided string, e.g., block:i12
  bool isLongTerm;
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

