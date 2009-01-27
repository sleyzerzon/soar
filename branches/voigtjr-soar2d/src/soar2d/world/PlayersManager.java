package soar2d.world;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Iterator;
import java.util.ListIterator;

import org.apache.log4j.Logger;

import soar2d.Soar2D;
import soar2d.map.GridMap;
import soar2d.players.MoveInfo;
import soar2d.players.Player;

public class PlayersManager {
	private static Logger logger = Logger.getLogger(PlayersManager.class);

	private ArrayList<Player> players = new ArrayList<Player>(7);
	private ArrayList<Player> humanPlayers = new ArrayList<Player>(7);
	private HashMap<String, Player> playersMap = new HashMap<String, Player>(7);
	private HashMap<Player, int []> initialLocations = new HashMap<Player, int []>(7);
	private HashMap<Player, int []> locations = new HashMap<Player, int []>(7);
	private HashMap<Player, double []> floatLocations = new HashMap<Player, double []>(7);
	private HashMap<Player, MoveInfo> lastMoves = new HashMap<Player, MoveInfo>(7);
	
	int numberOfPlayers() {
		return players.size();
	}

	Iterator<Player> humanIterator() {
		return humanPlayers.iterator();
	}
	
	ArrayList<Player> getAll() {
		return players;
	}
	
	Iterator<Player> iterator() {
		return players.iterator();
	}
	
	ListIterator<Player> listIterator() {
		return players.listIterator();
	}
	
	ListIterator<Player> listIterator(int index) {
		return players.listIterator(index);
	}
	
	int [] getLocation(Player player) {
		return locations.get(player);
	}
	
	void setLocation(Player player, int [] location) {
		locations.put(player, location);
	}
	
	double [] getFloatLocation(Player player) {
		return floatLocations.get(player);
	}
	
	void setFloatLocation(Player player, double [] location) {
		floatLocations.put(player, location);
	}
	
	MoveInfo getMove(Player player) {
		return lastMoves.get(player);
	}
	
	void setMove(Player player, MoveInfo move) {
		lastMoves.put(player, move);
	}
	
	Player get(String name) {
		return playersMap.get(name);
	}
	
	Player get(int index) {
		return players.get(index);
	}
	
	int indexOf(Player player) {
		return players.indexOf(player);
	}
	
	void remove(Player player) {
		logger.info("Removing player " + player);
		players.remove(player);
		humanPlayers.remove(player);
		playersMap.remove(player.getName());
		initialLocations.remove(player);
		locations.remove(player);
		floatLocations.remove(player);
		lastMoves.remove(player);
	}
	
	void add(Player player, GridMap map, int [] initialLocation, boolean human) throws Exception {
		logger.info("Adding player " + player);

		if(playersMap.containsKey(player.getName())) {
			throw new Exception("Player already exists.");
		}

		players.add(player);
		playersMap.put(player.getName(), player);
		
		if (initialLocation != null) {
			initialLocations.put(player, initialLocation);
		}
		
		if (human) {
			humanPlayers.add(player);
		}
	}
	
	boolean hasInitialLocation(Player player) {
		return initialLocations.containsKey(player);
	}

	int [] getInitialLocation(Player player) {
		return initialLocations.get(player);
	}
	
	boolean exists(Player player) {
		return playersMap.containsKey(player.getName());
	}
	
	double angleOff(Player left, Player right) {
		double [] target = new double [] { floatLocations.get(right)[0], floatLocations.get(right)[1] };
		
		return angleOff(left, target);
	}

	int size() {
		return players.size();
	}
	
	double angleOff(Player left, double [] target) {
		double [] playerVector = new double [] { floatLocations.get(left)[0], floatLocations.get(left)[1] };

		if (Soar2D.config.roomConfig().continuous == false) {
			// translate the player's location back a little bit to increase peripheral vision
			playerVector[0] -= Math.cos(left.getHeadingRadians());
			playerVector[1] -= Math.sin(left.getHeadingRadians());
		}
			
		double [] targetVector = new double [] { target[0], target[1] };
		
		// translate target so i'm the origin
		targetVector[0] -= playerVector[0];
		targetVector[1] -= playerVector[1];
		
		// make target unit vector
		double targetVectorLength = Math.sqrt(Math.pow(targetVector[0], 2) + Math.pow(targetVector[1], 2));
		if (targetVectorLength > 0) {
			targetVector[0] /= targetVectorLength;
			targetVector[1] /= targetVectorLength;
		} else {
			targetVector[0] = 0;
			targetVector[1] = 0;
		}
		
		// make player facing vector
		playerVector[0] = Math.cos(left.getHeadingRadians());
		playerVector[1] = Math.sin(left.getHeadingRadians());
		
		double dotProduct = (targetVector[0] * playerVector[0]) + (targetVector[1] * playerVector[1]);
		double crossProduct = (targetVector[0] * playerVector[1]) - (targetVector[1] * playerVector[0]);
		
		// calculate inverse cosine of that for angle
		if (crossProduct < 0) {
			return Math.acos(dotProduct);
		}
		return Math.acos(dotProduct) * -1;
	}

	int [] putInStartingLocation(Player player, GridMap map, boolean useInitialLocation) throws Exception {
		int[] location = null;
		if (useInitialLocation && hasInitialLocation(player)) {
			location = getInitialLocation(player);
			if (!map.isAvailable(location)) {
				logger.warn(player.getName() + ": Initial location (" + location[0] + "," + location[1] + ") is blocked, going random.");
				location = null;
			}
		}
		
		if (location == null) {
			location = map.getAvailableLocationAmortized();
			if (location == null) {
				throw new Exception("There are no suitable starting locations for " + player.getName() + ".");
			}
		}
		
		// put the player in it
		map.getCell(location).setPlayer(player);
		setLocation(player, location);
		return location;
	}

	int[] getSortedScores() {
		int[] scores = new int[players.size()];
		
		for (int i = 0; i < players.size(); ++i) {
			scores[i] = players.get(i).getPoints();
		}
		Arrays.sort(scores);
		return scores;
	}
}
