/**
 * 
 */
package org.msoar.sps.control;

import org.apache.log4j.Logger;

import sml.Identifier;

final class DisableWaypointCommand implements Command {
	private static final Logger logger = Logger.getLogger(DisableWaypointCommand.class);
	static final String NAME = "disable-waypoint";

	public CommandStatus execute(InputLinkInterface inputLink, Identifier command, SplinterState splinter, OutputLinkManager outputLinkManager) {
		String id = command.GetParameterValue("id");
		if (id == null) {
			logger.warn("No id on disable-waypoint command");
			return CommandStatus.error;
		}

		logger.debug(String.format("disable-waypoint: %16s", id));

		if (inputLink.disableWaypoint(id) == false) {
			logger.warn("Unable to disable waypoint " + id + ", no such waypoint");
			return CommandStatus.error;
		}

		return CommandStatus.complete;
	}

	public boolean isInterruptable() {
		return false;
	}

	public boolean createsDDC() {
		return false;
	}

	public DifferentialDriveCommand getDDC() {
		throw new AssertionError();
	}
}