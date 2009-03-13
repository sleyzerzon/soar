/**
 * 
 */
package org.msoar.sps.control;

import org.apache.log4j.Logger;

import sml.Identifier;

final class BroadcastMessageCommand implements Command {
	private static final Logger logger = Logger.getLogger(BroadcastMessageCommand.class);
	
	public CommandStatus execute(InputLinkInterface inputLink, Identifier command, SplinterModel splinter, OutputLinkManager outputLinkManager) {
		logger.warn("broadcast-message command not implemented, ignoring");
		return CommandStatus.error;
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