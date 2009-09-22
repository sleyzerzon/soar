package edu.umich.soar.gridmap2d.map;

import java.util.List;

import edu.umich.soar.gridmap2d.core.PlayerColor;
import edu.umich.soar.gridmap2d.core.Simulation;
import edu.umich.soar.robot.ReceiveMessagesInterface;

public class Robot extends Player {
	private RobotCommander commander;
	private RobotState state;
	
	public Robot(Simulation sim, String name, PlayerColor color) {
		super(sim, name, color);
		
		state = new RobotState();
		state.reset();
	}
	
	public RobotState getState() {
		return state;
	}

	public void setCommander(RobotCommander commander) {
		this.commander = commander;
	}
	
	public RobotCommand getCommand() {
		return commander.nextCommand();
	}
	
	@Override
	public void reset() {
		super.reset();
		
		if (state != null) {
			state.reset();
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

	public List<double[]> getWaypointList() {
		return commander.getWaypointList();
	}

	public ReceiveMessagesInterface getReceiveMessagesInterface() {
		return commander.getReceiveMessagesInterface();
	}
}
