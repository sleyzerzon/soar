#include <portability.h>

#include "svs.h"
#include "agent.h"
#include "nsg_node.h"

svs::svs(agent *a) 
: agnt(a)
{
}

void svs::pre_env_callback()
{
}

void svs::post_env_callback()
{
	static bool init = false;
	if (!init) {
		nsg_node *n = new nsg_node("blah");
		wm_sgo(agnt, svs_sym_spatial_scene, NULL, n);
		init = true;
	}
}
