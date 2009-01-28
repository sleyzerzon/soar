package soar2d.world;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Iterator;
import java.util.ListIterator;

import org.apache.log4j.Logger;

import sml.Agent;
import soar2d.Direction;
import soar2d.Names;
import soar2d.Soar2D;
import soar2d.config.PlayerConfig;
import soar2d.map.CellObject;
import soar2d.map.EatersMap;
import soar2d.map.GridMap;
import soar2d.map.CellObject.RewardApply;
import soar2d.players.CommandInfo;
import soar2d.players.Eater;
import soar2d.players.Player;
import soar2d.players.SoarEater;

public class EatersWorld implements World {
	private static Logger logger = Logger.getLogger(EatersWorld.class);

	private File eatersMapFile;
	private EatersMap eatersMap;
	private WorldData data;
	private PlayersManager<Eater> players = new PlayersManager<Eater>();
	
	public EatersWorld(String map) throws Exception {
		
		eatersMapFile = new File(map);
		if (!eatersMapFile.exists()) {
			throw new Exception("Map file doesn't exist: " + eatersMapFile.getAbsolutePath());
		}
		
		reset();
	}
	
	public void reset() throws Exception {
		eatersMap = new EatersMap(eatersMapFile);
		data = new WorldData();
		resetPlayers();
	}
	
	private void resetPlayers() throws Exception {
		if (players.numberOfPlayers() == 0) {
			return;
		}
		
		for (Eater eater : players.getAll()) {
			// find a suitable starting location
			int [] startingLocation = players.getStartingLocation(eater, eatersMap, true);

			// remove food from it
			eatersMap.getCell(startingLocation).removeAllByProperty(Names.kPropertyEdible);
			
			// put the player in it
			eatersMap.getCell(startingLocation).setPlayer(eater);

			resetPlayer(eater);
		}
		
		updatePlayers();
	}
	
	public void update() throws Exception {
		
		String restartMessage = null;
		
		Soar2D.config.generalConfig().hidemap = false;
		
		// Collect human input
		for (Eater eater : players.getAll()) {
			if (Soar2D.config.generalConfig().force_human || players.isHuman(eater)) {
				if (eater.getCommand(true) == null) {
					return;
				}
			}
		}
		
		++data.worldCount;

		if (Soar2D.config.terminalsConfig().max_updates > 0) {
			if (data.worldCount >= Soar2D.config.terminalsConfig().max_updates) {
				String message = "Reached maximum updates, stopping.";
				if (Soar2D.control.checkRunsTerminal()) {
					if (!data.printedStats) {
						WorldUtil.dumpStats(players.getSortedScores(), players.getAllAsPlayers(), true, message);
						data.printedStats = true;
					}
					return;
				} else {
					restartMessage = message;
				}
			}
		}
		
		if (Soar2D.config.terminalsConfig().winning_score > 0) {
			int[] scores = players.getSortedScores();
			if (scores[scores.length - 1] >= Soar2D.config.terminalsConfig().winning_score) {
				String message = "At least one player has achieved at least " + Soar2D.config.terminalsConfig().winning_score + " points.";
				if (Soar2D.control.checkRunsTerminal()) {
					if (!data.printedStats) {
						WorldUtil.dumpStats(players.getSortedScores(), players.getAllAsPlayers(), true, message);
						data.printedStats = true;
					}
					return;
				} else {
					restartMessage = message;
				}
			}
		}
		
		if (Soar2D.config.terminalsConfig().points_remaining) {
			if (eatersMap.getScoreCount() <= 0) {
				if (!data.printedStats) {
					WorldUtil.dumpStats(players.getSortedScores(), players.getAllAsPlayers(), true, "There are no points remaining.");
					data.printedStats = true;
				}
				return;
			}
		}
	
		if (Soar2D.config.terminalsConfig().food_remaining) {
			if (eatersMap.getFoodCount() <= 0) {
				String message = "All of the food is gone.";
				if (Soar2D.control.checkRunsTerminal()) {
					if (!data.printedStats) {
						WorldUtil.dumpStats(players.getSortedScores(), players.getAllAsPlayers(), true, message);
						data.printedStats = true;
					}
					return;
					
				} else {
					restartMessage = message;
				}
			}
		}

		if (Soar2D.config.terminalsConfig().unopened_boxes) {
			if (eatersMap.getUnopenedBoxCount() <= 0) {
				if (!data.printedStats) {
					WorldUtil.dumpStats(players.getSortedScores(), players.getAllAsPlayers(), true, "All of the boxes are open.");
					data.printedStats = true;
				}
				return;
			}
		}

		if (players.numberOfPlayers() == 0) {
			logger.warn("Update called with no players.");
			Soar2D.control.stopSimulation();
			return;
		}
		
		// get moves
		for (Eater eater : players.getAll()) {
			CommandInfo move = eater.getCommand(false);
			if (Soar2D.control.isShuttingDown()) {
				return;
			}
			
			assert move != null;
			String moveString = move.toString();
			if (moveString.length() > 0) logger.info(eater.getName() + ": " + moveString);

			players.setMove(eater, move);
			
			if (move.stopSim) {
				String message = eater.getName() + " issued simulation stop command.";
				if (Soar2D.config.terminalsConfig().agent_command) {
					if (!data.printedStats) {
						WorldUtil.dumpStats(players.getSortedScores(), players.getAllAsPlayers(), true, message);
						data.printedStats = true;
					}
				} else {
					logger.warn(message);
				}
			}
		}
		
		moveEaters();
		if (Soar2D.control.isShuttingDown()) {
			return;
		}
		restartMessage = updateMapAndEatFood();
		handleEatersCollisions(findCollisions(players));	
		updatePlayers();
		eatersMap.updateObjects();

		if (restartMessage != null) {
			if (!data.printedStats) {
				WorldUtil.dumpStats(players.getSortedScores(), players.getAllAsPlayers(), true, restartMessage);
				data.printedStats = true;
			}
			
			reset();
			if (Soar2D.wm.using()) {
				Soar2D.wm.reset();
			}
			Soar2D.control.startSimulation(false, false);
		}
	}
	
	public void updatePlayers() throws Exception {
		for (Eater eater : players.getAll()) {
			eater.update(players.getLocation(eater), eatersMap);
		}
	}

	private void moveEaters() {
		for (Eater eater : players.getAll()) {
			CommandInfo move = players.getMove(eater);			

			if (!move.move) {
				continue;
			}

			// Calculate new location
			int [] oldLocation = players.getLocation(eater);
			int [] newLocation = Arrays.copyOf(oldLocation, oldLocation.length);
			Direction.translate(newLocation, move.moveDirection);
			if (move.jump) {
				Direction.translate(newLocation, move.moveDirection);
			}
			
			// Verify legal move and commit move
			if (eatersMap.isInBounds(newLocation) && !eatersMap.getCell(newLocation).hasAnyWithProperty(Names.kPropertyBlock)) {
				// remove from cell
				eatersMap.getCell(oldLocation).setPlayer(null);
				
				if (move.jump) {
					eater.adjustPoints(Soar2D.config.eatersConfig().jump_penalty, "jump penalty");
				}
				players.setLocation(eater, newLocation);
				
			} else {
				eater.adjustPoints(Soar2D.config.eatersConfig().wall_penalty, "wall collision");
			}
		}
	}

	private String updateMapAndEatFood() {
		for (Eater eater : players.getAll()) {
			CommandInfo lastMove = players.getMove(eater);
			int [] location = players.getLocation(eater);
			
			if (lastMove.move || lastMove.jump) {
				eatersMap.getCell(location).setPlayer(eater);

				ArrayList<CellObject> moveApply = eatersMap.getCell(location).getAllWithProperty(Names.kPropertyMoveApply);
				if (moveApply != null) {
					for (CellObject object : moveApply) {
						if (apply(object, eater)) {
							eatersMap.getCell(location).removeObject(object.getName());
						}
					}
				}
			}
			
			if (!lastMove.dontEat) {
				eat(eater, location);
			}
			
			if (lastMove.open) {
				return open(eater, location, lastMove.openCode);
			}
		}
		return null;
	}
	
	private boolean apply(CellObject object, Eater eater) {
		object.propertiesApply();
		{
			int points = object.pointsApply();
			if (points != 0) {
				eater.adjustPoints(points, object.getName());
			}
		}
		{
			RewardApply reward = object.rewardApply();
			if (reward != null && reward.points != 0) {
				eater.adjustPoints(reward.points, reward.message);
			}
		}
		return object.removeApply();
	}
	
	private String open(Eater eater, int [] location, int openCode) {
		ArrayList<CellObject> boxes = eatersMap.getCell(location).getAllWithProperty(Names.kPropertyBox);
		if (boxes == null) {
			logger.warn(eater.getName() + " tried to open but there is no box.");
			return null;
		}

		// TODO: multiple boxes
		assert boxes.size() <= 1;
		
		CellObject box = boxes.get(0);
		if (box.hasProperty(Names.kPropertyStatus)) {
			if (box.getProperty(Names.kPropertyStatus).equalsIgnoreCase(Names.kOpen)) {
				logger.warn(eater.getName() + " tried to open an open box.");
				return null;
			}
		}
		if (openCode != 0) {
			box.addProperty(Names.kPropertyOpenCode, Integer.toString(openCode));
		}
		if (apply(box, eater)) {
			eatersMap.getCell(location).removeObject(box.getName());
		}
		
		if (box.getResetApply()) {
			String message = "Max resets achieved.";
			if (Soar2D.control.checkRunsTerminal()) {
				if (!data.printedStats) {
					WorldUtil.dumpStats(players.getSortedScores(), players.getAllAsPlayers(), true, message);
				}
			} else {
				return message;
			}
		}
		return null;
	}
	
	private void eat(Eater eater, int [] location) {
		ArrayList<CellObject> list = eatersMap.getCell(location).getAllWithProperty(Names.kPropertyEdible);
		if (list != null) {
			for (CellObject food : list) {
				if (apply(food, eater)) {
					// if this returns true, it is consumed
					eatersMap.getCell(location).removeObject(food.getName());
				}
			}
		}			
	}
	
	private ArrayList<ArrayList<Eater>> findCollisions(PlayersManager<Eater> eaters) {
		ArrayList<ArrayList<Eater>> collisions = new ArrayList<ArrayList<Eater>>(players.numberOfPlayers() / 2);

		// Make sure collisions are possible
		if (players.numberOfPlayers() < 2) {
			return collisions;
		}
		
		// Optimization to not check the same name twice
		HashSet<Eater> colliding = new HashSet<Eater>(players.numberOfPlayers());
		ArrayList<Eater> collision = new ArrayList<Eater>(players.numberOfPlayers());

		ListIterator<Eater> leftIter = players.listIterator();
		while (leftIter.hasNext()) {
			Eater left = leftIter.next();
			
			// Check to see if we're already colliding
			if (colliding.contains(left)) {
				continue;
			}
			
			ListIterator<Eater> rightIter = players.listIterator(leftIter.nextIndex());
			// Clear collision list now
			collision.clear();
			while (rightIter.hasNext()) {
				// For each player to my right (I've already checked to my left)
				Eater right = rightIter.next();

				// Check to see if we're already colliding
				if (colliding.contains(right)) {
					continue;
				}
				
				// If the locations match, we have a collision
				if (players.getLocation(left).equals(players.getLocation(right))) {
					
					// Add to this set to avoid checking same player again
					colliding.add(left);
					colliding.add(right);
					
					// Add the left the first time a collision is detected
					if (collision.size() == 0) {
						collision.add(left);
						
						logger.debug("collision at " + players.getLocation(left));
					}
					// Add each right as it is detected
					collision.add(right);
				}
			}
			
			// Add the collision to the total collisions if there is one
			if (collision.size() > 0) {
				collisions.add(new ArrayList<Eater>(collision));
			}
		}

		return collisions;
	}
		
	private void handleEatersCollisions(ArrayList<ArrayList<Eater>> collisions) throws Exception {
		
		// if there are no total collisions, we're done
		if (collisions.size() < 1) {
			return;
		}

		ArrayList<Eater> collision = new ArrayList<Eater>(players.numberOfPlayers());
		
		Iterator<ArrayList<Eater>> collisionIter = collisions.iterator();
		while (collisionIter.hasNext()) {
			collision = collisionIter.next();

			assert collision.size() > 0;
			logger.debug("Processing collision group with " + collision.size() + " collidees.");

			// Redistribute wealth
			int cash = 0;			
			ListIterator<Eater> collideeIter = collision.listIterator();
			while (collideeIter.hasNext()) {
				cash += collideeIter.next().getPoints();
			}
			if (cash > 0) {
				int trash = cash % collision.size();
				cash /= collision.size();
				logger.debug("Cash to each: " + cash + " (" + trash + " lost in division)");
				collideeIter = collision.listIterator();
				while (collideeIter.hasNext()) {
					collideeIter.next().setPoints(cash, "collision");
				}
			} else {
				logger.debug("Sum of cash is negative.");
			}
			
			int [] collisionLocation = players.getLocation(collision.get(0));

			// Add the boom on the map
			setExplosion(collisionLocation);

			// Remove from former location (only one of these for all players)
			eatersMap.getCell(collisionLocation).setPlayer(null);
			
			// Move to new cell, consume food
			collideeIter = collision.listIterator();
			while (collideeIter.hasNext()) {
				Eater eater = collideeIter.next();
				int [] location = players.getStartingLocation(eater, eatersMap, false);

				// put the player in it
				eatersMap.getCell(location).setPlayer(eater);
				
				eater.setFragged(true);
				if (!players.getMove(eater).dontEat) {
					eat(eater, location);
				}
			}
		}
	}

	public void setExplosion(int[] xy) {
		CellObject explosion = eatersMap.createObjectByName(Names.kExplosion);
		explosion.addProperty(Names.kPropertyLinger, "2");
		explosion.setLingerUpdate(true);
		eatersMap.getCell(xy).addObject(explosion);
	}
	

	public int getMinimumAvailableLocations() {
		return 1;
	}

	public void resetPlayer(Eater eater) throws Exception {
		eater.reset();
	}

	public boolean isTerminal() {
		return data.printedStats;
	}

	public void removePlayer(String name) throws Exception {
		Eater eater = players.get(name);
		eatersMap.getCell(players.getLocation(eater)).setPlayer(null);
		players.remove(eater);
		eater.shutdownCommander();
		updatePlayers();
	}
	
	public void interrupted(String agentName) throws Exception {
		players.interrupted(agentName);
		
		if (!data.printedStats) {
			WorldUtil.dumpStats(players.getSortedScores(), players.getAllAsPlayers(), true, "interrupted");
		}
	}

	public int numberOfPlayers() {
		return players.numberOfPlayers();
	}

	public void addPlayer(String playerId, PlayerConfig playerConfig) throws Exception {
		
		boolean human = playerConfig.productions == null;
		Eater eater = new Eater(playerId);

		players.add(eater, eatersMap, playerConfig.pos, human);
		
		if (playerConfig.productions != null) {
			Agent agent = Soar2D.simulation.createSoarAgent(playerConfig.name, playerConfig.productions);
			SoarEater soarEater = new SoarEater(eater, agent, Soar2D.config.eatersConfig().vision, playerConfig.shutdown_commands, eatersMap.getMetadataFile());
			eater.setCommander(soarEater);
		}

		int [] location = players.getStartingLocation(eater, eatersMap, true);

		// remove food from it
		eatersMap.getCell(location).removeAllByProperty(Names.kPropertyEdible);

		// put the player in it
		eatersMap.getCell(location).setPlayer(eater);
		
		logger.info(eater.getName() + ": Spawning at (" + location[0] + "," + location[1] + ")");
		
		updatePlayers();
	}

	public GridMap getMap() {
		return eatersMap;
	}

	public int getWorldCount() {
		return data.worldCount;
	}

	public Player[] getPlayers() {
		return players.getAllAsPlayers();
	}
}
