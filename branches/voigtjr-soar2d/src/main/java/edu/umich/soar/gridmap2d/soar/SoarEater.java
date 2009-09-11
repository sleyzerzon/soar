package edu.umich.soar.gridmap2d.soar;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.gridmap2d.core.Direction;
import edu.umich.soar.gridmap2d.core.Names;
import edu.umich.soar.gridmap2d.core.Simulation;
import edu.umich.soar.gridmap2d.map.Eater;
import edu.umich.soar.gridmap2d.map.EaterCommand;
import edu.umich.soar.gridmap2d.map.EaterCommander;
import edu.umich.soar.gridmap2d.map.EatersMap;

import sml.Agent;
import sml.Identifier;

public final class SoarEater implements EaterCommander, SoarAgent {
	private static Log logger = LogFactory.getLog(SoarEater.class);

	private Eater player;
	
	private SoarEaterIL input;
	private Agent agent;
	private String[] shutdownCommands;
	boolean fragged = false;
	private final Simulation sim;
	private EaterCommand command;
	
	public SoarEater(Simulation sim, Eater player, Agent agent, int vision, String[] shutdownCommands) {
		this.sim = sim;
		this.player = player;
		this.agent = agent;
		agent.SetBlinkIfNoChange(false);
		
		this.shutdownCommands = shutdownCommands;
		
		input = new SoarEaterIL(agent, vision);
		input.create(player.getName(), player.getPoints());
	}
	
	@Override
	public EaterCommand nextCommand() {
		EaterCommand temp = command;
		command = null;
		return temp;
	}
	
	@Override
	public void reset() {
		if (agent == null) {
			return;
		}
		command = null;
		input.destroy();
		agent.InitSoar();
		input.create(player.getName(), player.getPoints());
	}

	@Override
	public void shutdown() {
		assert agent != null;
		if (shutdownCommands != null) { 
			// execute the pre-shutdown commands
			for (String command : shutdownCommands) {
				String result = player.getName() + ": result: " + agent.ExecuteCommandLine(command, true);
				logger.info(player.getName() + ": shutdown command: " + command);
				if (agent.HadError()) {
					sim.error("Soar Eater", result);
				} else {
					logger.info(player.getName() + ": result: " + result);
				}
			}
		}
	}

	@Override
	public void updateSoarInput() {
		input.update(player.getMoved(), player.getLocation(), (EatersMap)sim.getMap(), player.getPoints());
	}

	@Override
	public void processSoarOuput() {
		// if there was no command issued, that is kind of strange
		if (agent.GetNumberCommands() == 0) {
			logger.debug(player.getName() + " issued no command.");
			command = EaterCommand.NULL;
			return;
		}

		// go through the commands
		// see move info for details
		EaterCommand.Builder builder = new EaterCommand.Builder();
		for (int i = 0; i < agent.GetNumberCommands(); ++i) {
			Identifier commandId = agent.GetCommand(i);
			String commandName = commandId.GetAttribute();
			
			if (commandName.equalsIgnoreCase(Names.kMoveID)) {
				String direction = commandId.GetParameterValue(Names.kDirectionID);
				if (direction != null) {
					if (direction.equals(Names.kNone)) {
						// legal wait
						commandId.AddStatusComplete();
						continue;
					} else {
						builder.move(Direction.parse(direction));
						commandId.AddStatusComplete();
						continue;
					}
				}
				
			} else if (commandName.equalsIgnoreCase(Names.kJumpID)) {
				String direction = commandId.GetParameterValue(Names.kDirectionID);
				if (direction != null) {
					builder.move(Direction.parse(direction));
					builder.jump();
					commandId.AddStatusComplete();
					continue;
				}

			} else if (commandName.equalsIgnoreCase(Names.kStopSimID)) {
				builder.stopSim();
				commandId.AddStatusComplete();
				continue;
				
			} else if (commandName.equalsIgnoreCase(Names.kOpenID)) {
				builder.open();
				commandId.AddStatusComplete();
				continue;
				
			} else if (commandName.equalsIgnoreCase(Names.kDontEatID)) {
				builder.dontEat();
				commandId.AddStatusComplete();
				continue;
				
			} else {
				logger.warn("Unknown command: " + commandName);
				continue;
			}
			
			logger.warn("Improperly formatted command: " + commandName);
		}

		agent.ClearOutputLinkChanges();

		command = builder.build();
	}
}
