package edu.umich.soar.gridmap2d.core.events;

import edu.umich.soar.gridmap2d.map.Player;

public class HumanInputEvent extends AbstractPlayerEvent {

	HumanInputEvent(Player player) {
		super(player);
	}
}
