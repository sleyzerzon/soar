package soar2d.world;

import soar2d.players.Player;

public interface IWorld {
	// simulation
	public void update() throws Exception;
	public void reset() throws Exception;
	
	// player management
	public void addPlayer(Player player, int [] suggestedInitialLocation, boolean human) throws Exception;
	public void removePlayer(String name);
	public void removeAllPlayers();
	
	// control
	public boolean isTerminal();
}
