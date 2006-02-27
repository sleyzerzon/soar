package tanksoar;

import java.util.*;

import org.eclipse.swt.graphics.*;

import simulation.*;
import sml.*;
import utilities.*;

public class World implements WorldManager {
	private static final String kTagTankSoarWorld = "tanksoar-world";

	private static final String kTagCells = "cells";
	private static final String kParamWorldWidth = "world-width";
	private static final String kParamWorldHeight = "world-height";
	private static final String kParamType = "type";
	
	private static final String kTypeWall = "wall";
	private static final String kTypeEmpty = "empty";
	private static final String kTypeTank = "tank";
	
	private static final int kWallInt = 0;
	private static final int kEmptyInt = 1;
	private static final int kTankInt = 2;

	public class Cell {
		private int m_Type;
		private boolean m_Collision = false;
		private boolean m_Modified = true;
		private Tank m_Tank;
		
		public Cell(String name) throws Exception {
			if (name.equalsIgnoreCase(kTypeWall)) {
				m_Type = kWallInt;
				return;
			} else if (name.equalsIgnoreCase(kTypeEmpty)) {
				m_Type = kEmptyInt;			
				return;
			} else {	
				throw new Exception("Invalid type name: " + name);
			}
		}

		void setCollision(boolean setting) {
			m_Collision = setting;
			m_Modified = true;
		}
		
		public boolean checkCollision() {
			return m_Collision;
		}
		
		void clearModified() {
			m_Modified = false;
		}
		
		public boolean removeTank() {
			if (m_Type != kTankInt) {
				return false;
			}
			m_Modified = true;
			m_Type = kEmptyInt;
			m_Tank = null;
			return true;
		}
		
		public boolean isWall() {
			return m_Type == kWallInt;
		}
		
		public boolean isEmpty() {
			return m_Type == kEmptyInt;
		}
		
		public boolean isTank() {
			return m_Type == kTankInt;
		}
		
		public void setTank(Tank tank) {
			m_Modified = true;
			// TODO: remove missiles ?
			m_Type = kTankInt;
			m_Tank = tank;
		}
	}
	
	private Logger m_Logger = Logger.logger;
	private TankSoarSimulation m_Simulation;
	private Cell[][] m_World;
	private boolean m_PrintedStats;
	private int m_WorldWidth;
	private int m_WorldHeight;
	private Tank[] m_Tanks;

	public World(TankSoarSimulation simulation) {
		m_Simulation = simulation;
	}
	
	public boolean load(String mapFile) {
		m_PrintedStats = false;
		
		try {
			// Open file
			JavaElementXML root = JavaElementXML.ReadFromFile(mapFile);
			
			if (!root.getTagName().equalsIgnoreCase(kTagTankSoarWorld)) {
				throw new Exception("Not a TankSoar map!");
			}
			// TODO: Check version
			
			// Create map
			JavaElementXML cells = root.findChildByNameThrows(kTagCells);
			
			// Get dimentions
			m_WorldWidth = cells.getAttributeIntThrows(kParamWorldWidth);
			m_WorldHeight = cells.getAttributeIntThrows(kParamWorldHeight);
			
			// Create map array
			m_World = new Cell[m_WorldHeight][m_WorldWidth];
			
			// generate world
			generateWorldFromXML(cells);

		} catch (Exception e) {
			m_Logger.log("Error loading map: " + e.getMessage());
			return false;
		}
		
		resetTanks();
		
		m_Logger.log(mapFile + " loaded.");
		return true;
	}
	
	private void generateWorldFromXML(JavaElementXML cells) throws Exception {
		for(int row = 0; row < m_WorldHeight; ++row) {
			//String rowString = new String();
			for (int col = 0; col < m_WorldWidth; ++col) {
				try {
					m_World[row][col] = new Cell(cells.getChild(row).getChild(col).getAttributeThrows(kParamType));
					//rowString += m_World[row][col];
				} catch (Exception e) {
					throw new Exception("Error (generateWorldFromXML) on row: " + row + ", column: " + col);
				}
			}
			//m_Logger.log(rowString);
		}
	}
	
	private void resetTanks() {
		if (m_Tanks == null) {
			return;
		}
		for (int i = 0; i < m_Tanks.length; ++i) {
			Point location = findStartingLocation();
			m_Tanks[i].setLocation(location);
			// Put tank on map
			getCell(location).setTank(m_Tanks[i]);
			m_Tanks[i].setScore(0);
			m_Tanks[i].initSoar();
		}
		updateTankInput();
	}

	private Point findStartingLocation() {
		// set random starting location
		Random random = new Random();
		Point location = new Point(random.nextInt(m_WorldWidth), random.nextInt(m_WorldHeight));
		while (getCell(location).isWall() || getCell(location).isTank()) {
			location.x = random.nextInt(m_WorldWidth);
			location.y = random.nextInt(m_WorldHeight);				
		}
		
		return location;
	}
	
	private void updateTankInput() {
		for (int i = 0; i < m_Tanks.length; ++i) {
			m_Tanks[i].updateInput(this);
		}
	}
	
	public Cell getCell(Point location) {
		return m_World[location.y][location.x];
	}
	
	public Cell getCell(int x, int y) {
		return m_World[y][x];
	}
	
	void createTank(Agent agent, String productions, String color) {
		createTank(agent, productions, color, null);
	}

	void createTank(Agent agent, String productions, String color, Point location) {
		if (location == null) {
			location = findStartingLocation();
		}
		
		Tank tank = new Tank(agent, productions, color, location);
		// Put eater on map, ignore food
		getCell(location).setTank(tank);

		if (m_Tanks == null) {
			m_Tanks = new Tank[1];
			m_Tanks[0] = tank;
		} else {
			Tank[] original = m_Tanks;
			m_Tanks = new Tank[original.length + 1];
			for (int i = 0; i < original.length; ++i) {
				m_Tanks[i] = original[i];
			}
			m_Tanks[original.length] = tank;
		}

		updateTankInput();
	}
	
	void destroyTank(Tank tank) {
		if (m_Tanks == null) {
			return;
		}
		for (int i = 0; i < m_Tanks.length; ++i) {
			if (tank == m_Tanks[i]) {
				if (m_Tanks.length == 1) {
					m_Tanks = null;
				} else {
					Tank[] original = m_Tanks;
					m_Tanks = new Tank[original.length - 1];
					for (int j = 0; j < m_Tanks.length; ++j) {
						if (j < i) {
							m_Tanks[j] = original[j];
						} else {
							m_Tanks[j] = original[j+1];
						}
					}
				}
				getCell(tank.getLocation()).removeTank();
				if (m_Tanks == null) {
					break;
				}
			}
		}
	}
	
	public void update() {
		// reset modified flags, skipping edges
		for (int y = 1; y < m_World.length - 1; ++y) {
			for (int x = 1; x < m_World[y].length - 1; ++x) {
				m_World[y][x].clearModified();
				if (m_World[y][x].checkCollision()) {
					m_World[y][x].setCollision(false);
				}
			}
		}			
		
		// TODO: check goal condition
//		if (getFoodCount() <= 0) {
//			if (!m_PrintedStats) {
//				m_Simulation.stopSimulation();
//				m_PrintedStats = true;
//				m_Logger.log("All of the food is gone.");
//				for (int i = 0; i < m_Tanks.length; ++i) {
//					m_Logger.log(m_Tanks[i].getName() + ": " + m_Tanks[i].getScore());
//				}
//			}
//			return;
//		}

		if (m_Tanks == null) {
			m_Logger.log("Update called with no tanks.");
			return;
		}
		
		moveTanks();
		updateMap();
		handleCollisions();	
		updateTankInput();
	}
	
	////////////////// HERE
	private void moveTanks() {
		for (int i = 0; i < m_Tanks.length; ++i) {
			Tank.MoveInfo move = m_Tanks[i].getMove();
			if (move == null) {
				continue;
			}

			Point oldLocation = m_Tanks[i].getLocation();
			Point newLocation;
			int distance = move.jump ? 2 : 1;
			if (move.direction.equalsIgnoreCase(Eater.kNorth)) {
				newLocation = new Point(oldLocation.x, oldLocation.y - distance);
			} else if (move.direction.equalsIgnoreCase(Eater.kEast)) {
				newLocation = new Point(oldLocation.x + distance, oldLocation.y);
				
			} else if (move.direction.equalsIgnoreCase(Eater.kSouth)) {
				newLocation = new Point(oldLocation.x, oldLocation.y + distance);
				
			} else if (move.direction.equalsIgnoreCase(Eater.kWest)) {
				newLocation = new Point(oldLocation.x - distance, oldLocation.y);
				
			} else {
				m_Logger.log("Invalid move direction: " + move.direction);
				return;
			}
			
			if (isInBounds(newLocation) && !getCell(newLocation).isWall()) {
				if (!getCell(oldLocation).removeEater()) {
					m_Logger.log("Warning: moving eater " + m_Tanks[i].getName() + " not at old location " + oldLocation);
				}
				m_Tanks[i].setLocation(newLocation);
				if (move.jump) {
					m_Tanks[i].adjustScore(kJumpPenalty);
				}
			} else {
				m_Tanks[i].adjustScore(kWallPenalty);
			}
		}
	}
	
	public boolean noAgents() {
		return (m_Tanks == null);
	}
	
	public void shutdown() {
		while (m_Tanks != null) {
			m_Simulation.destroyTank(m_Tanks[0]);
		}
	}
}
