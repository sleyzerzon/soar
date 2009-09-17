package edu.umich.soar.gridmap2d.map;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;
import java.util.Set;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.commsen.stopwatch.Report;
import com.commsen.stopwatch.Stopwatch;

import edu.umich.soar.gridmap2d.config.PlayerConfig;
import edu.umich.soar.gridmap2d.config.SimConfig;
import edu.umich.soar.gridmap2d.core.Direction;
import edu.umich.soar.gridmap2d.core.Names;
import edu.umich.soar.gridmap2d.core.Simulation;


public class EatersWorld implements World {
	private static Log logger = LogFactory.getLog(EatersWorld.class);

	private EatersMap map;
	private final PlayersManager<Eater, EaterCommand> players = new PlayersManager<Eater, EaterCommand>();
	private final List<String> stopMessages = new ArrayList<String>();
	private final Simulation sim;
	
	public EatersWorld(Simulation sim) {
		this.sim = sim;
	}
	
	@Override
	public void setAndResetMap(String mapPath) {
		SimConfig config = sim.getConfig();
		EatersMap newMap = EatersMap.generateInstance(mapPath, config.terminalsConfig().unopened_boxes, config.eatersConfig().low_probability, config.eatersConfig().high_probability); 
		if (newMap == null) {
			return;
		}
		map = newMap;
		resetState();
	}
	
	@Override
	public void reset() {
		map.reset();
		resetState();
	}
	
	private void resetState() {
		stopMessages.clear();
		resetPlayers();
	}
	
	/**
	 * @throws IllegalStateException If there are now empty locations available to respawn players
	 */
	private void resetPlayers() {
		if (players.numberOfPlayers() == 0) {
			return;
		}
		
		for (Eater eater : players.getAll()) {
			// find a suitable starting location
			int [] location = WorldUtil.getStartingLocation(map, players.getInitialLocation(eater));
			if (location == null) {
				throw new IllegalStateException("no empty locations available for spawn");
			}
			players.setLocation(eater, location);

			// remove food from it
			map.getCell(location).removeAllObjectsByProperty(Names.kPropertyEdible);
			
			// put the player in it
			assert map.getCell(location).hasPlayers() == false;
			map.getCell(location).addPlayer(eater);

			eater.reset();
		}
		
		setLocations();
	}

	private void checkPointsRemaining() {
		if (sim.getConfig().terminalsConfig().points_remaining) {
			if (map.getScoreCount() <= 0) {
				stopMessages.add("There are no points remaining.");
			}
		}
	}
	
	private void checkFoodRemaining() {
		if (sim.getConfig().terminalsConfig().food_remaining) {
			if (map.getFoodCount() <= 0) {
				stopMessages.add("All the food is gone.");
			}
		}
	}
	
	private void checkUnopenedBoxes() {
		if (sim.getConfig().terminalsConfig().unopened_boxes) {
			if (map.getUnopenedBoxCount() <= 0) {
				stopMessages.add("All of the boxes are open.");
			}
		}
	}

	@Override
	public void update(int worldCount) {
		WorldUtil.checkNumPlayers(sim, players.numberOfPlayers());

		// Collect input
		for (Eater eater : players.getAll()) {
			eater.resetPointsChanged();

			EaterCommand command = eater.getCommand();
			if (command == null) {
				sim.stop();
				return;
			}
			players.setCommand(eater, command);
			WorldUtil.checkStopSim(sim, stopMessages, command.isStopSim(), eater);
		}
		
		moveEaters();
		// TODO: what is this for
		if (sim.isShuttingDown()) {
			return;
		}
		
		updateMapAndEatFood();
		
		handleEatersCollisions(findCollisions(players));	
		setLocations();
		map.updateObjects();

		checkPointsRemaining();
		checkFoodRemaining();
		checkUnopenedBoxes();
		WorldUtil.checkMaxUpdates(sim, stopMessages, worldCount);
		WorldUtil.checkWinningScore(sim, stopMessages, players.getSortedScores());
		
		if (stopMessages.size() > 0) {

			for (Report report : Stopwatch.getAllReports()) {
				System.out.println(report);
			}

			sim.stop();
		}
	}
	
	private void setLocations() {
		for (Eater eater : players.getAll()) {
			eater.setLocation(players.getLocation(eater));
		}
	}

	private void moveEaters() {
		for (Eater eater : players.getAll()) {
			EaterCommand command = players.getCommand(eater);			

			if (!command.isMove()) {
				continue;
			}

			// Calculate new location
			int [] oldLocation = players.getLocation(eater);
			int [] newLocation = Arrays.copyOf(oldLocation, oldLocation.length);
			Direction.translate(newLocation, command.getMoveDirection());
			if (command.isJump()) {
				Direction.translate(newLocation, command.getMoveDirection());
			}
			
			// Verify legal move and commit move
			if (map.isInBounds(newLocation) && !map.getCell(newLocation).hasObjectWithProperty(Names.kPropertyBlock)) {
				// remove from cell
				map.getCell(oldLocation).removePlayer(eater);
				
				if (command.isJump()) {
					eater.adjustPoints(sim.getConfig().eatersConfig().jump_penalty, "jump penalty");
				}
				players.setLocation(eater, newLocation);
				if (logger.isTraceEnabled()) {
					logger.trace(eater + " from " + Arrays.toString(oldLocation) + " to " + Arrays.toString(newLocation));
				}
			} else {
				eater.adjustPoints(sim.getConfig().eatersConfig().wall_penalty, "wall collision");
			}
		}
	}

	private void updateMapAndEatFood() {
		for (Eater eater : players.getAll()) {
			EaterCommand lastCommand = players.getCommand(eater);
			int [] location = players.getLocation(eater);
			
			if (lastCommand.isMove() || lastCommand.isJump()) {
				map.getCell(location).addPlayer(eater);

				Set<CellObject> moveApply = map.getCell(location).getAllObjectsWithProperty(Names.kPropertyMoveApply);
				for (CellObject object : moveApply) {
					if (apply(object, eater)) {
						map.getCell(location).removeObject(object);
					}
				}
			}
			
			if (!lastCommand.isDontEat()) {
				eat(eater, location);
			}
			
			if (lastCommand.isOpen()) {
				open(eater, location);
			}
		}
	}
	
	private boolean apply(CellObject object, Eater eater) {
		object.doApplyProperties();
		
		if (object.hasProperty("apply.points")) {
			int points = object.getProperty("apply.points", 0, Integer.class);
			eater.adjustPoints(points, object.getProperty("name"));
		}
		if (object.getProperty("apply.reward", false, Boolean.class)) {
			// am I the positive box
			if (object.getProperty("apply.reward.correct", false, Boolean.class)) {
				// reward positively
				eater.adjustPoints(object.getProperty("apply.reward.positive", 0, Integer.class), "positive reward");
			} else {
				// I'm  not the positive box, set resetApply false
				object.removeProperty("apply.reset");
				
				// reward negatively
				eater.adjustPoints(-1 * object.getProperty("apply.reward.positive", 0, Integer.class), "negative reward (wrong box)");
			}
		}
		
		return object.getProperty("apply.remove", false, Boolean.class);
	}
	
	private void open(Eater eater, int [] location) {
		CellObject box = map.getCell(location).getFirstObjectWithProperty(Names.kPropertyBox);
		if (box == null) {
			logger.warn(eater.getName() + " tried to open but there is no box.");
			return;
		}

		if (box.hasProperty(Names.kPropertyStatus)) {
			if (box.getProperty(Names.kPropertyStatus).equalsIgnoreCase(Names.kOpen)) {
				logger.warn(eater.getName() + " tried to open an open box.");
			}
		}
		if (apply(box, eater)) {
			map.getCell(location).removeObject(box);
		}
		checkResetApply(box);
	}

	private void checkResetApply(CellObject box) {
		if (box.getProperty("apply.reset", false, Boolean.class)) {
			stopMessages.add(box.getProperty("name") + " called for reset.");
		}
	}
	
	private void eat(Eater eater, int [] location) {
		for (CellObject food : map.getCell(location).getAllObjectsWithProperty(Names.kPropertyEdible)) {
			if (apply(food, eater)) {
				// if this returns true, it is consumed
				map.getCell(location).removeObject(food);
			}
		}			
	}
	
	private List<List<Eater>> findCollisions(PlayersManager<Eater, EaterCommand> eaters) {
		List<List<Eater>> collisions = new ArrayList<List<Eater>>(players.numberOfPlayers() / 2);

		// Make sure collisions are possible
		if (players.numberOfPlayers() < 2) {
			return collisions;
		}
		
		// Optimization to not check the same name twice
		Set<Eater> colliding = new HashSet<Eater>(players.numberOfPlayers());
		List<Eater> collision = new ArrayList<Eater>(players.numberOfPlayers());

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
				if (Arrays.equals(players.getLocation(left), players.getLocation(right))) {
					
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
		
	/**
	 * @param collisions
	 * @throws IllegalStateException If there are no available locations to spawn the eaters.
	 */
	private void handleEatersCollisions(List<List<Eater>> collisions) {
		
		// if there are no total collisions, we're done
		if (collisions.size() < 1) {
			return;
		}

		List<Eater> collision = new ArrayList<Eater>(players.numberOfPlayers());
		
		Iterator<List<Eater>> collisionIter = collisions.iterator();
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
			map.getCell(collisionLocation).clearPlayers();
			
			// Move to new cell, consume food
			collideeIter = collision.listIterator();
			while (collideeIter.hasNext()) {
				Eater eater = collideeIter.next();
				
				int [] location = WorldUtil.getStartingLocation(map, null);
				if (location == null) {
					throw new IllegalStateException("no empty locations available for spawn");
				}
				
				players.setLocation(eater, location);

				// put the player in it
				assert map.getCell(location).hasPlayers() == false;
				map.getCell(location).addPlayer(eater);
				
				eater.setFragged(true);
				if (!players.getCommand(eater).isDontEat()) {
					eat(eater, location);
				}
			}
		}
	}

	private void setExplosion(int[] xy) {
		CellObject explosion = map.createObjectByName(Names.kExplosion);
		explosion.setProperty("update.linger", 2);
		map.getCell(xy).addObject(explosion);
	}
	

	@Override
	public boolean isTerminal() {
		return stopMessages.size() > 0;
	}

	@Override
	public void removePlayer(String name) {
		Eater eater = players.get(name);
		map.getCell(players.getLocation(eater)).clearPlayers();
		players.remove(eater);
		eater.shutdownCommander();
		setLocations();
	}
	
	@Override
	public void interrupted(String agentName) {
		players.interrupted(agentName);
		stopMessages.add("interrupted");
	}

	@Override
	public int numberOfPlayers() {
		return players.numberOfPlayers();
	}

	@Override
	public boolean hasPlayer(String name) {
		return players.get(name) != null;
	}
	
	@Override
	public Player addPlayer(PlayerConfig cfg) {
		int [] location = WorldUtil.getStartingLocation(map, cfg.pos);
		if (location == null) {
			sim.error("There are no suitable starting locations.");
			return null;
		}
		
		List<EaterCommand> script = null;
		if (cfg.script != null) {
			try {
				script = EaterCommands.loadScript(cfg.script);
			} catch (IOException e) {
				sim.error("IOException loading script " + cfg.script);
				return null;
			}
		}

		
		Eater player = new Eater(sim, cfg.name, cfg.color, cfg.facing);
		players.add(player, cfg.pos);
		
		if (cfg.productions != null) {
			EaterCommander cmdr = sim.getCogArch().createEaterCommander(player, cfg.productions, sim.getConfig().eatersConfig().vision, cfg.shutdown_commands);
			if (cmdr == null) {
				players.remove(player);
				return null;
			}
			player.setCommander(cmdr);
		} else if (cfg.script != null) {
			assert script != null;
			player.setCommander(new ScriptedEater(script));
		}

		players.setLocation(player, location);
		
		// remove food from it
		map.getCell(location).removeAllObjectsByProperty(Names.kPropertyEdible);
		assert map.getCell(location).hasObjectWithProperty(Names.kPropertyEdible) == false;

		// put the player in it
		assert map.getCell(location).hasPlayers() == false;
		map.getCell(location).addPlayer(player);
		
		logger.info(player.getName() + ": Spawning at (" + location[0] + "," + location[1] + ")");
		
		setLocations();
		return player;
	}

	@Override
	public GridMap getMap() {
		return map;
	}

	@Override
	public List<? extends Player> getPlayers() {
		return players.getAllAsPlayers();
	}
}
