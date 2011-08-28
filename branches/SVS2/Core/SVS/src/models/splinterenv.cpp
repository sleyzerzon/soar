/*
 Interface to use the splinterenv class as a model
*/

#include <iostream>
#include "splinterenv.h"
#include "model.h"

using namespace std;

class splinterenv_model : public model {
public:
	bool predict(const floatvec &x, floatvec &y) {
		return env.predict(x, y);
	}
	
	void get_slots(vector<string> &inputs, vector<string> &outputs) const {
		env.get_slots(inputs, outputs);
	}

	string get_type() const {
		return string("splinterenv");
	}
	
	int get_input_size() const {
		return env.get_input_size();
	}
	
	int get_output_size() const {
		return env.get_output_size();
	}
	
private:
	splinterenv env;
};

model *_make_splinterenv_model_(soar_interface *si, Symbol *root) {
	return new splinterenv_model();
}
