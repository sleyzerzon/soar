package soar2d.players;

import soar2d.map.TankSoarMap;

public interface TankCommander extends Commander {
	public void update(TankSoarMap tankSoarMap) throws Exception;
	public void playersChanged(Player[] players) throws Exception;
	public void commit() throws Exception;
	public void fragged();
}
