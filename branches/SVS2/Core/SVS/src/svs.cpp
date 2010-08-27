#include "svs.h"
#include "nsg_node.h"
#include "wm_sgo.h"
#include "soar_interface.h"

using namespace std;

svs::svs(soar_interface *si)
: soarint(si), scn("world"), interp(&scn)
{
	wm_sgo_root = NULL;
}

void svs::pre_env_callback() {
	if (!wm_sgo_root) {
		wm_sgo_root = new wm_sgo(soarint, soarint->get_scene_root(), NULL, scn.get_node("world"));
	}
}

void svs::post_env_callback() {
	string line;
	while (soarint->get_env_line(line)) {
		interp.parse_line(line);
	}
}
