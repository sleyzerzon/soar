package soar2d.world;

import soar2d.config.PlayerConfig;
import soar2d.map.GridMap;
import soar2d.players.Player;

public interface World {
	// simulation
	public void update() throws Exception;
	public void reset() throws Exception;
	public GridMap getMap();
	public int getWorldCount();
	
	// player management
	public int numberOfPlayers();
	public void addPlayer(String playerId, PlayerConfig playerConfig) throws Exception;
	public void removePlayer(String name) throws Exception;
	public Player[] getPlayers();
	
	// control
	public boolean isTerminal();
	public void interrupted(String agentName) throws Exception;
}
