package edu.umich.soar.gridmap2d.map;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.ListIterator;
import java.util.Map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;


public class PlayersManager<P extends Player, T> {
	private static Log logger = LogFactory.getLog(PlayersManager.class);

	private final List<P> players = new ArrayList<P>(7);
	private final Map<String, P> playersMap = new HashMap<String, P>(7);
	private final Map<P, int []> initialLocations = new HashMap<P, int []>(7);
	private final Map<P, T> lastCommands = new HashMap<P, T>(7);
	
	public int numberOfPlayers() {
		return players.size();
	}

	List<P> getAll() {
		return players;
	}
	
	List<? extends Player> getAllAsPlayers() {
		return players;
	}
	
	ListIterator<P> listIterator() {
		return players.listIterator();
	}
	
	ListIterator<P> listIterator(int index) {
		return players.listIterator(index);
	}
	
	public T getCommand(P player) {
		return lastCommands.get(player);
	}
	
	void setCommand(P player, T move) {
		lastCommands.put(player, move);

		String moveString = move.toString();
		if (moveString.length() > 0) {
			logger.info(player.getName() + ": " + moveString);
		}
	}
	
	P get(String name) {
		return playersMap.get(name);
	}
	
	P get(int index) {
		return players.get(index);
	}
	
	int indexOf(P player) {
		return players.indexOf(player);
	}
	
	void remove(P player) {
		logger.info("Removing player " + player);
		players.remove(player);
		playersMap.remove(player.getName());
		initialLocations.remove(player);
		lastCommands.remove(player);
	}
	
	/**
	 * @param player The player to add, the player's name must be unique.
	 * @param initialLocation The player's starting location.
	 * 
	 * @throws IllegalStateException If the player name is already in use.
	 */
	void add(P player, int [] initialLocation) {
		if(playersMap.containsKey(player.getName())) {
			throw new IllegalStateException(player.getName() + " already exists");
		}

		logger.info("Adding player " + player);
		players.add(player);
		playersMap.put(player.getName(), player);
		
		if (initialLocation != null) {
			initialLocations.put(player, Arrays.copyOf(initialLocation, initialLocation.length));
		}
	}
	
	boolean hasInitialLocation(P player) {
		return initialLocations.containsKey(player);
	}

	int [] getInitialLocation(P player) {
		return initialLocations.get(player);
	}
	
	boolean exists(String name) {
		return playersMap.containsKey(name);
	}
	
	int size() {
		return players.size();
	}
	
}
