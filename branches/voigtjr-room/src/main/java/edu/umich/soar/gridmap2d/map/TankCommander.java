package edu.umich.soar.gridmap2d.map;

import java.util.List;


public interface TankCommander extends Commander {
	public void playersChanged(List<? extends Player> players);
	public void fragged();
	public TankCommand nextCommand();
}
