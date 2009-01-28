package soar2d.players;

import soar2d.map.TankSoarMap;

public interface TankCommander {
	public void update(TankSoarMap tankSoarMap) throws Exception;
	public CommandInfo getCommand() throws Exception;
	public void reset() throws Exception;
	public void shutdown() throws Exception;
	public void playersChanged(Player[] players) throws Exception;
	public void commit() throws Exception;
	public void fragged();
}
