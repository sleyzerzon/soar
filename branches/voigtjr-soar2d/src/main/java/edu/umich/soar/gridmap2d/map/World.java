package edu.umich.soar.gridmap2d.map;


import java.util.List;

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
	public Player addPlayer(PlayerConfig playerConfig);
	public void removePlayer(String name);
	public List<? extends Player> getPlayers();
	
	// control
	public boolean isTerminal();
	public void interrupted(String agentName);
}
