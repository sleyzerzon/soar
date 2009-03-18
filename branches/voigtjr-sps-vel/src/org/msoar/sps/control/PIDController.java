package org.msoar.sps.control;

import jmat.MathUtil;

class PIDController {
	private double pGain;
	private double iGain;
	private double dGain;
	private double previousError;
	private double integral;
	private double previousTarget;
	
	PIDController() {
	}
	
	PIDController(double pGain, double iGain, double dGain) {
		this.pGain = pGain;
		this.iGain = iGain;
		this.dGain = dGain;
	}
	
	void setPGain(double pGain) {
		this.pGain = pGain;
	}
	
	void setIGain(double iGain) {
		this.iGain = iGain;
	}
	
	void setDGain(double dGain) {
		this.dGain = dGain;
	}
	
	void clearIntegral() {
		integral = 0;
	}
	
	double computeMod2Pi(double dt, double target, double actual) {
		return computeInternal(dt, target, actual, true);
	}
	
	double compute(double dt, double target, double actual) {
		return computeInternal(dt, target, actual, false);
	}
	
	private double computeInternal(double dt, double target, double actual, boolean mod2pi) {
		if (Double.compare(dt, 0) == 0) {
			throw new IllegalArgumentException();
		}
		
		if (Math.signum(target) != Math.signum(previousTarget)) {
			integral = 0;
		}
		
		double error = target - actual;
		if (mod2pi) {
			error = MathUtil.mod2pi(error);
		}
		integral = integral + error * dt;
		double derivative = (error - previousError) / dt;

		double output = pGain * error;
		output += iGain * integral;
		output += dGain * derivative;

		previousError = error;
		previousTarget = target;
		
		return output;
	}
}
