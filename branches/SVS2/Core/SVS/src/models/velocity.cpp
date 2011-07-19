#include <string>
#include "model.h"
#include "soar_interface.h"

using namespace std;

class velocity_model : public model {
public:
	velocity_model() { }

	bool predict(const floatvec &x, floatvec &y) {
		if (x.size() != 6 || y.size() != 3) {
			return false;
		}

		for (int i = 0; i < 3; ++i) {
			y[i] = x[i] + x[i + 3];
		}
		return true;
	}
	
	void printinfo() const {
		cout << "VELOCITY" << endl;
	}
	
	void get_slots(vector<string> &in_slots, vector<string> &out_slots) const {
		const char *in_names[] = { "px", "py", "pz", "vx", "vy", "vz" };
		const char *out_names[] = { "px", "py", "pz" };
		int i;

		for (i = 0; i < 6; ++i) {
			in_slots.push_back(in_names[i]);
		}
		for (i = 0; i < 3; ++i) {
			out_slots.push_back(out_names[i]);
		}
	}
};

model *_make_velocity_model_(soar_interface *si, Symbol *root) {
	return new velocity_model();
}
