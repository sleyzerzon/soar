package edu.umich.soar.gridmap2d;

import javax.swing.JPanel;

import edu.umich.soar.gridmap2d.events.SimEvent;
import edu.umich.soar.gridmap2d.events.SimEventListener;

abstract class GridMapPanel extends JPanel implements SimEventListener {

	private static final long serialVersionUID = -6979980443605257397L;

	@Override
	public void onEvent(SimEvent event) {
		repaint();
	}
}
