/**
 * 
 */
package org.msoar.sps.control.io.o;

import org.apache.log4j.Logger;

import lcmtypes.pose_t;
import sml.Identifier;

class RemoveWaypointCommand implements Command {
	private static Logger logger = Logger.getLogger(RemoveWaypointCommand.class);
	
	public SplinterInput execute(SplinterInput input, Identifier commandwme, pose_t pose, OutputLinkManager outputLinkManager) {
		String id = commandwme.GetParameterValue("id");
		if (id == null) {
			logger.warn("No id on remove-waypoint command");
			commandwme.AddStatusError();
			return input;
		}

		logger.debug(String.format("remove-waypoint: %16s", id));

		if (outputLinkManager.waypointsIL.remove(id) == false) {
			logger.warn("Unable to remove waypoint " + id + ", no such waypoint");
			commandwme.AddStatusError();
			return input;
		}

		commandwme.AddStatusComplete();
		return input;
	}
}