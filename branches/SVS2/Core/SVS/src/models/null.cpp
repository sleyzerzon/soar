#include "model.h"

/* Doesn't do anything */
class null_model : public model {
public:
	null_model() { }
	void predict(scene *scn, const env_output &out) {}
};

model *_make_null_model_(Symbol* id) {
	return new null_model();
}
