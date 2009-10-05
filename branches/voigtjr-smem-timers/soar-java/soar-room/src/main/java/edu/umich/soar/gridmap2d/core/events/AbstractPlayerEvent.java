package edu.umich.soar.gridmap2d.core.events;

import edu.umich.soar.gridmap2d.events.SimEvent;
import edu.umich.soar.gridmap2d.map.Robot;

public abstract class AbstractPlayerEvent implements SimEvent {
	private final Robot player;
	
	AbstractPlayerEvent(Robot player) {
		this.player = player;
	}
	
	public Robot getPlayer() {
		return player;
	}
}
