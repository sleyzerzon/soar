/**
 * 
 */
package org.msoar.sps.control;

import org.apache.log4j.Logger;

import sml.Identifier;

class MoveCommand implements Command {
	private static final Logger logger = Logger.getLogger(MoveCommand.class);
	
	private double linearVelocity;
	
	public CommandStatus execute(InputLinkInterface inputLink, Identifier command, SplinterState splinter, OutputLinkManager outputLinkManager) {
		try {
			linearVelocity = Double.parseDouble(command.GetParameterValue("velocity"));
		} catch (NullPointerException ex) {
			logger.warn("No velocity on move command");
			return CommandStatus.error;
		} catch (NumberFormatException e) {
			logger.warn("Unable to parse velocity: " + command.GetParameterValue("velocity"));
			return CommandStatus.error;
		}

		logger.debug(String.format("move: %10.3f", linearVelocity));

		return CommandStatus.executing;
	}

	public boolean isInterruptable() {
		return false;
	}

	public boolean createsDDC() {
		return true;
	}

	public DifferentialDriveCommand getDDC() {
		return DifferentialDriveCommand.newLinearVelocityCommand(linearVelocity);
	}
}