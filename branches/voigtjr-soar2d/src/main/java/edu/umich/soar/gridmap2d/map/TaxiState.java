package edu.umich.soar.gridmap2d.map;

import java.util.Arrays;

import edu.umich.soar.gridmap2d.core.Simulation;

public class TaxiState {
	
	private int fuel;
	private int fuelStartMin;
	private int fuelStartMax;
	private int refuel;
	private boolean disableFuel;
	private int[] location;
	private final Points points = new Points();

	TaxiState(int fuelStartMin, int fuelStartMax, int refuel, boolean disableFuel) {
		this.fuelStartMin = fuelStartMin;
		this.fuelStartMax = fuelStartMax;
		this.refuel = refuel;
		this.disableFuel = disableFuel;
	}
	
	void reset() {
		fuel = Simulation.random.nextInt(1 + fuelStartMax - fuelStartMin);
		fuel += fuelStartMin;
		location = new int[] { -1, -1 };
		points.reset();
	}
	
	void setLocation(int[] newLocation) {
		this.location = Arrays.copyOf(newLocation, newLocation.length);
	}
	
	public int[] getLocation() {
		return Arrays.copyOf(location, location.length);
	}
	
	public Points getPoints() {
		return points;
	}
	
	public boolean isDisableFuel() {
		return disableFuel;
	}
	
	public int getFuel() {
		return fuel;
	}

	int getRefuel() {
		return refuel;
	}

	void setFuel(int fuel) {
		this.fuel = fuel;
	}
	
}
