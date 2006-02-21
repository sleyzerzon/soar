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
				
				World.Cell cell = world.getCell(x, y);
				
				if (cell.isWall()) {
				    gc.setBackground(EatersWindowManager.black);
				    gc.fillRectangle(m_CellSize*xDraw + 1, m_CellSize*yDraw + 1, m_CellSize - 2, m_CellSize - 2);
					
				} else if (cell.isEmpty()) {
					gc.setBackground(EatersWindowManager.widget_background);
					gc.fillOval(m_CellSize*xDraw, m_CellSize*yDraw, m_CellSize, m_CellSize);
					
				} else if (cell.isEater()) {
					
					Eater eater = cell.getEater();
					
					gc.setBackground(eater.getColor());
					gc.fillOval(m_CellSize*xDraw, m_CellSize*yDraw, m_CellSize, m_CellSize);
					gc.setBackground(EatersWindowManager.widget_background);
					
					String facing = eater.getFacing();
					if (facing != null) {
						if (facing.equalsIgnoreCase(Eater.kNorth)) {
							drawEaterMouth(xDraw, yDraw, 1, 0, 1, 1, gc);
						} else if (facing.equalsIgnoreCase(Eater.kSouth)) {
							drawEaterMouth(xDraw, yDraw + 1, 1, 0, 1, -1, gc);
						} else if (facing.equalsIgnoreCase(Eater.kEast)) {
							drawEaterMouth(xDraw + 1, yDraw, 0, 1, -1, 1, gc);
						} else if (facing.equalsIgnoreCase(Eater.kWest)) {
							drawEaterMouth(xDraw, yDraw, 0, 1, 1, 1, gc);
						}					
					}
				} else {
				
					World.Food food = cell.getFood();
					
					gc.setBackground(food.getColor());
					
					if (food.getShape().equalsIgnoreCase(World.Food.kRound)) {
						fill1 = (int)(m_CellSize/2.8);
						fill2 = m_CellSize - fill1 + 1;
						gc.fillOval(m_CellSize*xDraw + fill1, m_CellSize*yDraw + fill1, m_CellSize - fill2, m_CellSize - fill2);
						gc.drawOval(m_CellSize*xDraw + fill1, m_CellSize*yDraw + fill1, m_CellSize - fill2 - 1, m_CellSize - fill2 - 1);
					} else if (food.getShape().equalsIgnoreCase(World.Food.kSquare)) {
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
	
	void drawEaterMouth(int x, int y, int x_mult, int y_mult, int cx_mult, int cy_mult, GC gc){		
	    switch(m_Simulation.getWorldCount() % 8){
			case(0):{
			    gc.fillPolygon(new int[]{m_CellSize*x, m_CellSize*y,
			            m_CellSize*x + x_mult*m_CellSize, m_CellSize * y + y_mult*m_CellSize,
			            m_CellSize*x + cx_mult*m_CellSize/2, m_CellSize*y + cy_mult*m_CellSize/2});
			    break;
			}
			case(1):{
			    gc.fillPolygon(new int[]{m_CellSize*x + x_mult*m_CellSize/8, m_CellSize*y + y_mult*m_CellSize/8,
			            m_CellSize*x + x_mult*(m_CellSize - m_CellSize/8), m_CellSize * y + y_mult*(m_CellSize - m_CellSize/8),
			            m_CellSize*x + cx_mult*m_CellSize/2, m_CellSize*y + cy_mult*m_CellSize/2});
			    break;
			}
			case(2):{
			    gc.fillPolygon(new int[]{m_CellSize*x + x_mult*m_CellSize/4, m_CellSize*y + y_mult*m_CellSize/4,
			            m_CellSize*x + x_mult*(m_CellSize - m_CellSize/4), m_CellSize * y + y_mult*(m_CellSize - m_CellSize/4),
			            m_CellSize*x + cx_mult * m_CellSize/2, m_CellSize*y + cy_mult * m_CellSize/2});
			    break;
			}
			case(3):{
			    gc.fillPolygon(new int[]{m_CellSize*x + x_mult * 3*m_CellSize/8, m_CellSize*y + y_mult * 3*m_CellSize/8,
			            m_CellSize*x + x_mult*(m_CellSize - 3*m_CellSize/8), m_CellSize * y + y_mult*(m_CellSize - 3*m_CellSize/8),
			            m_CellSize*x + cx_mult*m_CellSize/2, m_CellSize*y + cy_mult*m_CellSize/2});
			    break;
			}
			case(4): break;
			case(5):{
			    gc.fillPolygon(new int[]{m_CellSize*x + x_mult * 3*m_CellSize/8, m_CellSize*y + y_mult * 3*m_CellSize/8,
			            m_CellSize*x + x_mult*(m_CellSize - 3*m_CellSize/8), m_CellSize * y + y_mult*(m_CellSize - 3*m_CellSize/8),
			            m_CellSize*x + cx_mult*m_CellSize/2, m_CellSize*y + cy_mult*m_CellSize/2});
			    break;
			}
			case(6):{
			    gc.fillPolygon(new int[]{m_CellSize*x + x_mult*m_CellSize/4, m_CellSize*y + y_mult*m_CellSize/4,
			            m_CellSize*x + x_mult*(m_CellSize - m_CellSize/4), m_CellSize * y + y_mult*(m_CellSize - m_CellSize/4),
			            m_CellSize*x + cx_mult * m_CellSize/2, m_CellSize*y + cy_mult * m_CellSize/2});
			    break;
			}
			case(7):{
			    gc.fillPolygon(new int[]{m_CellSize*x + x_mult*m_CellSize/8, m_CellSize*y + y_mult*m_CellSize/8,
			            m_CellSize*x + x_mult*(m_CellSize - m_CellSize/8), m_CellSize * y + y_mult*(m_CellSize - m_CellSize/8),
			            m_CellSize*x + cx_mult*m_CellSize/2, m_CellSize*y + cy_mult*m_CellSize/2});
			    break;	
			}
		}
	}
 	
	public void simulationEventHandler(int type, Object object) {
		if (type == SimulationListener.kUpdateEvent || type == SimulationListener.kNewWorldEvent || type == SimulationListener.kStopEvent) {
			m_Display.asyncExec(new Runnable() { 
				public void run () { 
					redraw(); 
				} 
			});
		}
	}
}
