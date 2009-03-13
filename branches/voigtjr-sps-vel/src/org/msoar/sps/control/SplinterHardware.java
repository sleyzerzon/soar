package org.msoar.sps.control;

import java.util.Timer;
import java.util.TimerTask;

import jmat.LinAlg;

import lcmtypes.differential_drive_command_t;
import lcmtypes.pose_t;

import org.apache.log4j.Logger;

final class SplinterHardware extends TimerTask {
	private static final Logger logger = Logger.getLogger(SplinterHardware.class);
	private static final double PGAIN = 1;
	
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
	
	private SplinterHardware(LCMProxy lcmProxy) {
		this.lcmProxy = lcmProxy;
		
		this.dc.left_enabled = true;
		this.dc.right_enabled = true;
		
		setMotors(0, 0);
		
		// TODO: make configurable
		timer.schedule(this, 0, 1000 / 20); // 20 Hz	
	}

	public void setPose(pose_t pose) {
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
		throw new AssertionError("Not implemented");
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
			double gain = PGAIN * dt;
			
			double aerror = av - pose.rotation_rate[2];
			double apout = aerror * gain;
			
			dc.left -= apout;
			dc.right += apout;

			double lerror = lv - LinAlg.magnitude(pose.vel);
			double lpout = lerror * gain;
			
			dc.left += lpout;
			dc.right += lpout;

			logger.trace(String.format("ae%f le%f po%f", aerror, apout, lerror, lpout));
		}
		
		// transmit dc
		assert dc.left_enabled == true;
		assert dc.right_enabled == true;
		lcmProxy.transmitDC(dc);
	}
}
