package edu.umich.soar.gridmap2d.core.events;

import edu.umich.soar.gridmap2d.map.Player;

public class PlayerRemovedEvent extends AbstractPlayerEvent {

	PlayerRemovedEvent(Player player) {
		super(player);
	}
}
