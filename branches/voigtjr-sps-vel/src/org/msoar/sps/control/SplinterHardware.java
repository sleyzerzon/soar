package org.msoar.sps.control;

import lcmtypes.differential_drive_command_t;

import org.apache.log4j.Logger;

final class SplinterHardware implements Runnable {
	private static final Logger logger = Logger.getLogger(SplinterModel.class);

	static SplinterHardware newInstance(LCMProxy lcmProxy) {
		return new SplinterHardware(lcmProxy);
	}
	
	private final LCMProxy lcmProxy;
	private final differential_drive_command_t dc = new differential_drive_command_t();
	private final PIDSetting pid = new PIDSetting();
	private double av;
	private double lv;
	
	private SplinterHardware(LCMProxy lcmProxy) {
		this.lcmProxy = lcmProxy;
		
		this.dc.left_enabled = true;
		this.dc.right_enabled = true;
		
		setMotors(0, 0);
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
		this.dc.left = left;
		this.dc.right = right;
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
		// compute
		if (pid.isEnabled()) {
			// TODO
			throw new AssertionError();
		}
		
		// transmit dc
		assert dc.left_enabled == true;
		assert dc.right_enabled == true;
		lcmProxy.transmitDC(dc);
	}
	
	
}
