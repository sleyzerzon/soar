package edu.umich.soar.gridmap2d.core.events;

import edu.umich.soar.gridmap2d.map.Robot;

public class PlayerRemovedEvent extends AbstractPlayerEvent {

	public PlayerRemovedEvent(Robot player) {
		super(player);
	}
}
