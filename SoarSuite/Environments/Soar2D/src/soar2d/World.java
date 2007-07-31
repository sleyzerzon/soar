package soar2d;

import java.awt.Point;
import java.awt.geom.Point2D;
import java.util.*;
import java.util.logging.*;
import java.lang.Math;

import soar2d.Configuration.SimType;
import soar2d.player.*;
import soar2d.world.*;

public class World {

	private IWorld worldModule;
	
	private static Logger logger = Logger.getLogger("soar2d");

	private PlayersManager players = new PlayersManager();
	
	private int worldCount = 0;
	private int runsFoodRemaining = 0;
	private boolean printedStats = false;
	
	private GridMap map;
	public GridMap getMap() {
		return map;
	}
	
	public boolean load() {
		if (worldModule == null) {
			switch(Soar2D.config.getType()) {
			case kTankSoar:
				worldModule = new TankSoarWorld();
				break;
			case kEaters:
				worldModule = new EatersWorld();
				break;
			case kBook:
				worldModule = new BookWorld();
				break;
			}
		}
		
		this.runsFoodRemaining = Soar2D.config.getTerminalFoodRemainingContinue();
		Soar2D.control.setRunsTerminal(Soar2D.config.getTerminalMaxRuns());
		return loadInternal(false);
	}

	private boolean loadInternal(boolean resetDuringRun) {
		GridMap newMap = new GridMap(Soar2D.config);
		
		try {
			newMap.load();
		} catch (GridMap.LoadError e) {
			Soar2D.control.severeError(e.getMessage());
			return false;
		}
		
		if (worldModule.postLoad(newMap) == false) {
			return false;
		}
		
		
		map = newMap;
		
		reset();
		Soar2D.control.resetTime();
		resetPlayers(resetDuringRun);
		
		logger.info("Map loaded, world reset.");
		if (map.getOpenCode() != 0) {
			logger.info("The correct open code is: " + map.getOpenCode());
		}
		return true;
	}
	
	void resetPlayers(boolean resetDuringRun) {
		if (players.numberOfPlayers() == 0) {
			return;
		}
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			// for each player
			Player player = iter.next();
			
			resetPlayer(player, resetDuringRun);
		}
		
		worldModule.updatePlayers(false, map, players);
	}
	
	private boolean resetPlayer(Player player, boolean resetDuringRun) {
		// find a suitable starting location
		Point startingLocation = putInStartingLocation(player, true);
		if (startingLocation == null) {
			return false;
		}
		
		switch (Soar2D.config.getType()) {
		case kEaters:
			// remove food from it
			map.removeAllWithProperty(startingLocation, Names.kPropertyEdible);
			
			// This is here because the TOSCA stuff wants to keep around the reward
			// in the beginning of the next phase
			
			if (resetDuringRun) {
				player.mapReset();
			}
			
			// falls through
			
		case kTankSoar:
			// reset (init-soar)
			player.reset();
			break;
			
		case kBook:
			if (resetDuringRun) {
				player.fragged();
			} else {
				// reset (init-soar)
				player.reset();
			}
			break;
		}
		return true;
	}
	
	public void shutdown() {
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			Soar2D.simulation.destroyPlayer(iter.next());
			iter = players.iterator();
		}
		assert this.players.numberOfPlayers() == 0;
		map.shutdown();
	}
	
	public void removePlayer(String name) {
		logger.info("Removing player " + name);
		Player player = players.get(name);
		if (player == null) {
			logger.warning("destroyPlayer: Couldn't find player name match for " + name + ", ignoring.");
			return;
		}
		map.setPlayer(players.getLocation(player), null);
		players.remove(player);
		worldModule.updatePlayers(true, map, players);
		
	}
	
	Point putInStartingLocation(Player player, boolean useInitialLocation) {
		// Get available cells
		
		ArrayList<Point> availableLocations = map.getAvailableLocations();
		// make sure there is an available cell
		switch (Soar2D.config.getType()) {
		case kTankSoar:
			// There must be enough room for all tank and missile packs
			if (availableLocations.size() < Soar2D.config.getMaxMissilePacks() + 1) {
				Soar2D.control.severeError("There are no suitable starting locations for " + player.getName() + ".");
				return null;
			}
			break;
		case kEaters:
		case kBook:
			if (availableLocations.size() < 1) {
				Soar2D.control.severeError("There are no suitable starting locations for " + player.getName() + ".");
				return null;
			}
			break;
		}
		
		Point location = null;

		if (useInitialLocation && players.hasInitialLocation(player)) {
			location = players.getInitialLocation(player);
			if (!availableLocations.contains(location)) {
				logger.warning(player.getName() + ": Initial location (" + location.x + "," + location.y + ") is blocked, going random.");
				location = null;
			}
		}
		
		if (location == null) {
			location = availableLocations.get(Simulation.random.nextInt(availableLocations.size()));
		}
		
		// put the player in it
		map.setPlayer(location, player);
		players.setLocation(player, location);
		worldModule.putInStartingLocation(player, map, players, location);
		return location;
	}

	public boolean addPlayer(Player player, Point initialLocation, boolean human) {
		assert players.exists(player) == false;
		logger.info("Adding player " + player);

		players.add(player, initialLocation, human);

		if (!resetPlayer(player, false)) {
			players.remove(player);
			return false;
		}
		
		Point location = players.getLocation(player);
		assert location != null;
		logger.info(player.getName() + ": Spawning at (" + 
				location.x + "," + location.y + ")");
		
		worldModule.updatePlayers(true, map, players);
		return true;
	}
	
	void stopAndDumpStats(String message) {
		int[] scores = getSortedScores();
		
		if (!printedStats) {
			printedStats = true;
			Soar2D.control.infoPopUp(message);
			Soar2D.control.stopSimulation();
			boolean draw = false;
			if (scores.length > 1) {
				if (scores[scores.length - 1] ==  scores[scores.length - 2]) {
					if (logger.isLoggable(Level.FINER)) logger.finer("Draw detected.");
					draw = true;
				}
			}
			
			Iterator<Player> iter = players.iterator();
			while (iter.hasNext()) {
				String status = null;
				Player player = iter.next();
				if (player.getPoints() == scores[scores.length - 1]) {
					status = draw ? "draw" : "winner";
				} else {
					status = "loser";
				}
				logger.info(player.getName() + ": " + player.getPoints() + " (" + status + ").");
			}
		}
	}
	
	private int[] getSortedScores() {
		int[] scores = new int[players.numberOfPlayers()];
		Iterator<Player> iter = players.iterator();
		int i = 0;
		Player player;
		while (iter.hasNext()) {
			player = iter.next();
			scores[i] = player.getPoints();
			++i;
		}
		Arrays.sort(scores);
		return scores;
	}
	
	public void update() {
		
		boolean restartAfterUpdate = false;
		
		Soar2D.config.setHide(false);
		
		// Collect human input
		Iterator<Player> humanPlayerIter = players.humanIterator();
		while (humanPlayerIter.hasNext()) {
			Player human = humanPlayerIter.next();
			if (!human.getHumanMove()) {
				return;
			}
		}
		
		++worldCount;

		if (Soar2D.config.getTerminalMaxUpdates() > 0) {
			if (worldCount >= Soar2D.config.getTerminalMaxUpdates()) {
				if (Soar2D.control.checkRunsTerminal()) {
					stopAndDumpStats("Reached maximum updates, stopping.");
					return;
				} else {
					restartAfterUpdate = true;
				}
			}
		}
		
		if (Soar2D.config.getTerminalWinningScore() > 0) {
			int[] scores = getSortedScores();
			if (scores[scores.length - 1] >= Soar2D.config.getTerminalWinningScore()) {
				if (Soar2D.config.getTerminalWinningScoreContinue()) {
					if (Soar2D.control.checkRunsTerminal()) {
						stopAndDumpStats("At least one player has achieved at least " + Soar2D.config.getTerminalWinningScore() + " points.");
						return;
					} else {
						restartAfterUpdate = true;
					}
				} else {
					stopAndDumpStats("At least one player has achieved at least " + Soar2D.config.getTerminalWinningScore() + " points.");
					return;
				}
			}
		}
		
		if (Soar2D.config.getTerminalPointsRemaining()) {
			if (map.getScoreCount() <= 0) {
				stopAndDumpStats("There are no points remaining.");
				return;
			}
		}

		if (Soar2D.config.getTerminalFoodRemaining()) {
			if (map.getFoodCount() <= 0) {
				boolean stopNow = true;
				
				if (Soar2D.config.getTerminalFoodRemainingContinue() != 0) {
					// reduce continues by one if it is positive
					if (runsFoodRemaining > 0) {
						runsFoodRemaining -= 1;
					}

					// we only stop if continues is zero
					if (runsFoodRemaining != 0) {
						stopNow = false;
					}
				}
				
				if (stopNow) {
					stopAndDumpStats("All of the food is gone.");
					return;
					
				} else {
					restartAfterUpdate = true;
				}
			}
		}

		if (Soar2D.config.getTerminalUnopenedBoxes()) {
			if (map.getUnopenedBoxCount() <= 0) {
				stopAndDumpStats("All of the boxes are open.");
				return;
			}
		}

		if (players.numberOfPlayers() == 0) {
			logger.warning("Update called with no players.");
			Soar2D.control.stopSimulation();
			return;
		}
		
		// get moves
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			Player player = iter.next();
			
			MoveInfo move = player.getMove();
			if (Soar2D.control.isShuttingDown()) {
				return;
			}
			
			assert move != null;
			String moveString = move.toString();
			if (moveString.length() > 0) logger.info(player.getName() + ": " + moveString);

			players.setMove(player, move);
			
			if (move.stopSim) {
				if (Soar2D.config.getTerminalAgentCommand()) {
					stopAndDumpStats(player.getName() + " issued simulation stop command.");
				} else {
					Soar2D.logger.warning(player.getName() + " issued ignored stop command.");
				}
			}
		}
		
		if (worldModule.update(map, players)) {
			restartAfterUpdate = true;
		}

		if (restartAfterUpdate) {
			int[] scores = getSortedScores();
			boolean draw = false;
			if (scores.length > 1) {
				if (scores[scores.length - 1] ==  scores[scores.length - 2]) {
					if (logger.isLoggable(Level.FINER)) logger.finer("Draw detected.");
					draw = true;
				}
			}
			
			iter = players.iterator();
			while (iter.hasNext()) {
				String status = null;
				Player player = iter.next();
				if (player.getPoints() == scores[scores.length - 1]) {
					status = draw ? "draw" : "winner";
				} else {
					status = "loser";
				}
				logger.info(player.getName() + ": " + player.getPoints() + " (" + status + ").");
			}
			
			loadInternal(true);
			if (Soar2D.wm.using()) {
				Soar2D.wm.reset();
			}
		}
	}
	
	
	void fragPlayer(Player player) {
		// remove from past cell
		Point oldLocation = players.getLocation(player);
		map.setExplosion(oldLocation);
		map.setPlayer(oldLocation, null);

		// Get available spots
		ArrayList<Point> spots = map.getAvailableLocations();
		assert spots.size() > 0;
		
		// pick one and put the player in it
		Point location = spots.get(Simulation.random.nextInt(spots.size()));
		map.setPlayer(location, player);
		
		// save the location
		players.setLocation(player, location);
		worldModule.fragPlayer(player, map, players, location);
		
		// reset the player state
		player.fragged();

		Soar2D.logger.info(player.getName() + ": Spawning at (" + 
				location.x + "," + location.y + ")");
	}
	
	public void reset() {
		worldCount = 0;
		printedStats = false;
		worldModule.reset();
		//System.out.println(map);
	}
	
	public int getWorldCount() {
		return worldCount;
	}
	
	boolean isTerminal() {
		return printedStats;
	}

	public boolean recentlyMovedOrRotated(Player targetPlayer) {
		MoveInfo move = players.getMove(targetPlayer);
		if (move == null) {
			return false;
		}
		return move.move || move.rotate;
	}

	public void interruped(String name) {
		if (Soar2D.wm.using()) {
			return;
		}
		if (players.numberOfPlayers() <= 1) {
			return;
		}
		Iterator<Player> iter = players.iterator();
		Player thePlayer = null;
		Integer lowestScore = null;
		while (iter.hasNext()) {
			Player player = iter.next();
			if (player.getName().equals(name)) {
				thePlayer = player;
			} else {
				if (lowestScore == null) {
					lowestScore = new Integer(player.getPoints());
				} else {
					lowestScore = Math.min(lowestScore, player.getPoints());
				}
			}
			
		}
		if ((thePlayer == null) || (lowestScore == null)) {
			// shouldn't happen if name is valid
			return;
		}
		
		lowestScore -= 1;
		thePlayer.setPoints(lowestScore, "interrupted");
		this.stopAndDumpStats("interrupted");
	}
	
	public PlayersManager getPlayers() {
		return players;
	}
}
