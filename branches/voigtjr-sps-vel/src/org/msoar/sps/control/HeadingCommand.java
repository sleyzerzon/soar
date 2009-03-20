/**
 * 
 */
package org.msoar.sps.control;

import org.apache.log4j.Logger;

import sml.Identifier;

final class HeadingCommand implements Command {
	private static final Logger logger = Logger.getLogger(HeadingCommand.class);
	static final String NAME = "rotate-to";

	double yaw;
	
	public CommandStatus execute(InputLinkInterface inputLink, Identifier command, SplinterState splinter, OutputLinkManager outputLinkManager) {
		try {
			yaw = Double.parseDouble(command.GetParameterValue("yaw"));
		} catch (NullPointerException ex) {
			logger.warn("No yaw on heading command");
			return CommandStatus.error;
		} catch (NumberFormatException e) {
			logger.warn("Unable to parse yaw: " + command.GetParameterValue("yaw"));
			return CommandStatus.error;
		}
		yaw = Math.toRadians(yaw);

		logger.debug(String.format("heading: %10.3f", yaw));
		
		return CommandStatus.accepted;
	}
	
	public boolean isInterruptable() {
		return true;
	}

	public boolean createsDDC() {
		return true;
	}

	public DifferentialDriveCommand getDDC() {
		return DifferentialDriveCommand.newHeadingCommand(yaw);
	}
}