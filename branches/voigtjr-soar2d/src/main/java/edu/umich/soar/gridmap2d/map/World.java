package edu.umich.soar.gridmap2d.map;


import edu.umich.soar.gridmap2d.config.PlayerConfig;

public interface World {
	// simulation
	public void update(int worldCount);
	public void reset();
	public void setAndResetMap(String mapPath);
	public GridMap getMap();
	
	// player management
	public int numberOfPlayers();
	public boolean hasPlayer(String name);
	public boolean addPlayer(String playerId, PlayerConfig playerConfig);
	public void removePlayer(String name);
	public Player[] getPlayers();
	
	// control
	public boolean isTerminal();
	public void interrupted(String agentName);
}
