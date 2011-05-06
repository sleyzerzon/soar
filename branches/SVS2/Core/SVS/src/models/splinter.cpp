#include <iostream>
#include "model.h"
#include "splinter.h"

using namespace std;

bool splinter_scene_update(flat_scene &scn, const output &out) {
	vec3 pos, vel, rot, rotrate;
	double lvolt, rvolt, lrps, rrps;
	
	if (!scn.get_node_trans("splinter", 'p', pos) ||
	    !scn.get_node_trans("splinter", 'r', rot) ||
	    !scn.get_property("vel_0", vel.x) ||
	    !scn.get_property("vel_1", vel.y) ||
	    !scn.get_property("vel_2", vel.z) ||
	    !scn.get_property("rotation_rate_0", rotrate.x) ||
	    !scn.get_property("rotation_rate_1", rotrate.y) ||
	    !scn.get_property("rotation_rate_2", rotrate.z) ||
	    !scn.get_property("left_rads_per_sec", lrps) ||
	    !scn.get_property("right_rads_per_sec", rrps))
	{
		return false;
	}
	
	lvolt = out.get("left");
	rvolt = out.get("right");
	
	splinter_update(lvolt, rvolt, lrps, rrps, pos, vel, rot, rotrate);
	
	scn.set_node_trans("splinter", 'p', pos);
	scn.set_node_trans("splinter", 'r', rot);
	scn.set_property("vel_0", vel.x);
	scn.set_property("vel_1", vel.y);
	scn.set_property("vel_2", vel.z);
	scn.set_property("rotation_rate_0", rotrate.x);
	scn.set_property("rotation_rate_1", rotrate.y);
	scn.set_property("rotation_rate_2", rotrate.z);
	scn.set_property("left_rads_per_sec", lrps);
	scn.set_property("right_rads_per_sec", rrps);
	
	return true;
}

class splinter_model : public model {
public:
	splinter_model() {}
	
    bool predict(flat_scene &scn, const trajectory &trj) {
    	vector<output>::const_iterator i;
    	for (i = trj.t.begin(); i != trj.t.end(); ++i) {
    		if (!splinter_scene_update(scn, *i)) {
    			return false;
    		}
    	}
    	return true;
	}
};

model *_make_splinter_model_(soar_interface *si, Symbol *root) {
	return new splinter_model();
}
