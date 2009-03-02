/**
 * 
 */
package org.msoar.sps.control.io.o;


import lcmtypes.pose_t;
import sml.Identifier;

interface Command {
	public SplinterInput execute(SplinterInput input, Identifier commandwme, pose_t pose, OutputLinkManager outputLinkManager);
}