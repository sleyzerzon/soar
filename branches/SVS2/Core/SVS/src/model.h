#ifndef MODEL_H
#define MODEL_H

#include "scene.h"
#include "env.h"
#include "soar_interface.h"

class model {
public:
	virtual void predict(scene *scn, const env_output &out) = 0;
};

model *parse_model_struct(soar_interface *si, Symbol *root);

#endif
