package soar2d.world;

import soar2d.config.PlayerConfig;
import soar2d.map.GridMap;
import soar2d.players.Player;

public interface World {
	// simulation
	public void update(int worldCount) throws Exception;
	public void reset() throws Exception;
	public void setMap(String mapPath) throws Exception;
	public GridMap getMap();
	
	// player management
	public int numberOfPlayers();
	public void addPlayer(String playerId, PlayerConfig playerConfig, boolean debug) throws Exception;
	public void removePlayer(String name) throws Exception;
	public Player[] getPlayers();
	
	// control
	public boolean isTerminal();
	public void interrupted(String agentName) throws Exception;
}
