package soar2d.world;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Iterator;
import java.util.ListIterator;

import org.apache.log4j.Logger;

import soar2d.CognitiveArchitecture;
import soar2d.Direction;
import soar2d.Names;
import soar2d.Soar2D;
import soar2d.config.PlayerConfig;
import soar2d.map.CellObject;
import soar2d.map.EatersMap;
import soar2d.map.GridMap;
import soar2d.players.CommandInfo;
import soar2d.players.Eater;
import soar2d.players.EaterCommander;
import soar2d.players.Player;

public class EatersWorld implements World {
	private static Logger logger = Logger.getLogger(EatersWorld.class);

	private EatersMap eatersMap;
	private PlayersManager<Eater> players = new PlayersManager<Eater>();
	private ArrayList<String> stopMessages = new ArrayList<String>();
	private CognitiveArchitecture cogArch;

	public EatersWorld(CognitiveArchitecture cogArch) throws Exception {
		this.cogArch = cogArch;
	}
	
	public void setMap(String mapPath) throws Exception {
		EatersMap oldMap = eatersMap;
		try {
			eatersMap = new EatersMap(mapPath);
		} catch (Exception e) {
			if (oldMap == null) {
				throw e;
			}
			eatersMap = oldMap;
			logger.error("Map load failed, restored old map.");
			return;
		}
		
		// This can throw a more fatal error.
		reset();
	}
	
	public void reset() throws Exception {
		eatersMap.reset();
		stopMessages.clear();
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

			eater.reset();
		}
		
		updatePlayers();
	}

	private void checkPointsRemaining() {
		if (Soar2D.config.terminalsConfig().points_remaining) {
			if (eatersMap.getScoreCount() <= 0) {
				stopMessages.add("There are no points remaining.");
			}
		}
	}
	
	private void checkFoodRemaining() {
		if (Soar2D.config.terminalsConfig().food_remaining) {
			if (eatersMap.getFoodCount() <= 0) {
				stopMessages.add("All the food is gone.");
			}
		}
	}
	
	private void checkUnopenedBoxes() {
		if (Soar2D.config.terminalsConfig().unopened_boxes) {
			if (eatersMap.getUnopenedBoxCount() <= 0) {
				stopMessages.add("All of the boxes are open.");
			}
		}
	}

	public void update(int worldCount) throws Exception {
		// Collect input
		for (Eater eater : players.getAll()) {
			CommandInfo command = eater.getCommand();
			if (command == null) {
				Soar2D.control.stopSimulation();
				return;
			}
			players.setCommand(eater, command);
			WorldUtil.checkStopSim(stopMessages, command, eater);
		}
		
		WorldUtil.checkMaxUpdates(stopMessages, worldCount);
		WorldUtil.checkWinningScore(stopMessages, players.getSortedScores());
		checkPointsRemaining();
		checkFoodRemaining();
		checkUnopenedBoxes();
		WorldUtil.checkNumPlayers(players.numberOfPlayers());
		
		moveEaters();
		if (Soar2D.control.isShuttingDown()) {
			return;
		}
		
		updateMapAndEatFood();
		
		handleEatersCollisions(findCollisions(players));	
		updatePlayers();
		eatersMap.updateObjects();

		if (stopMessages.size() > 0) {
			boolean stopping = Soar2D.control.checkRunsTerminal();
			WorldUtil.dumpStats(players.getSortedScores(), players.getAllAsPlayers(), stopping, stopMessages);

			if (stopping) {
				Soar2D.control.stopSimulation();
			} else {
				// reset and continue;
				reset();
				Soar2D.control.startSimulation(false, false);
			}
		}
	}
	
	private void updatePlayers() throws Exception {
		for (Eater eater : players.getAll()) {
			eater.update(players.getLocation(eater), eatersMap);
		}
	}

	private void moveEaters() {
		for (Eater eater : players.getAll()) {
			CommandInfo command = players.getCommand(eater);			

			if (!command.move) {
				continue;
			}

			// Calculate new location
			int [] oldLocation = players.getLocation(eater);
			int [] newLocation = Arrays.copyOf(oldLocation, oldLocation.length);
			Direction.translate(newLocation, command.moveDirection);
			if (command.jump) {
				Direction.translate(newLocation, command.moveDirection);
			}
			
			// Verify legal move and commit move
			if (eatersMap.isInBounds(newLocation) && !eatersMap.getCell(newLocation).hasAnyWithProperty(Names.kPropertyBlock)) {
				// remove from cell
				eatersMap.getCell(oldLocation).setPlayer(null);
				
				if (command.jump) {
					eater.adjustPoints(Soar2D.config.eatersConfig().jump_penalty, "jump penalty");
				}
				players.setLocation(eater, newLocation);
				
			} else {
				eater.adjustPoints(Soar2D.config.eatersConfig().wall_penalty, "wall collision");
			}
		}
	}

	private void updateMapAndEatFood() {
		for (Eater eater : players.getAll()) {
			CommandInfo lastCommand = players.getCommand(eater);
			int [] location = players.getLocation(eater);
			
			if (lastCommand.move || lastCommand.jump) {
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
			
			if (!lastCommand.dontEat) {
				eat(eater, location);
			}
			
			if (lastCommand.open) {
				open(eater, location, lastCommand.openCode);
			}
		}
	}
	
	private boolean apply(CellObject object, Eater eater) {
		object.applyProperties();
		
		if (object.hasProperty("apply.points")) {
			int points = object.getIntProperty("apply.points", 0);
			eater.adjustPoints(points, object.getName());
		}
		if (object.hasProperty("apply.reward")) {
			// am I the positive box
			if (object.hasProperty("apply.reward.correct")) {
				// if the open code is not zero, get an open code
				int suppliedOpenCode = object.getIntProperty("open-code", 0);

				// see if we opened the box correctly (will both be 0 if no open code)
				if (suppliedOpenCode == object.getIntProperty("apply.reward.code", 0)) {
					// reward positively
					eater.adjustPoints(object.getIntProperty("apply.reward.positive", 0), "positive reward");
				} else {
					eater.adjustPoints(object.getIntProperty("apply.reward.negative", 0), "small reward (wrong open code)");
				}
			} else {
				// I'm  not the positive box, set resetApply false
				object.removeProperty("apply.reset");
				
				// reward negatively
				eater.adjustPoints(-1 * object.getIntProperty("apply.reward.positive", 0), "negative reward (wrong box)");
			}
		}
		
		return object.hasProperty("apply.remove");
	}
	
	private void open(Eater eater, int [] location, int openCode) {
		ArrayList<CellObject> boxes = eatersMap.getCell(location).getAllWithProperty(Names.kPropertyBox);
		if (boxes == null) {
			logger.warn(eater.getName() + " tried to open but there is no box.");
		}

		// TODO: multiple boxes
		assert boxes.size() <= 1;
		
		CellObject box = boxes.get(0);
		if (box.hasProperty(Names.kPropertyStatus)) {
			if (box.getProperty(Names.kPropertyStatus).equalsIgnoreCase(Names.kOpen)) {
				logger.warn(eater.getName() + " tried to open an open box.");
			}
		}
		if (openCode != 0) {
			box.setIntProperty("open-code", openCode);
		}
		if (apply(box, eater)) {
			eatersMap.getCell(location).removeObject(box.getName());
		}
		checkResetApply(box);
	}

	private void checkResetApply(CellObject box) {
		if (box.hasProperty("apply.reset")) {
			stopMessages.add(box.getName() + " called for reset.");
		}
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
				if (!players.getCommand(eater).dontEat) {
					eat(eater, location);
				}
			}
		}
	}

	private void setExplosion(int[] xy) {
		CellObject explosion = eatersMap.createObjectByName(Names.kExplosion);
		explosion.setIntProperty("update.linger", 2);
		eatersMap.getCell(xy).addObject(explosion);
	}
	

	public boolean isTerminal() {
		return stopMessages.size() > 0;
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
		stopMessages.add("interrupted");
	}

	public int numberOfPlayers() {
		return players.numberOfPlayers();
	}

	public void addPlayer(String playerId, PlayerConfig playerConfig) throws Exception {
		
		boolean human = playerConfig.productions == null;
		Eater eater = new Eater(playerId);

		players.add(eater, eatersMap, playerConfig.pos, human);
		
		if (playerConfig.productions != null) {
			EaterCommander eaterCommander = cogArch.createEaterCommander(eater, playerConfig.productions, Soar2D.config.eatersConfig().vision, playerConfig.shutdown_commands, eatersMap.getMetadataFile());
			eater.setCommander(eaterCommander);
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

	public Player[] getPlayers() {
		return players.getAllAsPlayers();
	}
}
