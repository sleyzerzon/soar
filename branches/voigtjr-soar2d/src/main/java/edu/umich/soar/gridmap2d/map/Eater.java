package edu.umich.soar.gridmap2d.map;


import edu.umich.soar.gridmap2d.core.Direction;
import edu.umich.soar.gridmap2d.core.PlayerColor;
import edu.umich.soar.gridmap2d.core.Simulation;

public class Eater extends Player {	
	private EaterCommander commander;
	private Direction initialFacing;	
	private Direction facing;	// what direction I'm currently facing

	public Eater(Simulation sim, String name, PlayerColor color, String initialFacing) {
		super(sim, name, color);
		if (initialFacing != null) {
			this.initialFacing = Direction.parse(initialFacing);
			setFacing(this.initialFacing);
		} else {
			setFacing(Direction.values()[Simulation.random.nextInt(4) + 1]);
		}
	}
	
	public Direction getFacing() {
		return facing;
	}
	
	public void setFacing(Direction facing) {
		this.facing = facing;
	}
	
	public void setCommander(EaterCommander commander) {
		this.commander = commander;
	}
	
	public EaterCommand getCommand() {
		EaterCommand command;
		
		command = commander.nextCommand();
		
		// the facing depends on the move
		if (command != null && command.isMove()) { 
			setFacing(command.getMoveDirection());
		}

		return command;
	}
	
	@Override
	public void reset() {
		super.reset();

		if (initialFacing != null) {
			setFacing(initialFacing);
		} else {
			setFacing(Direction.values()[Simulation.random.nextInt(4) + 1]);
		}

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
