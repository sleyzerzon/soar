/**
 * 
 */
package edu.umich.soar.robot;

import jmat.LinAlg;
import lcmtypes.pose_t;

import org.apache.log4j.Logger;

import sml.Agent;
import sml.Identifier;

/**
 * @author voigtjr
 *
 * Gracefully stop movement.
 * 
 * Returns accepted. Not interruptible. Creates DDC.
 */
final public class StopCommand extends DDCCommand implements Command {
	private static final Logger logger = Logger.getLogger(StopCommand.class);
	private static final double TOLERANCE = 0.01; // meters per second
	static final String NAME = "stop";

	static Command newInstance(OffsetPose opose) {
		return new StopCommand(opose);
	}
	
	public StopCommand(OffsetPose opose) {
		this.opose = opose;
	}

	private final OffsetPose opose;
	private CommandStatus status;
	
	@Override
	public DifferentialDriveCommand getDDC() {
		return DifferentialDriveCommand.newVelocityCommand(0, 0);
	}

	@Override
	public boolean execute(Agent agent, Identifier command) {
		if (this.agent != null || this.command != null || this.status != null) {
			throw new IllegalStateException();
		}

		logger.debug(NAME + ":");
		CommandStatus.accepted.addStatus(command);
		status = CommandStatus.accepted;

		this.agent = agent;
		this.command = command;
		return true;
	}

	@Override
	public void interrupt() {
		if (agent == null || command == null || status == null) {
			throw new IllegalStateException();
		}
		
		CommandStatus.interrupted.addStatus(command);
		agent = null;
		command = null;
		status = null;
	}

	@Override
	public boolean update() {
		if (agent != null || command != null || status != null) {
			throw new IllegalStateException();
		}
		
		pose_t pose = opose.getPose();
		
		if (Double.compare(LinAlg.magnitude(pose.vel), TOLERANCE) < 0) {
			CommandStatus.complete.addStatus(command);
			agent = null;
			command = null;
			status = null;
			return true; // complete
		} else if (status == CommandStatus.accepted) {
			CommandStatus.executing.addStatus(command);
			status = CommandStatus.executing;
		}
		return false; // executing
	}
}
