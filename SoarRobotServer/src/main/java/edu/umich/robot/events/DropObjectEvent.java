package edu.umich.robot.events;

import edu.umich.robot.Robot;

public class DropObjectEvent implements AbstractProgramEvent
{
	private Robot robot;
	private int id; // the id of the object being dropped.

	public DropObjectEvent(Robot robot, int id) {
		this.robot = robot;
		this.id = id;
	}
    
    public Robot getRobot() {
        return robot;
    }

	public int getObjectID() {
		return id;
	}
    
}
