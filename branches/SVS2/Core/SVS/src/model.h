#ifndef MODEL_H
#define MODEL_H

#include "scene.h"
#include "env.h"
#include "soar_interface.h"

class model {
public:
	virtual bool predict(flat_scene &scn, const trajectory &trj) = 0;
	
	/* Add a training example to a learning model. The training
	   example is encoded as a pair of vectors (x, y) where
	   x = [prev_state, output], y = [next_state]
	*/
	virtual void learn(const flat_scene &pre, const output &out, const flat_scene &post) {}
};

model *parse_model_struct(soar_interface *si, Symbol *root, std::string &name);

#endif
