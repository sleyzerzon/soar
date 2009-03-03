/**
 * 
 */
package org.msoar.sps.control.o;

import org.apache.log4j.Logger;

import lcmtypes.pose_t;
import sml.Identifier;

class RemoveWaypointCommand implements Command {
	private static Logger logger = Logger.getLogger(RemoveWaypointCommand.class);
	
	public CommandStatus execute(Identifier command, pose_t pose, OutputLinkManager outputLinkManager) {
		String id = command.GetParameterValue("id");
		if (id == null) {
			logger.warn("No id on remove-waypoint command");
			return CommandStatus.error;
		}

		logger.debug(String.format("remove-waypoint: %16s", id));

		if (outputLinkManager.waypointsIL.remove(id) == false) {
			logger.warn("Unable to remove waypoint " + id + ", no such waypoint");
			return CommandStatus.error;
		}

		return CommandStatus.complete;
	}

	public boolean isInterruptable() {
		return false;
	}

	public boolean modifiesInput() {
		return false;
	}

	public void updateInput(SplinterInput input) {
		assert false;
	}
}