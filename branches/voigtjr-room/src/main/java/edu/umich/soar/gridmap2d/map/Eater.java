package edu.umich.soar.gridmap2d.map;

import edu.umich.soar.gridmap2d.core.PlayerColor;

public class Eater extends Player {	
	private EaterCommander commander;
	final private EaterState state;
	
	Eater(String name, PlayerColor color) {
		super(name, color);
		state = new EaterState();
	}
	
	void setCommander(EaterCommander commander) {
		this.commander = commander;
	}
	
	EaterCommand nextCommand() {
		state.setLastCommand(commander.nextCommand());
		return state.getLastCommand();
	}
	
	public EaterState getState() {
		return state;
	}
	
	@Override
	void reset() {
		state.reset();

		if (commander != null) {
			commander.reset();
		}
	}

	@Override
	void shutdown() {
		if (commander != null) {
			commander.shutdown();
		}
	}
	
}
