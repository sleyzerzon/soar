#ifndef SPLINTER_H
#define SPLINTER_H

#include "linalg.h"

const float wheel_diameter     = 0.25;
const float baseline           = 0.35;
const float torque_constant    = 3.0; // torque (Nm) per amp
const float emf_constant       = 2.0; // volts per rad_per_sec
const float winding_resistance = 5.5; // ohms
const float inertia            = 0.5; // kg*m^2
const float drag_constant      = 1.0; // drag (Nm per rad_per_sec) ( >= 0)
const float dt                 = 0.016; // need a better way to figure this out

inline float calc_rps(float &rps, float input_volts) {
	float volts_emf = rps * emf_constant;
	float amps = (input_volts - volts_emf) / winding_resistance;
	float torque0 = amps * torque_constant;
	float torque_drag = rps * drag_constant;
	float torque_net = torque0 - torque_drag;
	float acceleration = torque_net / inertia;
	rps += acceleration * dt;
}

inline void splinter_update(float lvolt, float rvolt, float &lrps, float &rrps, 
                     vec3 &pos, vec3 &vel, vec3 &rot, vec3 &rotrate)
{
	calc_rps(lrps, lvolt * 12);
	calc_rps(rrps, rvolt * 12);
	float dleft  = dt * lrps * wheel_diameter;
	float dright = dt * rrps * wheel_diameter;
	
	quaternion orient(rot);
	vel = orient.rotate(vec3((dleft + dright) / 2, 0, 0));
	rotrate[0] = 0; rotrate[1] = 0; rotrate[2] = (dright - dleft) / baseline;
	pos = pos + vel;
	rot += rotrate;
}

#endif
