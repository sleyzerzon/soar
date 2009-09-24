package edu.umich.soar.gridmap2d.core.events;

import edu.umich.soar.gridmap2d.events.SimEvent;
import edu.umich.soar.gridmap2d.map.Player;

public abstract class AbstractPlayerEvent implements SimEvent {
	private final Player player;
	
	AbstractPlayerEvent(Player player) {
		this.player = player;
	}
	
	public Player getPlayer() {
		return player;
	}
}
