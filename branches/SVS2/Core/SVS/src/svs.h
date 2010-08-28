#ifndef SVS_H
#define SVS_H

#include "soar_int.h"
#include "scene.h"
#include "wm_sgo.h"
#include "sgel_interp.h"

typedef struct svs_goal_struct {
	scene  scn;
	wm_sgo wm_sg_root;
} svs_goal_info;

class svs {
public:
	svs(soar_interface *soarinterface);
	~svs();
	
	void goal_creation_callback(sym_hnd goal);
	void goal_deletion_callback(sym_hnd goal);
	void pre_env_callback();
	void post_env_callback();

private:
	soar_interface* soarint;
};

#endif
