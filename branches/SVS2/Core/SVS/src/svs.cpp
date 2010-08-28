#include "svs.h"
#include "nsg_node.h"
#include "wm_sgo.h"
#include "soar_int.h"

using namespace std;

svs::svs(soar_interface *si)
: soarint(si)
{ }

svs::~svs() {
	delete soarint;
}

void svs::goal_creation_callback(sym_hnd goal) {
	soarint->prepare_new_goal(goal);
}

void svs::goal_deletion_callback(sym_hnd goal) {
	soarint->prepare_del_goal(goal);
}

void svs::pre_env_callback() {
}

void svs::post_env_callback() {
}

