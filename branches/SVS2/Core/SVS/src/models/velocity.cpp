#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include "model.h"
#include "soar_interface.h"

using namespace std;

const char *in_names[]  = { "px", "py", "pz", "vx", "vy", "vz" };
const char *out_names[] = { "px", "py", "pz" };

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
	
	std::string get_type() const {
		return "velocity";
	}
	
	int get_input_size() const {
		return sizeof(in_names) / sizeof(char*);
	}
	
	int get_output_size() const {
		return sizeof(out_names) / sizeof(char*);
	}
	
	void get_slots(vector<string> &in_slots, vector<string> &out_slots) const {
		in_slots.clear(); out_slots.clear();
		copy(in_names, in_names + get_input_size(), back_inserter(in_slots));
		copy(out_names, out_names + get_output_size(), back_inserter(out_slots));
	}
};

model *_make_velocity_model_(soar_interface *si, Symbol *root) {
	return new velocity_model();
}
