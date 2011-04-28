#ifndef MODEL_H
#define MODEL_H

#include "scene.h"
#include "env.h"
#include "soar_interface.h"

class model {
public:
	virtual bool predict(flat_scene &scn, const env_output &out) = 0;
};

class learning_model : model {
public:
	virtual void add(const env_output &out, const flat_scene &scn) = 0;
};

model *parse_model_struct(soar_interface *si, Symbol *root);

#endif
