/**
 * 
 */
package org.msoar.sps.control;

import org.apache.log4j.Logger;

import sml.Identifier;

final class RotateCommand implements Command {
	private static final Logger logger = Logger.getLogger(RotateCommand.class);
	static final String NAME = "rotate";

	private double angularVelocity;

	public CommandStatus execute(InputLinkInterface inputLink,
			Identifier command, SplinterState splinter, OutputLinkManager outputLinkManager) {
		
		try {
			angularVelocity = Double
					.parseDouble(command.GetParameterValue("velocity"));
		} catch (NullPointerException ex) {
			logger.warn("No velocity on rotate command");
			return CommandStatus.error;
		} catch (NumberFormatException e) {
			logger.warn("Unable to parse velocity: "
					+ command.GetParameterValue("velocity"));
			return CommandStatus.error;
		}

		logger.debug(String.format("rotate: %10.3f", angularVelocity));

		return CommandStatus.executing;
	}

	public boolean isInterruptable() {
		return false;
	}

	public boolean createsDDC() {
		return true;
	}

	public DifferentialDriveCommand getDDC() {
		return DifferentialDriveCommand.newAngularVelocityCommand(angularVelocity);
	}
}