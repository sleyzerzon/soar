#include <iostream>
#include "model.h"

using namespace std;

/* Doesn't do anything */
class null_model : public model {
public:
	null_model() { }
	bool predict(flat_scene &scn, const trajectory &trj) { return true; }
	void printinfo() const {
		cout << "NULL" << endl;
	}
};

model *_make_null_model_(soar_interface *si, Symbol* root) {
	return new null_model();
}
