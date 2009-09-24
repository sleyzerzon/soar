package edu.umich.soar.gridmap2d.core;

public enum Game {
	TANKSOAR, 
	EATERS, 
	TAXI,
	ROOM;
	
	public String id() {
		return this.toString().toLowerCase();
	}
}
