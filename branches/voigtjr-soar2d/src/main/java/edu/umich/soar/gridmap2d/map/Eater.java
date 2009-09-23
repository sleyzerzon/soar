package edu.umich.soar.gridmap2d.map;

import edu.umich.soar.gridmap2d.core.PlayerColor;
import edu.umich.soar.gridmap2d.core.Simulation;

public class Eater extends Player {	
	private EaterCommander commander;
	final private EaterState state;
	
	Eater(Simulation sim, String name, PlayerColor color, String initialFacing) {
		super(sim, name, color);
		state = new EaterState();
	}
	
	void setCommander(EaterCommander commander) {
		this.commander = commander;
	}
	
	EaterCommand nextCommand() {
		state.setLastCommand(commander.nextCommand());
		return state.getLastCommand();
	}
	
	@Override
	void reset() {
		state.reset();

		if (commander != null) {
			commander.reset();
		}
	}

	void shutdownCommander() {
		if (commander != null) {
			commander.shutdown();
		}
	}
	
	public EaterState getState() {
		// TODO: make private
		return state;
	}
}
