package edu.umich.soar.gridmap2d.players;


import java.util.Arrays;

import jmat.LinAlg;
import jmat.MathUtil;

import lcmtypes.pose_t;

public class RoomPlayerState {

	private pose_t pose;
	
	private boolean collisionX;
	private boolean collisionY;
	private int locationId;
	private boolean hasDestYaw;
	private double destYaw;
	private double destYawSpeed;

	public void reset() {
		pose = new pose_t();
		
		setCollisionX(false);
		setCollisionY(false);
		setLocationId(-1);
		resetDestYaw();
	}
	
	public void setCollisionX(boolean collisionX) {
		this.collisionX = collisionX;
	}

	public boolean isCollisionX() {
		return collisionX;
	}

	public void setCollisionY(boolean collisionY) {
		this.collisionY = collisionY;
	}

	public boolean isCollisionY() {
		return collisionY;
	}

	public void setLocationId(int locationId) {
		this.locationId = locationId;
	}

	public int getLocationId() {
		return locationId;
	}
	
	public double angleOff(double [] target) {
		double [] playerVector = new double [] { pose.pos[0], pose.pos[1] };
		
		double [] targetVector = new double [] { target[0], target[1] };
		
		// translate target so i'm the origin
		targetVector[0] -= playerVector[0];
		targetVector[1] -= playerVector[1];
		
		// make target unit vector
		double targetVectorLength = Math.sqrt(Math.pow(targetVector[0], 2) + Math.pow(targetVector[1], 2));
		if (targetVectorLength > 0) {
			targetVector[0] /= targetVectorLength;
			targetVector[1] /= targetVectorLength;
		} else {
			targetVector[0] = 0;
			targetVector[1] = 0;
		}
		
		// make player facing vector
		double yaw = MathUtil.mod2pi(getYaw());
		playerVector[0] = Math.cos(yaw);
		playerVector[1] = Math.sin(yaw);
		
		double dotProduct = (targetVector[0] * playerVector[0]) + (targetVector[1] * playerVector[1]);
		double crossProduct = (targetVector[0] * playerVector[1]) - (targetVector[1] * playerVector[0]);
		
		// calculate inverse cosine of that for angle
		if (crossProduct < 0) {
			return Math.acos(dotProduct);
		}
		return MathUtil.mod2pi(Math.acos(dotProduct) * -1);
	}

	public void stop() {
		Arrays.fill(pose.vel, 0);
		Arrays.fill(pose.rotation_rate, 0);
	}
	
	public double getYaw() {
		return MathUtil.mod2pi(LinAlg.quatToRollPitchYaw(pose.orientation)[2]);
	}
	
	public void update(double elapsed) {
		// rotate
		double[] rpy = LinAlg.quatToRollPitchYaw(pose.orientation);
		rpy[2] = MathUtil.mod2pi(rpy[2]);
		double togo = 0;
		if (hasDestYaw) {
			togo = destYaw - rpy[2];
			if (togo < 0) {
				pose.rotation_rate[2] = destYawSpeed * -1;
			} else {
				pose.rotation_rate[2] = destYawSpeed;
			}
		}
		double change = pose.rotation_rate[2] * elapsed;
		if (hasDestYaw) {
			if (Math.abs(change) > Math.abs(togo)) {
				change = togo;
			}
		}
		rpy[2] += change;
		rpy[2] = MathUtil.mod2pi(rpy[2]);
		double [] newVel = Arrays.copyOf(pose.vel, pose.vel.length);
		newVel[0] = Math.cos(change) * pose.vel[0] - Math.sin(change) * pose.vel[1];
		newVel[1] = Math.sin(change) * pose.vel[0] + Math.cos(change) * pose.vel[1];
		pose.vel = newVel;
		pose.orientation = LinAlg.rollPitchYawToQuat(rpy);

		// translate
		LinAlg.add(pose.pos, LinAlg.scale(pose.vel, elapsed), pose.pos);
	}
	
	public void setAngularVelocity(double angvel) {
		pose.rotation_rate[2] = angvel;
	}

	public void setLinearVelocity(double linvel) {
		double yaw = getYaw();
		pose.vel[0] = Math.cos(yaw) * linvel;
		pose.vel[1] = Math.sin(yaw) * linvel;
	}
	
	public pose_t getPose() {
		return pose.copy();
	}

	public void setPos(double [] pos) {
		pose.pos[0] = pos[0];
		pose.pos[1] = pos[1];
	}
	
	public void resetDestYaw() {
		hasDestYaw = false;
	}
	
	public void setDestYaw(double yaw, double speed) {
		hasDestYaw = true;
		destYaw = MathUtil.mod2pi(yaw);
		destYawSpeed = speed;
	}
}