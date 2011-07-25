#include <iostream>
#include "model.h"
#include "splinter.h"

using namespace std;

const char *in_slots[] =  { "px", "py", "vx", "vy", "rz", "rtz", "lrps", "rrps", "lvolts", "rvolts" };
const char *out_slots[] = { "px", "py", "vx", "vy", "rz", "rtz", "lrps", "rrps" };

class splinter_model : public model {
public:
	splinter_model() {}
	
    bool predict(const floatvec &x, floatvec &y) {
		if (x.size() != 10 || y.size() != 8) {
			return false;
		}
		float px = x[0], py = x[1], vx = x[2], vy = x[3], rz = x[4], rtz = x[5],
		      lrps = x[6], rrps = x[7], lvolt = x[8], rvolt = x[9];
		splinter_update(px, py, vx, vy, rz, rtz, lrps, rrps, lvolt, rvolt);

		y[0] = px; y[1] = py; y[2] = vx; y[3] = vy; y[4] = rz; y[5] = rtz; y[6] = lrps; y[7] = rrps;
    	return true;
	}
	
	void printinfo() const {
		cout << "SPLINTER" << endl;
	}

	void get_slots(vector<string> &ins, vector<string> &outs) const {
		copy(in_slots, in_slots + 10, back_inserter(ins));
		copy(in_slots, in_slots + 8, back_inserter(outs));
	}
};

model *_make_splinter_model_(soar_interface *si, Symbol *root) {
	return new splinter_model();
}
