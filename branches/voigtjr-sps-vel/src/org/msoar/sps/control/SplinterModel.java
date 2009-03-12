package org.msoar.sps.control;

import java.io.DataInputStream;
import java.io.IOException;
import java.util.Arrays;

import jmat.LinAlg;
import jmat.MathUtil;

import lcm.lcm.LCM;
import lcm.lcm.LCMSubscriber;
import lcmtypes.differential_drive_command_t;
import lcmtypes.pose_t;

import org.apache.log4j.Logger;
import org.msoar.sps.SharedNames;

final class SplinterModel implements LCMSubscriber {
	private static final Logger logger = Logger.getLogger(SplinterModel.class);
	
	static SplinterModel newInstance() {
		return new SplinterModel();
	}

	private enum MotionMode {
		STOP, TANK, ROTATE_TO;
	}
	
	private final LCM lcm = LCM.getSingleton();
	private pose_t lcmPose;
	private final pose_t pose = new pose_t();

	private final differential_drive_command_t dc;
	private MotionMode mode = MotionMode.STOP;
	private double yawTarget;
	private boolean yawTurnInPlace;
	private double yawRate;
	private boolean slow = false;
	
	private SplinterModel() {
		lcm.subscribe(SharedNames.POSE_CHANNEL, this);
		dc = new differential_drive_command_t();
	}
	
	public void messageReceived(LCM lcm, String channel, DataInputStream ins) {
		if (channel.equals(SharedNames.POSE_CHANNEL)) {
			try {
				pose_t newPose = new pose_t(ins);
				if (lcmPose == null) {
					bootstrapPose(newPose);
					lcmPose = newPose;
				} else if(lcmPose.utime < newPose.utime) {
					lcmPose = newPose;
				}
			} catch (IOException e) {
				logger.error("Error decoding pose_t message: " + e.getMessage());
			}
		}
	}
	
	private void bootstrapPose(pose_t newPose) {
		updatePose(newPose);
		assert Double.compare(newPose.pos[2], 0) == 0;
		assert Double.compare(newPose.orientation[1], 0) == 0;
		assert Double.compare(newPose.orientation[2], 0) == 0;
		assert Double.compare(newPose.vel[0], 0) == 0;
		assert Double.compare(newPose.vel[1], 0) == 0;
		assert Double.compare(newPose.vel[2], 0) == 0;
		assert Double.compare(newPose.rotation_rate[0], 0) == 0;
		assert Double.compare(newPose.rotation_rate[1], 0) == 0;
		assert Double.compare(newPose.rotation_rate[2], 0) == 0;
		assert Double.compare(newPose.accel[0], 0) == 0;
		assert Double.compare(newPose.accel[1], 0) == 0;
		assert Double.compare(newPose.accel[2], 0) == 0;
	}
	
	private void updatePose(pose_t newPose, double dt) {
		pose.vel[0] = (newPose.pos[0] - pose.pos[0]) * (1.0 / dt);
		pose.vel[1] = (newPose.pos[1] - pose.pos[1]) * (1.0 / dt);
		double newTheta = LinAlg.quatToRollPitchYaw(newPose.orientation)[2];
		newTheta = MathUtil.mod2pi(newTheta);
		double oldTheta = LinAlg.quatToRollPitchYaw(pose.orientation)[2];
		oldTheta = MathUtil.mod2pi(oldTheta);
		pose.rotation_rate[2] = MathUtil.mod2pi(newTheta - oldTheta) * (1.0 / dt);
		
		if (logger.isTraceEnabled()) {
			logger.trace(String.format("dt%f vx%f vy%f r%f", dt, pose.vel[0], pose.vel[1], pose.rotation_rate[2]));
		}
	}
	
	private void updatePose(pose_t newPose) {
		pose.utime = newPose.utime;
		pose.pos[0] = newPose.pos[0];
		pose.pos[1] = newPose.pos[1];
		pose.orientation[0] = newPose.orientation[0];
		pose.orientation[3] = newPose.orientation[3];
	}
	
	void update() {
		if (lcmPose == null) {
			return;
		}
		
		// poor mans synchronization
		pose_t newPose = lcmPose.copy();
		
		// elapsed time
		double dt = (newPose.utime - pose.utime) * (1.0 / 1000000.0); // (new usec - old usec) * 1 / usec in sec

		updatePose(newPose, dt);
		updateDC(dt);
		
		transmit();
	}
	
	private void updateDC(double dt) {
		double theta = LinAlg.quatToRollPitchYaw(pose.orientation)[2];
		
		// update dc
		dc.utime = pose.utime;
		dc.left_enabled = true;
		dc.right_enabled = true;
		switch (mode) {
		case STOP:
			dc.left = 0;
			dc.right = 0;
			break;
			
		case TANK:
			// already set
			break;
			
		case ROTATE_TO: // TODO: handle yawTurnInPlace
			// how far from goal am I
			double to = yawTarget - theta;
			to = MathUtil.mod2pi(to);
			
			// how fast should I be turning
			double angVel = yawRate;
			
			// linearly scale turn rate by distance from goal
			if (Double.compare(Math.abs(to), (yawRate * 0.5)) <= 0) { // if under half rate in dist from goal
				angVel *= to / (yawRate * 0.5);	// this gives it the same sign as to
			}
			
			// linearly scale delta by distance from turn rate
			double delta = 1 * dt; // some constant scaled to time
			delta *= (angVel - pose.rotation_rate[2]);

			// rotate in terms of left
			dc.left -= delta;
			dc.right += delta;
			
			logger.info(String.format("to%f r%f rr%f yr%f d%f", to, angVel, pose.rotation_rate[2], yawRate, delta));
			break;
		}
	}
	
	private void transmit() {
		differential_drive_command_t transmitDC = dc.copy();
		if (slow) {
			logger.debug("slow mode: halving throttle");
			transmitDC.left /= 2;
			transmitDC.right /= 2;
		}
		
		if (logger.isTraceEnabled()) {
			logger.trace("transmit: " + transmitDC.left + "," + transmitDC.right);
		}
		lcm.publish(SharedNames.DRIVE_CHANNEL, transmitDC);
	}

	void setSlow(boolean setting) {
		slow = setting;
	}
	
	void stop() {
		mode = MotionMode.STOP;
	}
	
	void move(double vel) {
		throw new AssertionError();
	}
	
	void rotate(double vel, boolean inPlace) {
		throw new AssertionError();
	}
	
	void rotateTo(double yawRad, double rate, boolean inPlace) {
		if (Double.compare(rate, 0) < 0) {
			throw new IllegalArgumentException("rate must be positive");
		}
		
		logger.debug("rotate-to: " + yawRad + " " + rate + " " + inPlace);
		
		yawTarget = MathUtil.mod2pi(yawRad);
		yawTurnInPlace = inPlace;
		yawRate = rate;
		mode = MotionMode.ROTATE_TO;
	}
	
	void joystick(double x, double y) {
		throw new AssertionError();
	}
	
	void tank(double left, double right) {
		dc.left = left;
		dc.right = right;
		mode = MotionMode.TANK;
	}
	
	public String toString() {
		StringBuilder sb = new StringBuilder();
		sb.append("[");
		sb.append(dc.left);
		sb.append(",");
		sb.append(dc.right);
		sb.append("] ");
		sb.append(pose.utime);
		return sb.toString();
	}
}
