#include <iostream>
#include "model.h"

using namespace std;

const double wheel_diameter     = 0.25;
const double baseline           = 0.35;
const double torque_constant    = 3.0; // torque (Nm) per amp
const double emf_constant       = 2.0; // volts per rad_per_sec
const double winding_resistance = 5.5; // ohms
const double inertia            = 0.5; // kg*m^2
const double drag_constant      = 1.0; // drag (Nm per rad_per_sec) ( >= 0)
const double dt                 = 0.016; // need a better way to figure this out

double calc_rps(double rps, double input_volts) {
	double volts_emf = rps * emf_constant;
	double amps = (input_volts - volts_emf) / winding_resistance;
	double torque0 = amps * torque_constant;
	double torque_drag = rps * drag_constant;
	double torque_net = torque0 - torque_drag;
	double acceleration = torque_net / inertia;
	return rps + (acceleration * dt);
}

void splinter_sim(double leftvoltage, double rightvoltage,
                  double &leftrps, double &rightrps,
                  vec3 &pos, vec3 &vel, vec3 &rot, vec3 &rotrate) 
{
	leftrps = calc_rps(leftrps, leftvoltage * 12);
	rightrps = calc_rps(rightrps, rightvoltage * 12);
	double dleft  = dt * leftrps  * wheel_diameter;
	double dright = dt * rightrps * wheel_diameter;
	quaternion orient(rot);
	vel = orient.rotate(vec3((dleft + dright) / 2, 0, 0));
	rotrate.z = (dright - dleft) / baseline; rotrate.x = 0; rotrate.y = 0;
	pos = pos + vel;
	rot = (orient * quaternion(rotrate)).to_rpy();
}

class splinter_model : public model {
public:
	splinter_model() {}
	
    bool predict(flat_scene &scn, const env_output &out) {
		vec3 pos, vel, rot, rotrate;
		quaternion orient;
		double lv, rv, lrps, rrps;
		
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
		
		lv = out.get("left");
		rv = out.get("right");
		
		splinter_sim(lv, rv, lrps, rrps, pos, vel, rot, rotrate);
		
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
};

model *_make_splinter_model_(soar_interface *si, Symbol *root) {
	return new splinter_model();
}
