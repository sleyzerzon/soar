/**
 * 
 */
package edu.umich.soar.sps.control;

import edu.umich.soar.waypoints.OffsetPose;
import sml.Agent;
import sml.Identifier;

interface Command {
	/**
	 * @param inputLink Input link for waypoint stuff.
	 * @param agent Soar agent.
	 * @param command The command wme to store.
	 * @param splinter Current state.
	 * @param outputLinkManager The calling instance.
	 * @return true on success
	 */
	boolean execute(InputLinkInterface inputLink, Agent agent, Identifier command, OffsetPose splinter, OutputLinkManager outputLinkManager);
	/**
	 * @param splinter Current state.
	 * @return true if command is done executing.
	 */
	boolean update(OffsetPose splinter);
	/**
	 * Command is interrupted.
	 */
	void interrupt();
	/**
	 * @return Returns command wme.
	 */
	Identifier wme();
	/**
	 * @return true if the command generates a DDC.
	 */
	boolean createsDDC();
	/**
	 * @return Generated DDC.
	 */
	DifferentialDriveCommand getDDC();
}