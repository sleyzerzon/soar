package edu.umich.soar.gridmap2d.map;


public interface EaterCommander extends Commander {
	public EaterCommand nextCommand();
	public void update(EatersMap map);
}
