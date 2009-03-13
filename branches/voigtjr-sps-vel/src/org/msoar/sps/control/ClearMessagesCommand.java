/**
 * 
 */
package org.msoar.sps.control;

import sml.Identifier;

final class ClearMessagesCommand implements Command {
	public CommandStatus execute(InputLinkInterface inputLink, Identifier command, SplinterModel splinter, OutputLinkManager outputLinkManager) {
		inputLink.clearMessages();
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