
#ifndef SVS_H
#define SVS_H

#include "soar_interface.h"
#include "scene.h"
#include "wm_sgo.h"
#include "sgel_interp.h"

typedef struct agent_struct agent;

class svs {
public:
	svs(soar_interface *soarinterface);
	void pre_env_callback();
	void post_env_callback();

private:
	soar_interface* soarint;
	scene           scn;
	sgel_interp     interp;
	wm_sgo*         wm_sgo_root;
};

#endif
