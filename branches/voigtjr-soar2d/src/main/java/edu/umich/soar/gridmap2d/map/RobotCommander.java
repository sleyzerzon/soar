package edu.umich.soar.gridmap2d.map;

import java.util.List;

import edu.umich.soar.robot.ReceiveMessagesInterface;

public interface RobotCommander extends Commander {
	public List<double[]> getWaypointList();
	public ReceiveMessagesInterface getReceiveMessagesInterface();
	public RobotCommand nextCommand();
}
