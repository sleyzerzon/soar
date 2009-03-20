/**
 * 
 */
package org.msoar.sps.control;

import org.apache.log4j.Logger;

import sml.Identifier;

final class RemoveMessageCommand implements Command {
	private static final Logger logger = Logger.getLogger(RemoveMessageCommand.class);
	static final String NAME = "remove-message";

	public CommandStatus execute(InputLinkInterface inputLink, Identifier command, SplinterState splinter, OutputLinkManager outputLinkManager) {
		int id = -1;
		try {
			id = Integer.parseInt(command.GetParameterValue("id"));
		} catch (NullPointerException ignored) {
			logger.warn(NAME + ": No id on command");
			return CommandStatus.error;
		} catch (NumberFormatException e) {
			logger.warn(NAME + ": Unable to parse id: " + command.GetParameterValue("id"));
			return CommandStatus.error;
		}

		logger.debug(String.format(NAME + ": %d", id));
		
		if (inputLink.removeMessage(id) == false) {
			logger.warn(NAME + ": Unable to remove message " + id + ", no such message");
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