package edu.umich.soar.gridmap2d.map;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.gridmap2d.config.PlayerConfig;
import edu.umich.soar.gridmap2d.config.TaxiConfig;
import edu.umich.soar.gridmap2d.core.Direction;
import edu.umich.soar.gridmap2d.core.Simulation;

public class TaxiWorld implements World {
	private static Log logger = LogFactory.getLog(TaxiWorld.class);

	private TaxiMap map;
	private PlayersManager<Taxi, TaxiCommand> players = new PlayersManager<Taxi, TaxiCommand>();
	private List<String> stopMessages = new ArrayList<String>();
	private int fuelStartMin;
	private int fuelStartMax;
	private int refuel;
	private boolean disableFuel;
	private final Simulation sim;
	
	public TaxiWorld(Simulation sim) {
		TaxiConfig tc = sim.getConfig().taxiConfig();
		this.fuelStartMin = tc.fuel_starting_minimum;
		this.fuelStartMax = tc.fuel_starting_maximum;
		this.refuel = tc.fuel_maximum;
		this.disableFuel = tc.disable_fuel;
		this.sim = sim;
	}
	
	@Override
	public void setAndResetMap(String mapPath) {
		TaxiMap newMap = TaxiMap.generateInstance(mapPath);
		if (newMap == null) {
			return;
		}
		map = newMap;
		resetState();
	}
	
	private void resetState() {
		stopMessages.clear();
		resetPlayers();
	}
	
	/**
	 * @throws IllegalStateException If there are no available locations for the player to spawn
	 */
	private void resetPlayers() {
		if (players.numberOfPlayers() == 0) {
			return;
		}
		
		for (Taxi taxi : players.getAll()) {
			// find a suitable starting location
			int [] location = WorldUtil.getStartingLocation(map, players.getInitialLocation(taxi));
			if (location == null) {
				throw new IllegalStateException("no empty locations available for spawn");
			}
			players.setLocation(taxi, location);

			// put the player in it
			map.getCell(location).addPlayer(taxi);

			taxi.reset();
		}
		
		setLocations();
	}

	private void setLocations() {
		for (Taxi taxi : players.getAll()) {
			taxi.setLocation(players.getLocation(taxi));
		}
	}

	private void moveTaxis() {
		for (Taxi taxi : players.getAll()) {
			TaxiCommand command = players.getCommand(taxi);			

			int [] location = players.getLocation(taxi);
			if (command.isMove()) {
				// Calculate new location
				int [] newLocation = Arrays.copyOf(location, location.length);
				Direction.translate(newLocation, command.getMoveDirection());
				
				// Verify legal move and commit move
				if (map.isInBounds(newLocation) && map.exitable(location, command.getMoveDirection())) {
					taxi.consumeFuel();
					if (taxi.getFuel() < 0) {
						taxi.adjustPoints(-20, "fuel fell below zero");
					} else {
						// remove from cell
						map.getCell(newLocation).clearPlayers();
						players.setLocation(taxi, newLocation);
						
						map.getCell(newLocation).addPlayer(taxi);
						taxi.adjustPoints(-1, "legal move");
					}
				} else {
					taxi.adjustPoints(-1, "illegal move");
				}
				
			} else if (command.isPickup()) {
				if (map.pickUp(location)) {
					taxi.adjustPoints(-1, "legal pickup");
				} else {
					taxi.adjustPoints(-10, "illegal pickup");
				}
				
			} else if (command.isPutdown()) {
				if (map.putDown(location))  
				{
					if (map.isCorrectPassengerDestination(location)) {
						map.deliverPassenger();
						taxi.adjustPoints(20, "successful delivery");
					} else {
						taxi.adjustPoints(-10, "incorrect destination");
					}
				} else {
					taxi.adjustPoints(-10, "illegal putdown");
				}
				
			} else if (command.isFillup()) {
				if (map.isFuel(location)) {
					taxi.fillUp();
					taxi.adjustPoints(-1, "legal fillup");
				} else {
					taxi.adjustPoints(-10, "illegal fillup");
				}
			}
		}
	}
	
	@Override
	public void update(int worldCount) {
		WorldUtil.checkNumPlayers(sim, players.numberOfPlayers());

		// Collect input
		for (Taxi taxi : players.getAll()) {
			taxi.resetPointsChanged();
			TaxiCommand command = taxi.getCommand();
			if (command == null) {
				sim.stop();
				return;
			}
			players.setCommand(taxi, command);
			WorldUtil.checkStopSim(sim, stopMessages, command.isStopSim(), taxi);
		}

		moveTaxis();
		if (sim.isShuttingDown()) {
			return;
		}
		setLocations();
		
		checkFuelRemaining();
		checkPassengerDelivered();
		checkPassengerPickedUp();
		WorldUtil.checkMaxUpdates(sim, stopMessages, worldCount);
		WorldUtil.checkWinningScore(sim, stopMessages, players.getSortedScores());
		
		if (stopMessages.size() > 0) {
			sim.stop();
		}
	}
	
	private void checkFuelRemaining() {
		if (sim.getConfig().terminalsConfig().fuel_remaining) {
			for (Taxi taxi : players.getAll()) {
				if (taxi.getFuel() < 0) {
					stopMessages.add("Fuel is negative.");
				}
			}
		}
	}
	
	private void checkPassengerDelivered() {
		if (sim.getConfig().terminalsConfig().passenger_delivered) {
			if (map.isPassengerDelivered()) {
				stopMessages.add("Passenger delivered.");
			}
		}
	}
	
	private void checkPassengerPickedUp() {
		if (sim.getConfig().terminalsConfig().passenger_pick_up) {
			if (map.isPassengerCarried()) {
				stopMessages.add("There are no points remaining.");
			}
		}
	}
	
	@Override
	public void reset() {
		map.reset();
		resetState();
	}

	@Override
	public boolean hasPlayer(String name) {
		return players.get(name) != null;
	}
	
	@Override
	public boolean addPlayer(PlayerConfig cfg) {
		int [] location = WorldUtil.getStartingLocation(map, cfg.pos);
		if (location == null) {
			sim.error("Taxi", "There are no suitable starting locations.");
			return false;
		}

		Taxi player = new Taxi(sim, cfg.name, cfg.color, fuelStartMin, fuelStartMax, refuel, disableFuel);
		players.add(player, cfg.pos);
		
		if (cfg.productions != null) {
			TaxiCommander cmdr = sim.getCogArch().createTaxiCommander(player, cfg.productions, cfg.shutdown_commands );
			if (cmdr == null) {
				players.remove(player);
				return false;
			}
			player.setCommander(cmdr);
		}

		players.setLocation(player, location);
		
		// put the player in it
		map.getCell(location).addPlayer(player);
		
		logger.info(player.getName() + ": Spawning at (" + location[0] + "," + location[1] + ")");
		
		setLocations();
		return true;
	}

	@Override
	public GridMap getMap() {
		return map;
	}

	@Override
	public Player[] getPlayers() {
		return players.getAllAsPlayers();
	}

	@Override
	public void interrupted(String agentName) {
		players.interrupted(agentName);
		stopMessages.add("interrupted");
	}

	@Override
	public boolean isTerminal() {
		return stopMessages.size() > 0;
	}

	@Override
	public int numberOfPlayers() {
		return players.numberOfPlayers();
	}

	@Override
	public void removePlayer(String name) {
		Taxi taxi = players.get(name);
		map.getCell(players.getLocation(taxi)).clearPlayers();
		players.remove(taxi);
		taxi.shutdownCommander();
		setLocations();
	}
}
