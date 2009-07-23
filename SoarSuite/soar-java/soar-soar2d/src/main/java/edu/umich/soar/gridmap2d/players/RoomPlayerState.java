package edu.umich.soar.gridmap2d.players;


import java.util.Arrays;

import edu.umich.soar.gridmap2d.Simulation;

public class RoomPlayerState {

	private double angVel;
	private Double destinationHeading;
	private double heading;
	
	private boolean collisionX;
	private boolean collisionY;
	private int locationId;
	private double[] floatLocation;
	boolean rotated;

	public void reset() {
		setAngularVelocity(0);
		setDestinationHeading(null);
		setHeading(0);
		setCollisionX(false);
		setCollisionY(false);
		setLocationId(-1);
		floatLocation = new double[] { -1, -1 };
		rotated = true;
	}
	
	public boolean rotated() {
		return rotated;
	}
	
	public void resetRotated() {
		rotated = false;
	}

	public void setAngularVelocity(double angVel) {
		if (Double.compare(angVel, 0) != 0) {
			rotated = true;
		}
		this.angVel = angVel;
	}

	public double getAngularVelocity() {
		return angVel;
	}

	public boolean hasDestinationHeading() {
		return destinationHeading != null;
	}
	
	public void setDestinationHeading(Double destinationHeading) {
		this.destinationHeading = destinationHeading;
	}

	public Double getDestinationHeading() {
		assert destinationHeading != null;
		return destinationHeading;
	}
	
	public void resetDestinationHeading() {
		this.destinationHeading = null;
	}

	public void setHeading(double heading) {
		heading = Simulation.mod2pi(heading);
		if (Double.compare(heading, this.heading) != 0) {
			rotated = true;
		}
		this.heading = heading;
	}

	public double getHeading() {
		return heading;
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

	public void setLinearVelocity(double linVel) {
		// TODO: convert to x,y
		assert false;
	}

	public void setLocationId(int locationId) {
		this.locationId = locationId;
	}

	public int getLocationId() {
		return locationId;
	}
	
	public double[] getFloatLocation() {
		return Arrays.copyOf(floatLocation, floatLocation.length);
	}
	
	void setFloatLocation(double[] newFloatLocation) {
		assert newFloatLocation.length == floatLocation.length;
		floatLocation = Arrays.copyOf(newFloatLocation, newFloatLocation.length);
	}
	
	
	public double angleOff(double [] target) {
		double [] playerVector = getFloatLocation();
		
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
		playerVector[0] = Math.cos(getHeading());
		playerVector[1] = Math.sin(getHeading());
		
		double dotProduct = (targetVector[0] * playerVector[0]) + (targetVector[1] * playerVector[1]);
		double crossProduct = (targetVector[0] * playerVector[1]) - (targetVector[1] * playerVector[0]);
		
		// calculate inverse cosine of that for angle
		if (crossProduct < 0) {
			return Math.acos(dotProduct);
		}
		return Math.acos(dotProduct) * -1;
	}

	public void stop() {
		// TODO Auto-generated method stub
		assert false;
		
	}
}
