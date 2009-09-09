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

public class SoarTaxi implements TaxiCommander {
	private static Log logger = LogFactory.getLog(SoarTaxi.class);

	private Taxi player;
	private Agent agent;
	private String [] shutdownCommands;
	private SoarTaxiIL input;
	private final Simulation sim;

	public SoarTaxi(Simulation sim, Taxi taxi, Agent agent, String[] shutdown_commands) {
		this.player = taxi;
		this.agent = agent;
		this.sim = sim;
		this.shutdownCommands = shutdown_commands;
		
		agent.SetBlinkIfNoChange(false);
		
		input = new SoarTaxiIL(agent);
		input.create();

		if (!agent.Commit()) {
			sim.error("Soar Taxi", Names.Errors.commitFail + taxi.getName());
		}
	}

	@Override
	public void update(TaxiMap taxiMap) {
		input.update(player.getMoved(), player.getLocation(), taxiMap, player.getPointsDelta(), player.getFuel());
		
		if (!agent.Commit()) {
			sim.error("Soar Taxi", Names.Errors.commitFail + player.getName());
			sim.stop();
		}
	}

	@Override
	public TaxiCommand nextCommand() {
		// if there was no command issued, that is kind of strange
		if (agent.GetNumberCommands() == 0) {
			if (logger.isDebugEnabled()) {
				logger.debug(player.getName() + " issued no command.");
			}
			return TaxiCommand.NULL;
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
		
		if (!agent.Commit()) {
			sim.error("Soar Taxi", Names.Errors.commitFail + player.getName());
			sim.stop();
		}
		
		return builder.build();
	}

	@Override
	public void reset() {
		if (agent == null) {
			return;
		}
		
		input.destroy();

		if (!agent.Commit()) {
			sim.error("Soar Taxi", Names.Errors.commitFail + player.getName());
			sim.stop();
		}

		agent.InitSoar();
			
		input.create();
			 
		if (!agent.Commit()) {
			sim.error("Soar Taxi", Names.Errors.commitFail + player.getName());
			sim.stop();
		}

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
}
