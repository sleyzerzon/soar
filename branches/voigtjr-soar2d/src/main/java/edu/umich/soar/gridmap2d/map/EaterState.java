package edu.umich.soar.gridmap2d.map;

import java.util.Arrays;

public class EaterState {
	public static class Cell {
		
	}
	
	private Cell[][] cells;		// what I can see
	private int[] location;
	private EaterCommand lastCommand;
	private final Points points = new Points();

	EaterState() {
	}
	
	public void reset() {
		location = new int[] { -1, -1 };
		lastCommand = null;
		points.reset();
	}
	
	public EaterCommand getLastCommand() {
		return lastCommand;
	}
	
	void setLocation(int[] newLocation) {
		this.location = Arrays.copyOf(newLocation, newLocation.length);
	}
	
	public int[] getLocation() {
		return Arrays.copyOf(location, location.length);
	}

	void setLastCommand(EaterCommand nextCommand) {
		lastCommand = nextCommand;
	}
	
	public Points getPoints() {
		return points;
	}
}
