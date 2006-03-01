package tanksoar.visuals;

import java.util.*;

import org.eclipse.swt.events.*;
import org.eclipse.swt.graphics.*;
import org.eclipse.swt.widgets.*;

import simulation.*;
import simulation.visuals.*;
import tanksoar.*;

public class TankSoarVisualWorld extends VisualWorld implements PaintListener {
	static Image[] kTanks = new Image[4];
	static Image[] kRocks = new Image[3];
	static Image[] kTrees = new Image[3];
	static Image[] kGrass = new Image[3];
	static Image kRecharger;
	static Image kHealth;
	static Image kMissiles;
	static Image kMissile;
	static Image kWTF;
	static Image kRedRecharge;
	static Image kBlueRecharge;
	static Image kExplosion;
	
	private TankSoarSimulation m_Simulation;
	private TankSoarWorld m_World;
	private Point m_AgentLocation;
	private Random m_Random;
	
	public TankSoarVisualWorld(Composite parent, int style, TankSoarSimulation simulation, int cellSize) {
		super(parent, style, simulation, cellSize);
		
		m_Simulation = simulation;
		m_World = m_Simulation.getTankSoarWorld();
		m_Random = new Random();

		loadImages(parent.getDisplay());
		addPaintListener(this);		
	}
	
	private void loadImages(Display display) {
		kTanks[WorldEntity.kSouthInt] = new Image(display, TankSoar.class.getResourceAsStream("/images/tank_down.gif"));
		kTanks[WorldEntity.kNorthInt] = new Image(display, TankSoar.class.getResourceAsStream("/images/tank_up.gif"));
		kTanks[WorldEntity.kEastInt] = new Image(display, TankSoar.class.getResourceAsStream("/images/tank_right.gif"));
		kTanks[WorldEntity.kWestInt] = new Image(display, TankSoar.class.getResourceAsStream("/images/tank_left.gif"));
		kRecharger = new Image(display, TankSoar.class.getResourceAsStream("/images/battery.gif"));
		kHealth = new Image(display, TankSoar.class.getResourceAsStream("/images/health.gif"));
		kMissiles = new Image(display, TankSoar.class.getResourceAsStream("/images/missile.gif"));
		kMissile = new Image(display, TankSoar.class.getResourceAsStream("/images/fire4.gif"));
		kWTF = new Image(display, TankSoar.class.getResourceAsStream("/images/wtf.gif"));
		kRedRecharge = new Image(display, TankSoar.class.getResourceAsStream("/images/recharge.gif"));
		kBlueRecharge = new Image(display, TankSoar.class.getResourceAsStream("/images/battrecharge.gif"));
		kExplosion = new Image(display, TankSoar.class.getResourceAsStream("/images/explosion.gif"));
		
		loadNumberedImage(display, kRocks, "rock");
		loadNumberedImage(display, kTrees, "tree");
		loadNumberedImage(display, kGrass, "ground");
	}
	
	private void loadNumberedImage(Display display, Image[] images, String name){
		for(int i = 0; i < images.length; i++){
			images[i] = new Image(display, TankSoar.class.getResourceAsStream("/images/" + name + (i+1) + ".gif"));
		}
	}
	
	Tank getTankAtPixel(int x, int y) {
		x /= m_CellSize;
		y /= m_CellSize;
		TankSoarWorld.TankSoarCell cell = m_World.getCell(x, y);
		if (cell.isTank()) {
			return cell.getTank();
		}
		return null;
	}

	public void paintControl(PaintEvent e){
		if (m_AgentLocation != null || m_LastX != e.x || m_LastY != e.y || internalRepaint) {
			m_LastX = e.x;
			m_LastY = e.y;
			setRepaint();
		}
		
		GC gc = e.gc;		
        gc.setForeground(WindowManager.black);
		gc.setLineWidth(1);

		if (m_Disabled || !m_Painted) {
			gc.setBackground(WindowManager.widget_background);
			gc.fillRectangle(0,0, this.getWidth(), this.getHeight());
			if (m_Disabled) {
				m_Painted = true;
				return;
			}
		}
		
		// Draw world
		for(int x = 0; x < m_World.getWidth(); ++x){
			for(int y = 0; y < m_World.getHeight(); ++y){
				TankSoarWorld.TankSoarCell cell = m_World.getCell(x, y);
				if (!cell.isModified() && m_Painted) {
					continue;
				}
				
				if (cell.isWall()) {
					if (!m_Painted && ((x == 0) || (x == m_World.getWidth() - 1) || (y == 0) || (y == m_World.getHeight() - 1))) {
						gc.drawImage(kRocks[m_Random.nextInt(kTrees.length)], x*m_CellSize, y*m_CellSize);						
					} else {
						gc.drawImage(kTrees[m_Random.nextInt(kTrees.length)], x*m_CellSize, y*m_CellSize);
					}
				} else if (cell.isEmpty()) {
					gc.drawImage(kGrass[m_Random.nextInt(kGrass.length)], x*m_CellSize, y*m_CellSize);
				} else if (cell.isTank()) {
					gc.drawImage(kTanks[m_Random.nextInt(kTanks.length)], x*m_CellSize, y*m_CellSize);
				} else {
					m_Logger.log("Unknown cell at " + x + "," + y);
				}
			}
		}
		m_Painted = true;
	}
}
