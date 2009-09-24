package edu.umich.soar.gridmap2d.map;

import java.util.Arrays;

public class EaterState {
	private final int[] location = new int[] { -1, -1 };
	private EaterCommand lastCommand;
	private final Points points = new Points();

	EaterState() {
	}
	
	void reset() {
		Arrays.fill(location, -1);
		lastCommand = null;
		points.reset();
	}
	
	void setLocation(int[] newLocation) {
		assert newLocation != null;
		assert newLocation.length == location.length;
		System.arraycopy(newLocation, 0, location, 0, location.length);
	}
	
	void setLastCommand(EaterCommand nextCommand) {
		lastCommand = nextCommand;
	}
	
	public int[] getLocation() {
		return Arrays.copyOf(location, location.length);
	}

	public EaterCommand getLastCommand() {
		return lastCommand;
	}
	
	public Points getPoints() {
		return points;
	}
	
}
