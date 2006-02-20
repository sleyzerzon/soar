package eaters;

import org.eclipse.swt.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.graphics.*;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.layout.*;

import utilities.Logger;

public class VisualWorld extends Canvas implements PaintListener, SimulationListener {
	protected Logger m_Logger = Logger.logger;
	
	protected Display m_Display;
	protected EatersSimulation m_Simulation;
	protected int m_CellSize;
	
	protected Point m_AgentLocation;
	
	public VisualWorld(Composite parent, EatersSimulation simulation, int cellSize) {
		super(parent, SWT.NONE);
		
		m_Display = parent.getDisplay();
		m_Simulation = simulation;
		m_CellSize = cellSize;
		addPaintListener(this);
		m_Simulation.addSimulationListener(this);
		
		setLayoutData(new RowData(getWidth(), getHeight()));
	}
	
	public int getWidth() {
		return m_CellSize * m_Simulation.getWorld().getWidth();
	}
	
	public int getHeight() {
		return m_CellSize * m_Simulation.getWorld().getHeight();
	}
	
	public void setAgentLocation(Point location) {
		m_AgentLocation = location;
	}

	public void paintControl(PaintEvent e){
		World world = m_Simulation.getWorld();
		GC gc = e.gc;		
        gc.setForeground(EatersWindowManager.black);
		gc.setLineWidth(1);

		// Draw world
		int fill1, fill2, xDraw, yDraw;
		for(int x = 0; x < world.getWidth(); ++x){
			if (m_AgentLocation != null) {
				if ((x < m_AgentLocation.x - Eater.kEaterVision) || (x > m_AgentLocation.x + Eater.kEaterVision)) {
					continue;
				} 
				xDraw = x + Eater.kEaterVision - m_AgentLocation.x;
			} else {
				xDraw = x;
			}
			
			for(int y = 0; y < world.getHeight(); ++y){
				if (m_AgentLocation != null) {
					if ((y < m_AgentLocation.y - Eater.kEaterVision) || (y > m_AgentLocation.y + Eater.kEaterVision)) {
						continue;
					} 
					yDraw = y + Eater.kEaterVision - m_AgentLocation.y;
				} else {
					yDraw = y;
				}
				
				if (world.isWall(x, y)) {
				    gc.setBackground(EatersWindowManager.black);
				    gc.fillRectangle(m_CellSize*xDraw + 1, m_CellSize*yDraw + 1, m_CellSize - 2, m_CellSize - 2);
					
				} else if (world.isEmpty(x, y)) {
					gc.setBackground(EatersWindowManager.widget_background);
					gc.fillRectangle(m_CellSize*xDraw, m_CellSize*yDraw, m_CellSize, m_CellSize);
					
				} else if (world.isEater(x, y)) {
					gc.setBackground(EatersWindowManager.green);
					gc.fillRectangle(m_CellSize*xDraw, m_CellSize*yDraw, m_CellSize, m_CellSize);
					
				} else {
				
					World.FoodInfo info = world.getFoodInfo(x, y);
					
					gc.setBackground(info.getColor());
					
					if (info.getShape().equalsIgnoreCase(World.FoodInfo.kRound)) {
						fill1 = (int)(m_CellSize/2.8);
						fill2 = m_CellSize - fill1 + 1;
						gc.fillOval(m_CellSize*xDraw + fill1, m_CellSize*yDraw + fill1, m_CellSize - fill2, m_CellSize - fill2);
						gc.drawOval(m_CellSize*xDraw + fill1, m_CellSize*yDraw + fill1, m_CellSize - fill2 - 1, m_CellSize - fill2 - 1);
					} else if (info.getShape().equalsIgnoreCase(World.FoodInfo.kSquare)) {
						fill1 = (int)(m_CellSize/2.8);
						fill2 = m_CellSize - fill1 + 1;
						gc.fillRectangle(m_CellSize*xDraw + fill1, m_CellSize*yDraw + fill1, m_CellSize - fill2, m_CellSize - fill2);
						gc.drawRectangle(m_CellSize*xDraw + fill1, m_CellSize*yDraw + fill1, m_CellSize - fill2, m_CellSize - fill2);
					} else {
						m_Logger.log("Invalid food shape.");
						m_Simulation.shutdown(1);
					}
				}
			}
		}
	}
	
	public void simulationEventHandler(int type, Object object) {
		if (type == SimulationListener.kUpdateEvent) {
			m_Display.asyncExec(new Runnable() { 
				public void run () { 
					redraw(); 
				} 
			});
		}
	}
}
