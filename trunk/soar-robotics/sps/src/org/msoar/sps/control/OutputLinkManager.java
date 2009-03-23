package org.msoar.sps.control;

import java.util.HashMap;

import org.apache.log4j.Logger;

import sml.Agent;
import sml.Identifier;

/**
 * @author voigtjr Soar output-link management. Creates input for splinter and
 *         other parts of the system.
 */
final class OutputLinkManager {
	private static final Logger logger = Logger.getLogger(OutputLinkManager.class);

	private final SplinterState splinter;
	private final Agent agent;
	private final InputLinkInterface inputLink;
	private final HashMap<String, Command> commands = new HashMap<String, Command>();

	boolean useFloatYawWmes = true;
	
	private Command runningCommand;

	OutputLinkManager(Agent agent, InputLinkInterface inputLink, SplinterState splinter) {
		this.splinter = splinter;
		this.agent = agent;
		this.inputLink = inputLink;

		commands.put(MotorCommand.NAME, new MotorCommand());
		commands.put(SetVelocityCommand.NAME, new SetVelocityCommand());
		commands.put(SetHeadingCommand.NAME, new SetHeadingCommand());
		commands.put(StopCommand.NAME, new StopCommand());
		commands.put(EStopCommand.NAME, new EStopCommand());
		commands.put(AddWaypointCommand.NAME, new AddWaypointCommand());
		commands.put(RemoveWaypointCommand.NAME, new RemoveWaypointCommand());
		commands.put(EnableWaypointCommand.NAME, new EnableWaypointCommand());
		commands.put(DisableWaypointCommand.NAME, new DisableWaypointCommand());
		commands.put(BroadcastMessageCommand.NAME, new BroadcastMessageCommand());
		commands.put(RemoveMessageCommand.NAME, new RemoveMessageCommand());
		commands.put(ClearMessagesCommand.NAME, new ClearMessagesCommand());
		commands.put(ConfigureCommand.NAME, new ConfigureCommand());
		commands.put(SetOffsetCommand.NAME, new SetOffsetCommand());
	}

	boolean getUseFloatYawWmes() {
		return useFloatYawWmes;
	}

	DifferentialDriveCommand update() {
		// TODO: update status of running command
		
		// process output
		DifferentialDriveCommand ddc = null;
		for (int i = 0; i < agent.GetNumberCommands(); ++i) {
			Identifier commandWme = agent.GetCommand(i);

			// is it already running?
			synchronized (this) {
				if (runningCommand != null && runningCommand.wme().GetTimeTag() == commandWme.GetTimeTag()) {
					continue;
				}
			}
			
			String commandName = commandWme.GetAttribute();
			logger.debug(commandName + " " + commandWme.GetTimeTag());

			Command commandObject = commands.get(commandName);
			if (commandObject == null) {
				logger.warn("Unknown command: " + commandName);
				CommandStatus.error.addStatus(agent, commandWme);
				continue;
			}
			
			if (commandObject.createsDDC()) {
				if (ddc != null) {
					logger.warn("Ignoring command " + commandName + " because already have " + ddc);
					CommandStatus.error.addStatus(agent, commandWme);
					continue;
				}
				runningCommand.interrupt();
				runningCommand = null;
			}
			
			if (!commandObject.execute(inputLink, agent, commandWme, splinter, this)) {
				if (commandObject.createsDDC()) {
					logger.warn("Error with new drive command, commanding estop.");
					ddc = DifferentialDriveCommand.newEStopCommand();
				}
				continue;
			}

			if (commandObject.createsDDC()) {
				ddc = commandObject.getDDC();
				runningCommand = commandObject;
				logger.debug(ddc);
			}
		}
		
		if (runningCommand.update(splinter)) {
			runningCommand = null;
		}
		
		return ddc;
	}
}
