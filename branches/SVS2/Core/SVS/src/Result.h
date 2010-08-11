
// base class for results
// e.g.: ^value 10.2
// ^value true
// also maintains metadata (error or not)

class Result {
public:
  Result();
  bool isError();
};


class IntResult: public Result {
public:
  IntResult(int _val);
  int val;
};

// etc. for strings, floats, and bools
// need to determine whether to use dynamic_cast, or have an enum to detect
// types

