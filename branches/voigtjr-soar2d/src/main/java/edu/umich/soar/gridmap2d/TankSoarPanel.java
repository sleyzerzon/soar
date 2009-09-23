package edu.umich.soar.gridmap2d;

import java.awt.Graphics;

public class TankSoarPanel extends GridMapPanel {
	
	private static final long serialVersionUID = 7395715914166787141L;

	private final int cellSize = 20; // FIXME

	public TankSoarPanel(Adaptable app) {
	}
	
	@Override
	protected void paintComponent(Graphics g) {
	}

	@Override
	int getCellSize() {
		return cellSize;
	}
}
