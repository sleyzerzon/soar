#ifndef MODEL_H
#define MODEL_H

#include "scene.h"
#include "env.h"

class model {
public:
	virtual void predict(scene *scn, const env_output &out) = 0;
};

#endif
