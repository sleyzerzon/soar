
#ifndef SVS_H
#define SVS_H

#include "scene.h"

typedef struct agent_struct agent;

class svs {
public:
	svs(agent *a);
	void pre_env_callback();
	void post_env_callback();

private:
	struct agent_struct *agnt;
};

#endif
