/**
 * 
 */
package edu.umich.soar.robot;

import org.apache.log4j.Logger;

import sml.Agent;
import sml.Identifier;

/**
 * @author voigtjr
 *
 * Emergency stop.
 */
final public class EStopCommand extends DDCCommand implements Command {
	private static final Logger logger = Logger.getLogger(EStopCommand.class);
	static final String NAME = "estop";

	static Command newInstance() {
		return new EStopCommand();
	}
	
	@Override
	public DifferentialDriveCommand getDDC() {
		return DifferentialDriveCommand.newEStopCommand();
	}

	@Override
	public boolean execute(Agent agent, Identifier command) {
		logger.debug(NAME + ":");
		CommandStatus.accepted.addStatus(command);
		CommandStatus.complete.addStatus(command);
		
		this.agent = agent;
		this.command = command;
		return true;
	}
}
