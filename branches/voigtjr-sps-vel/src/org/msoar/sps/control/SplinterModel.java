package org.msoar.sps.control;

import jmat.LinAlg;
import jmat.MathUtil;
import lcmtypes.pose_t;

import org.apache.log4j.Logger;

final class SplinterModel {
	private static final Logger logger = Logger.getLogger(SplinterModel.class);
	
	static SplinterModel newInstance() {
		return new SplinterModel();
	}
	
	private final LCMProxy lcmProxy;
	private final SplinterHardware hardware;
	private final pose_t pose = new pose_t();
	
	private SplinterModel() {
		this.lcmProxy = LCMProxy.getInstance();
		this.hardware = SplinterHardware.newInstance(this.lcmProxy);
		
		if (pose.utime != 0) {
			throw new AssertionError();
		}
	}
	
	void update(DifferentialDriveCommand command) {
		// elapsed time
		double dt = updatePose();
		if (Double.compare(dt, 0) == 0) {
			// invalid state
			return;
		}
		updateDC(dt);
	}
	
	private void updateDC(double dt) {
		
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

	private double updatePose() {
		// get the new pose
		pose_t lcmPose = lcmProxy.getPose();
		if (lcmPose == null) {
			// haven't gotten one yet
			return 0;
		}
		
		if (lcmPose.utime <= pose.utime) {
			// not new information (LCM uses UDP)
			return 0;
		}
		
		if (pose.utime == 0) {
			// this is our first message
			bootstrapPose(lcmPose);
			return 0;
		}
		
		double dt = (lcmPose.utime - pose.utime) * (1.0 / 1000000.0); // (new usec - old usec) * 1 / usec in sec

		pose.vel[0] = (lcmPose.pos[0] - pose.pos[0]) * (1.0 / dt);	// TODO: may need modification for smoothing
		pose.vel[1] = (lcmPose.pos[1] - pose.pos[1]) * (1.0 / dt);
		
		double newTheta = LinAlg.quatToRollPitchYaw(lcmPose.orientation)[2];
		newTheta = MathUtil.mod2pi(newTheta);
		double oldTheta = LinAlg.quatToRollPitchYaw(pose.orientation)[2];
		oldTheta = MathUtil.mod2pi(oldTheta);
		pose.rotation_rate[2] = MathUtil.mod2pi(newTheta - oldTheta) * (1.0 / dt);
		
		if (logger.isTraceEnabled()) {
			logger.trace(String.format("dt%f vx%f vy%f r%f", dt, pose.vel[0], pose.vel[1], pose.rotation_rate[2]));
		}
		updatePose(lcmPose);
		
		return dt;
	}

	private void updatePose(pose_t newPose) {
		pose.utime = newPose.utime;
		pose.pos[0] = newPose.pos[0];
		pose.pos[1] = newPose.pos[1];
		pose.orientation[0] = newPose.orientation[0];
		pose.orientation[3] = newPose.orientation[3];
	}
	
	int getCommandResult(int id) {
		// TODO: return result for soar/whoever 
		return -1;
	}
	
	pose_t getSplinterPose() {
		return pose;
	}
}
