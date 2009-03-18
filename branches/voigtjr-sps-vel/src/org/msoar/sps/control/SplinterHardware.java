package org.msoar.sps.control;

import java.util.Timer;
import java.util.TimerTask;

import jmat.LinAlg;
import jmat.MathUtil;

import lcmtypes.differential_drive_command_t;
import lcmtypes.pose_t;

import org.apache.log4j.Logger;

final class SplinterHardware extends TimerTask {
	private static final Logger logger = Logger.getLogger(SplinterHardware.class);
	
	static SplinterHardware newInstance(LCMProxy lcmProxy) {
		return new SplinterHardware(lcmProxy);
	}
	
	private final LCMProxy lcmProxy;
	private final differential_drive_command_t dc = new differential_drive_command_t();
	private final PIDSetting pid = new PIDSetting();
	private double av;
	private double lv;
	private final Timer timer = new Timer(true);
	private pose_t pose;
	private long lastMillis;
	//private PIDController aController = new PIDController(0.12, 0, 0.015);	// experimentally derived
	//private PIDController lController = new PIDController(0.7,  0, 0.1);	// experimentally derived
	private PIDController aController = new PIDController(0.0238, 0, 0.0025); // experimentally derived in lab
	private PIDController lController = new PIDController(0.12,  0, 0.025);	// experimentally derived in lab
	
	private SplinterHardware(LCMProxy lcmProxy) {
		this.lcmProxy = lcmProxy;
		
		this.dc.left_enabled = true;
		this.dc.right_enabled = true;
		
		setMotors(0, 0);
		
		// TODO: make configurable
		timer.schedule(this, 0, 1000 / 30); // 30 Hz	
	}
	
	void setAGains(double p, double i, double d) {
		logger.info(String.format("Angular gains: p%f i%f d%f", p, i, d));
		aController.setPGain(p);
		aController.setIGain(i);
		aController.setDGain(d);
	}

	void setLGains(double p, double i, double d) {
		logger.info(String.format("Linear gains: p%f i%f d%f", p, i, d));
		lController.setPGain(p);
		lController.setIGain(i);
		lController.setDGain(d);
	}

	void setPose(pose_t pose) {
		this.pose = pose;
	}
	
	void setUTime(long utime) {
		dc.utime = utime;
	}
	
	void setAngularVelocity(double av) {
		pid.enablePID();
		
		this.av = av;
	}
	
	void setLinearVelocity(double lv) {
		pid.enablePID();
		
		this.lv = lv;
	}
	
	void setMotors(double left, double right) {
		pid.disablePID();
		
		// set motors
		dc.left = left;
		dc.right = right;
	}
	
	void estop() {
		setMotors(0, 0);
	}
	
	private final class PIDSetting {
		private boolean pidEnabled = false;
		
		private void enablePID() {
			if (pidEnabled) {
				return;
			}
			av = 0;
			lv = 0;
			aController.clearIntegral();
			lController.clearIntegral();
			
			pidEnabled = true;
		}
		 
		private void disablePID() {
			pidEnabled = false;
		}
		
		private boolean isEnabled() {
			return pidEnabled;
		}
	}

	public void run() {
		long current = System.currentTimeMillis();
		double dt = (current - lastMillis) / 1000.0;
		lastMillis = current;
		
		// compute
		if (pose != null && pid.isEnabled()) {
			if (Double.compare(pose.vel[2], 0) != 0) {
				throw new AssertionError();
			}

			double aout = aController.compute(dt, av, pose.rotation_rate[2]);
			dc.left -= aout;
			dc.right += aout;

			// convert vector to local frame and get forward (x) component
			double theta = MathUtil.mod2pi(LinAlg.quatToRollPitchYaw(pose.orientation)[2]);
			double xvel = LinAlg.rotate2(pose.vel, -theta)[0];
			double lout = lController.compute(dt, lv, xvel);
			dc.left += lout;
			dc.right += lout;

			// clamp -1..1
			dc.left = Math.max(dc.left, -1);
			dc.right = Math.max(dc.right, -1);
			dc.left = Math.min(dc.left, 1);
			dc.right = Math.min(dc.right, 1);
			
			logger.trace(String.format("a%f l%f", aout, lout));
		}
		
		// transmit dc
		assert dc.left_enabled == true;
		assert dc.right_enabled == true;
		lcmProxy.transmitDC(dc);
	}
}
