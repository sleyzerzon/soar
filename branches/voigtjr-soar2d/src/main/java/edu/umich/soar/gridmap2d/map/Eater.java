package edu.umich.soar.gridmap2d.map;


import edu.umich.soar.gridmap2d.core.Simulation;

public class Eater extends Player {	
	private EaterCommander commander;

	public Eater(Simulation sim, String playerId) {
		super(sim, playerId);
	}
	
	public void setCommander(EaterCommander commander) {
		this.commander = commander;
	}
	
	public EaterCommand getCommand() {
		EaterCommand command;
		
		command = commander.nextCommand();
		
		// the facing depends on the move
		if (command.isMove()) { 
			super.setFacing(command.getMoveDirection());
		}

		return command;
	}
	
	public void update(int[] newLocation, EatersMap eatersMap) {
		super.update(newLocation);
		
		if (commander != null) {
			commander.update(eatersMap);
		}
	}

	@Override
	public void reset() {
		super.reset();
		if (commander != null) {
			commander.reset();
		}
	}

	public void shutdownCommander() {
		if (commander != null) {
			commander.shutdown();
		}
	}
}
