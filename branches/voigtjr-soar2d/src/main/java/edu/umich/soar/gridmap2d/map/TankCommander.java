package edu.umich.soar.gridmap2d.map;


public interface TankCommander extends Commander {
	public void playersChanged(Player[] players);
	public void fragged();
	public TankCommand nextCommand();
}
