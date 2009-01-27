package soar2d.world;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Iterator;
import java.util.ListIterator;

import org.apache.log4j.Logger;

import soar2d.Direction;
import soar2d.Names;
import soar2d.Soar2D;
import soar2d.map.CellObject;
import soar2d.map.EatersMap;
import soar2d.players.MoveInfo;
import soar2d.players.Player;

public class EatersWorld implements IWorld {
	private static Logger logger = Logger.getLogger(EatersWorld.class);

	private File eatersMapFile;
	private EatersMap eatersMap;
	private WorldData data;
	private PlayersManager players = new PlayersManager();
	
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
		resetPlayers(false);
	}
	
	private void resetPlayers(boolean resetDuringRun) throws Exception {
		if (players.numberOfPlayers() == 0) {
			return;
		}
		
		for (Player player : players.getAll()) {
			// find a suitable starting location
			int [] startingLocation = players.putInStartingLocation(player, eatersMap, true);
			// remove food from it
			eatersMap.getCell(startingLocation).removeAllByProperty(Names.kPropertyEdible);
			
			resetPlayer(player, resetDuringRun);
		}
		
		updatePlayers();
	}
	
	public void update() throws Exception {
		
		String restartMessage = null;
		
		Soar2D.config.generalConfig().hidemap = false;
		
		// Collect human input
		Iterator<Player> humanPlayerIter = players.humanIterator();
		if (Soar2D.config.generalConfig().force_human) {
			humanPlayerIter = players.iterator();
		} else {
			humanPlayerIter = players.humanIterator();
		}
		while (humanPlayerIter.hasNext()) {
			Player human = humanPlayerIter.next();
			if (!human.getHumanMove()) {
				return;
			}
		}
		
		++data.worldCount;

		if (Soar2D.config.terminalsConfig().max_updates > 0) {
			if (data.worldCount >= Soar2D.config.terminalsConfig().max_updates) {
				String message = "Reached maximum updates, stopping.";
				if (Soar2D.control.checkRunsTerminal()) {
					if (!data.printedStats) {
						WorldUtil.dumpStats(players, true, message);
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
						WorldUtil.dumpStats(players, true, message);
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
					WorldUtil.dumpStats(players, true, "There are no points remaining.");
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
						WorldUtil.dumpStats(players, true, message);
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
					WorldUtil.dumpStats(players, true, "All of the boxes are open.");
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
				String message = player.getName() + " issued simulation stop command.";
				if (Soar2D.config.terminalsConfig().agent_command) {
					if (!data.printedStats) {
						WorldUtil.dumpStats(players, true, message);
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
				WorldUtil.dumpStats(players, true, restartMessage);
				data.printedStats = true;
			}
			
			reset();
			if (Soar2D.wm.using()) {
				Soar2D.wm.reset();
			}
			Soar2D.control.startSimulation(false, false);
		}
	}
	
	public void updatePlayers() {
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			Player player = iter.next();
			player.update(players.getLocation(player));
		}
	}

	private void moveEaters() {
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			Player player = iter.next();
			MoveInfo move = players.getMove(player);			

			if (!move.move) {
				continue;
			}

			// Calculate new location
			int [] oldLocation = players.getLocation(player);
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
					player.adjustPoints(Soar2D.config.eatersConfig().jump_penalty, "jump penalty");
				}
				players.setLocation(player, newLocation);
				
			} else {
				player.adjustPoints(Soar2D.config.eatersConfig().wall_penalty, "wall collision");
			}
		}
	}

	private String updateMapAndEatFood() {
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			Player player = iter.next();
			MoveInfo lastMove = players.getMove(player);
			int [] location = players.getLocation(player);
			
			if (lastMove.move || lastMove.jump) {
				eatersMap.getCell(location).setPlayer(player);

				ArrayList<CellObject> moveApply = eatersMap.getCell(location).getAllWithProperty(Names.kPropertyMoveApply);
				if (moveApply != null) {
					for (CellObject object : moveApply) {
						if (object.apply(player)) {
							eatersMap.getCell(location).removeObject(object.getName());
						}
					}
				}
			}
			
			if (!lastMove.dontEat) {
				eat(player, location);
			}
			
			if (lastMove.open) {
				return open(player, location, lastMove.openCode);
			}
		}
		return null;
	}
	
	private String open(Player player, int [] location, int openCode) {
		ArrayList<CellObject> boxes = eatersMap.getCell(location).getAllWithProperty(Names.kPropertyBox);
		if (boxes == null) {
			logger.warn(player.getName() + " tried to open but there is no box.");
			return null;
		}

		// TODO: multiple boxes
		assert boxes.size() <= 1;
		
		CellObject box = boxes.get(0);
		if (box.hasProperty(Names.kPropertyStatus)) {
			if (box.getProperty(Names.kPropertyStatus).equalsIgnoreCase(Names.kOpen)) {
				logger.warn(player.getName() + " tried to open an open box.");
				return null;
			}
		}
		if (openCode != 0) {
			box.addProperty(Names.kPropertyOpenCode, Integer.toString(openCode));
		}
		if (box.apply(player)) {
			eatersMap.getCell(location).removeObject(box.getName());
		}
		
		if (box.getResetApply()) {
			String message = "Max resets achieved.";
			if (Soar2D.control.checkRunsTerminal()) {
				if (!data.printedStats) {
					WorldUtil.dumpStats(players, true, message);
				}
			} else {
				return message;
			}
		}
		return null;
	}
	
	private void eat(Player player, int [] location) {
		ArrayList<CellObject> list = eatersMap.getCell(location).getAllWithProperty(Names.kPropertyEdible);
		if (list != null) {
			for (CellObject food : list) {
				if (food.apply(player)) {
					// if this returns true, it is consumed
					eatersMap.getCell(location).removeObject(food.getName());
				}
			}
		}			
	}
	
	private ArrayList<ArrayList<Player>> findCollisions(PlayersManager players) {
		ArrayList<ArrayList<Player>> collisions = new ArrayList<ArrayList<Player>>(players.numberOfPlayers() / 2);

		// Make sure collisions are possible
		if (players.numberOfPlayers() < 2) {
			return collisions;
		}
		
		// Optimization to not check the same name twice
		HashSet<Player> colliding = new HashSet<Player>(players.numberOfPlayers());
		ArrayList<Player> collision = new ArrayList<Player>(players.numberOfPlayers());

		ListIterator<Player> leftIter = players.listIterator();
		while (leftIter.hasNext()) {
			Player left = leftIter.next();
			
			// Check to see if we're already colliding
			if (colliding.contains(left)) {
				continue;
			}
			
			ListIterator<Player> rightIter = players.listIterator(leftIter.nextIndex());
			// Clear collision list now
			collision.clear();
			while (rightIter.hasNext()) {
				// For each player to my right (I've already checked to my left)
				Player right = rightIter.next();

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
				collisions.add(new ArrayList<Player>(collision));
			}
		}

		return collisions;
	}
		
	private void handleEatersCollisions(ArrayList<ArrayList<Player>> collisions) throws Exception {
		
		// if there are no total collisions, we're done
		if (collisions.size() < 1) {
			return;
		}

		ArrayList<Player> collision = new ArrayList<Player>(players.numberOfPlayers());
		
		Iterator<ArrayList<Player>> collisionIter = collisions.iterator();
		while (collisionIter.hasNext()) {
			collision = collisionIter.next();

			assert collision.size() > 0;
			logger.debug("Processing collision group with " + collision.size() + " collidees.");

			// Redistribute wealth
			int cash = 0;			
			ListIterator<Player> collideeIter = collision.listIterator();
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
				Player player = collideeIter.next();
				int [] location = players.putInStartingLocation(player, eatersMap, false);
				player.fragged();
				if (!players.getMove(player).dontEat) {
					eat(player, location);
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

	public void resetPlayer(Player player, boolean resetDuringRun) {
		// This is here because the TOSCA stuff wants to keep around the reward
		// in the beginning of the next phase
		
		if (resetDuringRun) {
			player.mapReset();
		}
		
		player.reset();
	}

	public void addPlayer(Player player, int [] suggestedInitialLocation, boolean human) throws Exception {
		players.add(player, eatersMap, suggestedInitialLocation, human);

		int [] location = players.putInStartingLocation(player, eatersMap, true);
		// remove food from it
		eatersMap.getCell(location).removeAllByProperty(Names.kPropertyEdible);

		logger.info(player.getName() + ": Spawning at (" + location[0] + "," + location[1] + ")");
		
		updatePlayers();
	}
	
	public PlayersManager getPlayers() {
		return players;
	}

	public boolean isTerminal() {
		return data.printedStats;
	}

	public void removeAllPlayers() {
		while (players.size() != 0) {
			removePlayer(players.get(0).getName());
		}
	}

	public void removePlayer(String name) {
		Player player = players.get(name);
		eatersMap.getCell(players.getLocation(player)).setPlayer(null);
		players.remove(player);
		updatePlayers();
	}
}
