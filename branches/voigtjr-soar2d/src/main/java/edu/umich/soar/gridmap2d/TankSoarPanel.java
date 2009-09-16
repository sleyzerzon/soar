package edu.umich.soar.gridmap2d;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.SystemColor;
import java.util.Set;

import edu.umich.soar.gridmap2d.core.Names;
import edu.umich.soar.gridmap2d.core.Simulation;
import edu.umich.soar.gridmap2d.core.events.AfterTickEvent;
import edu.umich.soar.gridmap2d.core.events.ResetEvent;
import edu.umich.soar.gridmap2d.map.CellObject;
import edu.umich.soar.gridmap2d.map.Eater;
import edu.umich.soar.gridmap2d.map.EatersMap;

public class TankSoarPanel extends GridMapPanel {
	
	private final Adaptable app;
	private final int cellSize = 20; // FIXME
	private int mouthCount = 0;

	public TankSoarPanel(Adaptable app) {
		this.app = app;
		
		Simulation sim = Adaptables.adapt(app, Simulation.class);
		sim.getEvents().addListener(AfterTickEvent.class, this);
		sim.getEvents().addListener(ResetEvent.class, this);
	}
	
	@Override
	protected void paintComponent(Graphics g) {
		super.paintComponent(g);

		Simulation sim = Adaptables.adapt(app, Simulation.class);
		EatersMap map = (EatersMap)sim.getMap();
		
		// Draw world
		int fill1, fill2, xDraw, yDraw;
		int [] location = new int [2];
		for(location[0] = 0; location[0] < map.size(); ++location[0]){
			xDraw = location[0];
			
			for(location[1] = 0; location[1] < map.size(); ++location[1]){
				yDraw = location[1];
				
//				if (!map.getCell(location).isModified() && painted) {
//					continue;
//				}
//				map.getCell(location).setModified(false);
				
				Set<CellObject> drawList = map.getCell(location).getAllObjectsWithProperty(Names.kPropertyShape);
				
				if (map.getCell(location).hasObjectWithProperty(Names.kPropertyBlock)) {
					g.setColor(Color.BLACK);
					g.fillRect(cellSize*xDraw + 1, cellSize*yDraw + 1, cellSize - 2, cellSize - 2);
					
				} else {
					boolean empty = true;
					
					Eater eater = (Eater)map.getCell(location).getFirstPlayer();
					
					if (eater != null) {
						empty = false;
						
						g.setColor(Colors.getColor(eater.getColor()));
						g.fillOval(cellSize*xDraw, cellSize*yDraw, cellSize, cellSize);
						
						switch (eater.getFacing()) {
						case NORTH:
							drawEaterMouth(g, xDraw, yDraw, 1, 0, 1, 1);
							break;
						case EAST:
							drawEaterMouth(g, xDraw + 1, yDraw, 0, 1, -1, 1);
							break;
						case SOUTH:
							drawEaterMouth(g, xDraw, yDraw + 1, 1, 0, 1, -1);
							break;
						case WEST:
							drawEaterMouth(g, xDraw, yDraw, 0, 1, 1, 1);
							break;
						default:
							break;
						}
					}
					
					for (CellObject object : drawList) {
						if (empty) {
							g.setColor(SystemColor.control);
							g.fillRect(cellSize*xDraw, cellSize*yDraw, cellSize, cellSize);
						}
						empty = false;
					    
					    Color color = Colors.getColor(object.getProperty(Names.kPropertyColor));
					    if (color == null) {
					    	color = Color.BLACK;
					    }
						g.setColor(color);
						
						Shape shape = Shape.getShape(object.getProperty(Names.kPropertyShape));
						if (shape != null) {
							if (shape.equals(Shape.ROUND)) {
								fill1 = (int)(cellSize/2.8);
								fill2 = cellSize - fill1 + 1;
								g.fillOval(cellSize*xDraw + fill1, cellSize*yDraw + fill1, cellSize - fill2, cellSize - fill2);
								g.setColor(Color.BLACK);
								g.drawOval(cellSize*xDraw + fill1, cellSize*yDraw + fill1, cellSize - fill2 - 1, cellSize - fill2 - 1);
								
							} else if (shape.equals(Shape.SQUARE)) {
								fill1 = (int)(cellSize/2.8);
								fill2 = cellSize - fill1 + 1;
								g.fillRect(cellSize*xDraw + fill1, cellSize*yDraw + fill1, cellSize - fill2, cellSize - fill2);
								g.setColor(Color.BLACK);
								g.drawRect(cellSize*xDraw + fill1, cellSize*yDraw + fill1, cellSize - fill2, cellSize - fill2);
							}
						}
					}
					
					if (empty) {
						g.setColor(SystemColor.window);
						g.fillRect(cellSize*xDraw, cellSize*yDraw, cellSize, cellSize);
					}
				}
				
				if (map.getCell(location).hasObjectWithProperty(Names.kExplosion)) {
					drawExplosion(g, xDraw, yDraw);
				}
			}
		}
		
		//painted = true;
	}

	private void drawExplosion(Graphics g, int x, int y) {
		g.setColor(Color.RED);
		int offCenter = cellSize/4;
		int xBase = cellSize*x;
		int yBase = cellSize*y;
		int halfCell = cellSize/2;
		
		int [] center = new int [] { xBase + halfCell, yBase + halfCell };
		int [] north = new int [] { center[0], yBase };
		int [] east = new int [] { xBase + cellSize, center[1] };
		int [] south = new int [] { center[0], yBase + cellSize };
		int [] west = new int [] { xBase, center[1] };
		
		g.fillPolygon(
				new int[] {center[0], north[0], center[0] + offCenter}, 
				new int[] {center[1], north[1], center[1]},
				3);
		g.fillPolygon(
				new int[] {center[0], east[0], center[0]},
				new int[] {center[1], east[1], center[1] + offCenter},
				3);
		g.fillPolygon(
				new int[] {center[0], south[0], center[0] - offCenter},
				new int[] {center[1], south[1], center[1]},
				3);
		g.fillPolygon(
				new int[] {center[0], west[0], center[0]},
				new int[] {center[1], west[1], center[1] - offCenter},
				3);
	}
	
	private void drawEaterMouth(Graphics g, int x, int y, int x_mult, int y_mult, int cx_mult, int cy_mult){		
		g.setColor(SystemColor.control);
	    switch(++mouthCount % 8){
			case(0):{
			    g.fillPolygon(
			    		new int[]{cellSize*x, cellSize*x + x_mult*cellSize, cellSize*x + cx_mult*cellSize/2},
			    		new int[]{cellSize*y, cellSize * y + y_mult*cellSize, cellSize*y + cy_mult*cellSize/2},
			    		3);
			    break;
			}
			case(1):{
			    g.fillPolygon(
			    		new int[]{cellSize*x + x_mult*cellSize/8, cellSize*x + x_mult*(cellSize - cellSize/8), cellSize*x + cx_mult*cellSize/2},
			            new int[]{cellSize*y + y_mult*cellSize/8, cellSize * y + y_mult*(cellSize - cellSize/8), cellSize*y + cy_mult*cellSize/2},
					    3);
			    break;
			}
			case(2):{
			    g.fillPolygon(
			    		new int[]{cellSize*x + x_mult*cellSize/4, cellSize*x + x_mult*(cellSize - cellSize/4), cellSize*x + cx_mult * cellSize/2},
			            new int[]{cellSize*y + y_mult*cellSize/4, cellSize * y + y_mult*(cellSize - cellSize/4), cellSize*y + cy_mult * cellSize/2},
			            3);
			    break;
			}
			case(3):{
			    g.fillPolygon(
			    		new int[]{cellSize*x + x_mult * 3*cellSize/8, cellSize*x + x_mult*(cellSize - 3*cellSize/8), cellSize*x + cx_mult*cellSize/2},
			            new int[]{cellSize*y + y_mult * 3*cellSize/8, cellSize * y + y_mult*(cellSize - 3*cellSize/8), cellSize*y + cy_mult*cellSize/2},
			            3);
			    break;
			}
			case(4): break;
			case(5):{
			    g.fillPolygon(
			    		new int[]{cellSize*x + x_mult * 3*cellSize/8, cellSize*x + x_mult*(cellSize - 3*cellSize/8), cellSize*x + cx_mult*cellSize/2},
			            new int[]{cellSize*y + y_mult * 3*cellSize/8, cellSize * y + y_mult*(cellSize - 3*cellSize/8), cellSize*y + cy_mult*cellSize/2},
			            3);
			    break;
			}
			case(6):{
			    g.fillPolygon(
			    		new int[]{cellSize*x + x_mult*cellSize/4, cellSize*x + x_mult*(cellSize - cellSize/4), cellSize*x + cx_mult * cellSize/2},
			            new int[]{cellSize*y + y_mult*cellSize/4, cellSize * y + y_mult*(cellSize - cellSize/4), cellSize*y + cy_mult * cellSize/2},
			            3);
			    break;
			}
			case(7):{
			    g.fillPolygon(
			    		new int[]{cellSize*x + x_mult*cellSize/8, cellSize*x + x_mult*(cellSize - cellSize/8), cellSize*x + cx_mult*cellSize/2},
			            new int[]{cellSize*y + y_mult*cellSize/8, cellSize * y + y_mult*(cellSize - cellSize/8), cellSize*y + cy_mult*cellSize/2},
			            3);
			    break;	
			}
		}
	}
}
