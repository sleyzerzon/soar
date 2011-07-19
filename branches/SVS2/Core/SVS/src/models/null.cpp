#include <iostream>
#include "model.h"

using namespace std;

/* Doesn't do anything */
class null_model : public model {
public:
	null_model() { }
	bool predict(const floatvec &x, floatvec &y) { return true; }
	void printinfo() const {
		cout << "NULL" << endl;
	}
	void get_slots(vector<string> &ins, vector<string> &outs) const { }
};

model *_make_null_model_(soar_interface *si, Symbol* root) {
	return new null_model();
}
