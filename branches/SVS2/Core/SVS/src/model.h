#ifndef MODEL_H
#define MODEL_H

#include "scene.h"
#include "env.h"

class model {
public:
	virtual void predict(scene *scn, env_output out) = 0;
};

/* Doesn't do anything */
class null_model : public model {
public:
	null_model() { }
	void predict(scene *scn, env_output out) {}
};

#endif
