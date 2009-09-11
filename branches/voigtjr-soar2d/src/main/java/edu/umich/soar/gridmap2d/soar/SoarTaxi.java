package edu.umich.soar.gridmap2d.soar;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.gridmap2d.core.Direction;
import edu.umich.soar.gridmap2d.core.Names;
import edu.umich.soar.gridmap2d.core.Simulation;
import edu.umich.soar.gridmap2d.map.Taxi;
import edu.umich.soar.gridmap2d.map.TaxiCommand;
import edu.umich.soar.gridmap2d.map.TaxiCommander;
import edu.umich.soar.gridmap2d.map.TaxiMap;

import sml.Agent;
import sml.Identifier;

public class SoarTaxi implements TaxiCommander, SoarAgent {
	private static Log logger = LogFactory.getLog(SoarTaxi.class);

	private Taxi player;
	private Agent agent;
	private String [] shutdownCommands;
	private SoarTaxiIL input;
	private final Simulation sim;
	private TaxiCommand command;
	
	public SoarTaxi(Simulation sim, Taxi taxi, Agent agent, String[] shutdown_commands) {
		this.player = taxi;
		this.agent = agent;
		this.sim = sim;
		this.shutdownCommands = shutdown_commands;
		
		agent.SetBlinkIfNoChange(false);
		
		input = new SoarTaxiIL(agent);
		input.create();
	}

	void update() {
	}

	@Override
	public TaxiCommand nextCommand() {
		TaxiCommand temp = command;
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
			
		input.create();
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
					sim.error("Soar Taxi", result);
				} else {
					logger.info(player.getName() + ": result: " + result);
				}
			}
		}
	}

	@Override
	public void processSoarOuput() {
		// if there was no command issued, that is kind of strange
		if (agent.GetNumberCommands() == 0) {
			if (logger.isDebugEnabled()) {
				logger.debug(player.getName() + " issued no command.");
			}
			command = TaxiCommand.NULL;
			return;
		}

		// go through the commands
		// see move info for details
		TaxiCommand.Builder builder = new TaxiCommand.Builder();
		if (agent.GetNumberCommands() > 1) {
			logger.debug(player.getName() + ": " + agent.GetNumberCommands() 
					+ " commands detected, all but the first will be ignored");
		}
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
				
			} else if (commandName.equalsIgnoreCase(Names.kStopSimID)) {
				builder.stopSim();
				commandId.AddStatusComplete();
				continue;
				
			} else if (commandName.equalsIgnoreCase(Names.kPickUpID)) {
				builder.pickup();
				commandId.AddStatusComplete();
				continue;
				
			} else if (commandName.equalsIgnoreCase(Names.kPutDownID)) {
				builder.putdown();
				commandId.AddStatusComplete();
				continue;
				
			} else if (commandName.equalsIgnoreCase(Names.kFillUpID)) {
				builder.fillup();
				commandId.AddStatusComplete();
				continue;
				
			} else {
				logger.warn("Unknown command: " + commandName);
				commandId.AddStatusError();
				continue;
			}
			
			logger.warn("Improperly formatted command: " + commandName);
			commandId.AddStatusError();
		}
		agent.ClearOutputLinkChanges();
		command = builder.build();
	}

	@Override
	public void updateSoarInput() {
		input.update(player.getMoved(), player.getLocation(), (TaxiMap)sim.getMap(), player.getPointsDelta(), player.getFuel());
	}
}
