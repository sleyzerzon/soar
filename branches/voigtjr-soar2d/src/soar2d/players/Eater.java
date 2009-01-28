package soar2d.players;

import soar2d.Soar2D;
import soar2d.map.EatersMap;

public class Eater extends Player {	
	private CommandInfo command;
	private EaterCommander commander;

	public Eater(String playerId) throws Exception {
		super(playerId);
	}
	
	public void setCommander(EaterCommander commander) {
		this.commander = commander;
	}
	
	public CommandInfo getCommand() throws Exception {
		if (commander != null) {
			command = commander.getCommand();
		} else {
			command = Soar2D.simulation.getHumanCommand(this);
		}
		
		// the facing depends on the move
		if (command.move) { 
			super.setFacing(command.moveDirection);
		}

		return command;
	}
	
	public void update(int[] newLocation, EatersMap eatersMap) throws Exception {
		super.update(newLocation);
		if (commander != null) {
			commander.update(eatersMap);
		}
		resetPointsChanged();
	}

	@Override
	public void reset() throws Exception {
		super.reset();
		if (commander != null) {
			commander.reset();
		}
	}

	public void shutdownCommander() throws Exception {
		if (commander != null) {
			commander.shutdown();
		}
	}
}
