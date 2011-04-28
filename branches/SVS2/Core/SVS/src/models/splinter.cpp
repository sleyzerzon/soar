#include "model.h"

const double wheel_diameter     = 0.25;
const double baseline           = 0.35;
const double torque_constant    = 3.0; // torque (Nm) per amp
const double emf_constant       = 2.0; // volts per rad_per_sec
const double winding_resistance = 5.5; // ohms
const double inertia            = 0.5; // kg*m^2
const double drag_constant      = 1.0; // drag (Nm per rad_per_sec) ( >= 0)

class motor {
public:
	motor() : input_volts(0.0), amps(0.0), rad_per_sec(0.0) {}
	
	void set_voltage(double v) {
		input_volts = v;
	}
	
	void update(double dt) {
		double volts_emf = rad_per_sec * emf_constant;
		amps = (input_volts - volts_emf) / winding_resistance;
		double torque0 = amps * torque_constant;
		double torque_drag = rad_per_sec * drag_constant;
		double torque_net = torque0 - torque_drag;
		double acceleration = torque_net / inertia;
		rad_per_sec += acceleration * dt;
	}
    
    double get_rad_per_sec() {
    	return rad_per_sec;
    }
    
private:
	double input_volts, rad_per_sec, amps;
};

class splinter_model : public model {
public:
	splinter_model() {}
	
	void sim(double leftvoltage, double rightvoltage, double dt, 
	         vec3 &pos, vec3 &vel, vec3 &rot, vec3 &rotrate) 
	{
		left_motor.set_voltage(leftvoltage * 12); left_motor.update(dt);
		right_motor.set_voltage(rightvoltage * 12); right_motor.update(dt);
		
		double dleft  = dt * left_motor.get_rad_per_sec()  * wheel_diameter;
		double dright = dt * right_motor.get_rad_per_sec() * wheel_diameter;
		
		quaternion orient(rot);
		vel = orient.rotate(vec3((dleft + dright) / 2, 0, 0));
		rotrate.z = (dright - dleft) / baseline; rotrate.x = 0; rotrate.y = 0;
		pos = pos + vel;
		rot = (orient * quaternion(rotrate)).to_rpy();
	}
    
    bool predict(scene *scn, const env_output &out) {
		vec3 pos, vel, rot, rotrate;
		quaternion orient;
		double left_voltage, right_voltage, dt;
		sg_node *splinter;
		
		if (!(splinter = scn->get_node("splinter"))) {
			return false;
		}
		pos = splinter->get_trans('p');
		rot = splinter->get_trans('r');
		
		vel.x = scn->get_property("vel_0");
		vel.y = scn->get_property("vel_1");
		vel.z = scn->get_property("vel_2");
		rotrate.x = scn->get_property("rotation_rate_0");
		rotrate.y = scn->get_property("rotation_rate_1");
		rotrate.z = scn->get_property("rotation_rate_2");
		left_voltage = out.get("left");
		right_voltage = out.get("right");
		
		sim(left_voltage, right_voltage, dt, pos, vel, rot, rotrate);
		
		splinter->set_trans('p', pos);
		splinter->set_trans('r', rot);
		scn->set_property("vel_0", vel.x);
		scn->set_property("vel_1", vel.y);
		scn->set_property("vel_2", vel.z);
		scn->set_property("rotation_rate_0", rotrate.x);
		scn->set_property("rotation_rate_1", rotrate.y);
		scn->set_property("rotation_rate_2", rotrate.z);
		
		return true;
	}
private:
	motor left_motor, right_motor;
	
};

model *_make_splinter_model_(soar_interface *si, Symbol *root) {
	return new splinter_model();
}
