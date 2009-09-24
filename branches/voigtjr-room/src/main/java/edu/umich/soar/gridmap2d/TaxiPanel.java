package edu.umich.soar.gridmap2d;

import java.awt.Graphics;

public class TaxiPanel extends GridMapPanel {
	
	private static final long serialVersionUID = -769695946832022941L;

	private final int cellSize = 20; // FIXME

	public TaxiPanel(Adaptable app) {
	}
	
	@Override
	protected void paintComponent(Graphics g) {
	}

	@Override
	int getCellSize() {
		return cellSize;
	}
}
