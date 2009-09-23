package edu.umich.soar.gridmap2d.map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.gridmap2d.core.PlayerColor;
import edu.umich.soar.gridmap2d.core.Simulation;

public class Taxi extends Player {	
	private static Log logger = LogFactory.getLog(Taxi.class);

	private TaxiCommander commander;
	private TaxiState state;

	public Taxi(Simulation sim, String name, PlayerColor color, int fuelStartMin, int fuelStartMax, int refuel, boolean disableFuel) {
		super(sim, name, color);
		
		state = new TaxiState(fuelStartMin, fuelStartMax, refuel, disableFuel);
		
		reset();
	}
	
	public TaxiState getState() {
		return state;
	}
	
	public void setCommander(TaxiCommander commander) {
		this.commander = commander;
	}
	
	public TaxiCommand getCommand() {
		return commander.nextCommand();
	}
	
	@Override
	public void reset() {
		state.reset();
		
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
		if (state.isDisableFuel()) {
			logger.info("fuel consuption disabled");
			return;
		}
		logger.info("fuel: " + Integer.toString(state.getFuel()) + " -> " + Integer.toString(state.getFuel()-1));
		state.setFuel(state.getFuel() - 1);
	}
	
	public int getFuel() {
		return state.getFuel();
	}

	public void fillUp() {
		logger.info("fuel: " + Integer.toString(state.getFuel()) + " -> " + state.getRefuel() + " (fillup)");
		state.setFuel(state.getRefuel());
	}
}
