#ifndef SPLINTER_H
#define SPLINTER_H

#include "linalg.h"

const double wheel_diameter     = 0.25;
const double baseline           = 0.35;
const double torque_constant    = 3.0; // torque (Nm) per amp
const double emf_constant       = 2.0; // volts per rad_per_sec
const double winding_resistance = 5.5; // ohms
const double inertia            = 0.5; // kg*m^2
const double drag_constant      = 1.0; // drag (Nm per rad_per_sec) ( >= 0)
const double dt                 = 0.016; // need a better way to figure this out

inline double calc_rps(double rps, double input_volts) {
	double volts_emf = rps * emf_constant;
	double amps = (input_volts - volts_emf) / winding_resistance;
	double torque0 = amps * torque_constant;
	double torque_drag = rps * drag_constant;
	double torque_net = torque0 - torque_drag;
	double acceleration = torque_net / inertia;
	return rps + (acceleration * dt);
}

inline void splinter_update(double lvolt, double rvolt, double &lrps, double &rrps, 
                     vec3 &pos, vec3 &vel, vec3 &rot, vec3 &rotrate)
{
	lrps = calc_rps(lrps, lvolt * 12);
	rrps = calc_rps(rrps, rvolt * 12);
	double dleft  = dt * lrps * wheel_diameter;
	double dright = dt * rrps * wheel_diameter;
	
	quaternion orient(rot);
	vel = orient.rotate(vec3((dleft + dright) / 2, 0, 0));
	rotrate.z = (dright - dleft) / baseline; rotrate.x = 0; rotrate.y = 0;
	pos = pos + vel;
	rot += rotrate;
}

#endif
