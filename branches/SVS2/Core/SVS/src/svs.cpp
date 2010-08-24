#include <portability.h>

#include "svs.h"
#include "agent.h"
#include "nsg_node.h"
#include "wm_sgo.h"

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
		nsg_node *n = new nsg_node("blah"), *n1 = new nsg_node("child");
		wm_sgo *root = new wm_sgo(agnt, agnt->top_goal->id.svs_spatial_scene_contents_header, NULL, n);
		root->add_child(n1);
		init = true;
	}
}
