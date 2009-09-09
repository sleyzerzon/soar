package edu.umich.soar.gridmap2d.map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.gridmap2d.core.Simulation;

public class Taxi extends Player {	
	private static Log logger = LogFactory.getLog(Taxi.class);

	private TaxiCommander commander;

	private int fuel;
	private int fuelStartMin;
	private int fuelStartMax;
	private int refuel;
	private boolean disableFuel;

	public Taxi(Simulation sim, String playerId, int fuelStartMin, int fuelStartMax, int refuel, boolean disableFuel) {
		super(sim, playerId);
		
		this.fuelStartMin = fuelStartMin;
		this.fuelStartMax = fuelStartMax;
		this.refuel = refuel;
		this.disableFuel = disableFuel;
		
		reset();
	}
	
	public void setCommander(TaxiCommander commander) {
		this.commander = commander;
	}
	
	public TaxiCommand getCommand() {
		TaxiCommand command;
//		if (commander != null) {
			command = commander.nextCommand();
//		} else {
//			command = Gridmap2D.control.getHumanCommand(this);
//		}
		
		return command;
	}
	
	public void update(int[] newLocation, TaxiMap taxiMap) {
		super.update(newLocation);
		if (commander != null) {
			commander.update(taxiMap);
		}
	}
	
	@Override
	public void reset() {
		super.reset();

		fuel = Simulation.random.nextInt(1 + fuelStartMax - fuelStartMin);
		fuel += fuelStartMin;
		
		if (commander != null) {
			commander.reset();
		}
	}

	public void shutdownCommander() {
		if (commander != null) {
			commander.shutdown();
		}
	}
	
	public void consumeFuel() {
		if (disableFuel) {
			logger.info("fuel consuption disabled");
			return;
		}
		logger.info("fuel: " + Integer.toString(fuel) + " -> " + Integer.toString(fuel-1));
		fuel -= 1;
	}
	
	public int getFuel() {
		return fuel;
	}

	public void fillUp() {
		logger.info("fuel: " + Integer.toString(fuel) + " -> " + refuel + " (fillup)");
		fuel = refuel;
	}
}
