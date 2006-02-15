package eaters;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.PaintEvent;
import org.eclipse.swt.events.PaintListener;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.widgets.Canvas;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Display;

import utilities.Logger;

public class VisualWorld extends Canvas implements PaintListener {

	protected Logger m_Logger = Logger.logger;
	
	protected Display m_Display;
	protected EatersSimulation m_Simulation;
	protected int m_CellSize = 20;
	
	public VisualWorld(Composite parent, EatersSimulation simulation) {
		super(parent, SWT.NONE);
		m_Display = parent.getDisplay();
		m_Simulation = simulation;
		addPaintListener(this);
	}

	public void paintControl(PaintEvent e){
		World world = m_Simulation.getWorld();
		GC gc = e.gc;
		Point location = new Point(0,0);
		
        gc.setForeground(EatersWindowManager.black);
		gc.setLineWidth(1);

		// Draw world
		for(location.x = 0; location.x < world.getWidth(); ++location.x){
			for(location.y = 0; location.y < world.getHeight(); ++location.y){
				if (world.isWall(location)) {
				    gc.setBackground(EatersWindowManager.black);
				    gc.fillRectangle(m_CellSize*location.x + 1, m_CellSize*location.y + 1, m_CellSize - 2, m_CellSize - 2);
					
				} else if (world.isEmpty(location)) {
					gc.setBackground(EatersWindowManager.widget_background);
					gc.fillRectangle(m_CellSize*location.x, m_CellSize*location.y, m_CellSize, m_CellSize);
					
				} else {
				
					FoodInfo info = world.getFoodInfo(location);
					
					// TODO: change color depending on info
					gc.setBackground(EatersWindowManager.blue);
					
					int fill1;
					int fill2;
					switch (info.getShape()) {
					case FoodInfo.Round:
						fill1 = (int)(m_CellSize/2.8);
						fill2 = m_CellSize - fill1 + 1;
						gc.fillOval(m_CellSize*location.x + fill1, m_CellSize*location.y + fill1, m_CellSize - fill2, m_CellSize - fill2);
						gc.drawOval(m_CellSize*location.x + fill1, m_CellSize*location.y + fill1, m_CellSize - fill2 - 1, m_CellSize - fill2 - 1);
						break;
					case FoodInfo.Square:
						fill1 = (int)(m_CellSize/2.8);
						fill2 = m_CellSize - fill1 + 1;
						gc.fillRectangle(m_CellSize*location.x + fill1, m_CellSize*location.y + fill1, m_CellSize - fill2, m_CellSize - fill2);
						gc.drawRectangle(m_CellSize*location.x + fill1, m_CellSize*location.y + fill1, m_CellSize - fill2, m_CellSize - fill2);
						break;
					default:
						m_Logger.log("Invalid food shape.");
						System.exit(1);
						break;
					}
				}
			}
		}
		
		// TODO: Draw eaters
	}
}
