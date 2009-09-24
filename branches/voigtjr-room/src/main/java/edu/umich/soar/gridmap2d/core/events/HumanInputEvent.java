package edu.umich.soar.gridmap2d.core.events;

import edu.umich.soar.gridmap2d.map.Robot;


public class HumanInputEvent extends AbstractPlayerEvent {

	HumanInputEvent(Robot player) {
		super(player);
	}
}
