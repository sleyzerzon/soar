#ifndef Utils_H
#define Utils_H

#include <iostream>

using namespace std;

inline std::string catStrInt(const char* str, int x) {
    ostringstream sstr;
    sstr << str << x;
    return sstr.str();
}

inline std::string int2str(int x) {
    ostringstream sstr;
    sstr << x;
    return sstr.str();
}

inline const char* getCommandParameter(sml::Identifier* c, const char *name) {
    const char *val = c->GetParameterValue(name);
    if (strlen(val) == 0) {
        cout << "Error: Parameter " << name << " does not exist." << endl;
        exit(1);
    }
    return val;
}

#endif 
