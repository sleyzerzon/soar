#include "model.h"

/* Doesn't do anything */
class null_model : public model {
public:
	null_model() { }
	bool predict(scene *scn, const env_output &out) { return true; }
};

model *_make_null_model_(soar_interface *si, Symbol* root) {
	return new null_model();
}
