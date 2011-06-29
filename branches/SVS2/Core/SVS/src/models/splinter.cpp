#include <iostream>
#include "model.h"
#include "splinter.h"

using namespace std;

bool splinter_scene_update(flat_scene &scn, const output &out) {
	vec3 pos, vel, rot, rotrate;
	float lvolt, rvolt, lrps, rrps;
	
	if (!scn.get_node_trans("splinter", 'p', pos) ||
	    !scn.get_node_trans("splinter", 'r', rot) ||
	    !scn.get_property("splinter", "vel_0", vel[0]) ||
	    !scn.get_property("splinter", "vel_1", vel[1]) ||
	    !scn.get_property("splinter", "vel_2", vel[2]) ||
	    !scn.get_property("splinter", "rotation_rate_0", rotrate[0]) ||
	    !scn.get_property("splinter", "rotation_rate_1", rotrate[1]) ||
	    !scn.get_property("splinter", "rotation_rate_2", rotrate[2]) ||
	    !scn.get_property("splinter", "left_rads_per_sec", lrps) ||
	    !scn.get_property("splinter", "right_rads_per_sec", rrps))
	{
		return false;
	}
	
	lvolt = out.get("left");
	rvolt = out.get("right");
	
	splinter_update(lvolt, rvolt, lrps, rrps, pos, vel, rot, rotrate);
	
	scn.set_node_trans("splinter", 'p', pos);
	scn.set_node_trans("splinter", 'r', rot);
	scn.set_property("splinter", "vel_0", vel[0]);
	scn.set_property("splinter", "vel_1", vel[1]);
	scn.set_property("splinter", "vel_2", vel[2]);
	scn.set_property("splinter", "rotation_rate_0", rotrate[0]);
	scn.set_property("splinter", "rotation_rate_1", rotrate[1]);
	scn.set_property("splinter", "rotation_rate_2", rotrate[2]);
	scn.set_property("splinter", "left_rads_per_sec", lrps);
	scn.set_property("splinter", "right_rads_per_sec", rrps);
	
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
	
	void printinfo() const {
		cout << "SPLINTER" << endl;
	}
};

model *_make_splinter_model_(soar_interface *si, Symbol *root) {
	return new splinter_model();
}
