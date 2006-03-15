package tanksoar;

import java.util.*;

import org.eclipse.swt.graphics.*;

import simulation.*;
import sml.*;
import utilities.*;

public class TankSoarWorld extends World implements WorldManager {
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
	
	private static final int kWallPenalty = -100;
	private static final int kWinningPoints = -100;

	public class TankSoarCell extends Cell {
		private Tank m_Tank;
		
		public TankSoarCell(String name) throws Exception {
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
		
		public Tank getTank() {
			return m_Tank;
		}
	}
	
	private TankSoarSimulation m_Simulation;
	private TankSoarCell[][] m_World;
	private boolean m_PrintedStats;
	private Tank[] m_Tanks;
	private ArrayList m_Collisions;

	public TankSoarWorld(TankSoarSimulation simulation) {
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
			m_World = new TankSoarCell[m_WorldHeight][m_WorldWidth];
			
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
					m_World[row][col] = new TankSoarCell(cells.getChild(row).getChild(col).getAttributeThrows(kParamType));
					//rowString += m_World[row][col];
				} catch (Exception e) {
					throw new Exception("Error (generateWorldFromXML) on row: " + row + ", column: " + col);
				}
			}
			//m_Logger.log(rowString);
		}
	}
	
	public Tank[] getTanks() {
		return m_Tanks;
	}
	
	public void destroyEntity(WorldEntity entity) {
		for (int i = 0; i < m_Tanks.length; ++i) {
			if (m_Tanks[i].getName() == entity.getName()) {
				destroyTank(m_Tanks[i]);
				return;
			}
		}
		m_Logger.log("Couldn't find entity name match for " + entity.getName() + ", ignoring.");
	}
	
	public WorldEntity[] getEntities() {
		return getTanks();
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
			m_Tanks[i].setPoints(0);
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
	
	public TankSoarCell getCell(Point location) {
		return m_World[location.y][location.x];
	}
	
	public TankSoarCell getCell(int x, int y) {
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
	
	public boolean getVictoryCondition() {
		return m_PrintedStats;
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
		
		for (int i = 0; i < m_Tanks.length; ++i) {
			if (m_Tanks[i].getPoints() >= kWinningPoints) {
				// Goal acheived
				if (!m_PrintedStats) {
					m_Simulation.stopSimulation();
					m_PrintedStats = true;
					m_Logger.log(m_Tanks[i].getName() + " is the winning tank.");
					for (int j = 0; j < m_Tanks.length; ++j) {
						m_Logger.log(m_Tanks[j].getName() + ": " + m_Tanks[j].getPoints());
					}
				}
				return;
			}
		}

		if (m_Tanks == null) {
			m_Logger.log("Update called with no tanks.");
			return;
		}
		
		moveTanks();
		updateMap();
		handleCollisions();	
		updateTankInput();
	}
	
	private void moveTanks() {
		for (int i = 0; i < m_Tanks.length; ++i) {
			Tank.MoveInfo move = m_Tanks[i].getMove();
			if (move == null) {
				continue;
			}
			
			if (move.move) {
				Point oldLocation = m_Tanks[i].getLocation();
				Point newLocation;
				if (move.moveDirection.equalsIgnoreCase(Tank.kNorth)) {
					newLocation = new Point(oldLocation.x, oldLocation.y - 1);
				} else if (move.moveDirection.equalsIgnoreCase(Tank.kEast)) {
					newLocation = new Point(oldLocation.x + 1, oldLocation.y);
					
				} else if (move.moveDirection.equalsIgnoreCase(Tank.kSouth)) {
					newLocation = new Point(oldLocation.x, oldLocation.y + 1);
					
				} else if (move.moveDirection.equalsIgnoreCase(Tank.kWest)) {
					newLocation = new Point(oldLocation.x - 1, oldLocation.y);
					
				} else {
					m_Logger.log("Invalid move direction: " + move.moveDirection);
					return;
				}
				
				if (isInBounds(newLocation) && !getCell(newLocation).isWall()) {
					if (!getCell(oldLocation).removeTank()) {
						m_Logger.log("Warning: moving tank " + m_Tanks[i].getName() + " not at old location " + oldLocation);
					}
					m_Tanks[i].setLocation(newLocation);
				} else {
					m_Tanks[i].adjustPoints(kWallPenalty);
				}
			}
		}
	}
	
	private void updateMap() {
		for (int i = 0; i < m_Tanks.length; ++i) {
			// TODO: Grab missiles
			getCell(m_Tanks[i].getLocation()).setTank(m_Tanks[i]);
		}
	}
	
	private void handleCollisions() {
		// generate collision groups
		ArrayList currentCollision = null;
		
		for (int i = 0; i < m_Tanks.length; ++i) {
			for (int j = i+1; j < m_Tanks.length; ++j) {
				// only check eaters who aren't already colliding
				if (m_Tanks[i].isColliding()) {
					continue;
				}
				
				if (m_Tanks[i].getLocation().equals(m_Tanks[j].getLocation())) {
					
					// Create data structures
					if (m_Collisions == null) {
						m_Collisions = new ArrayList();
					}
					if (currentCollision == null) {
						currentCollision = new ArrayList();
						
						// Add first agent to current collision
						currentCollision.add(m_Tanks[i]);
						
						// Flipping collision flag unnecessary as first agent will not be traversed again

						// Flip collision flag for cell
						getCell(m_Tanks[i].getLocation()).setCollision(true);

						m_Logger.log("Starting collision group at " + m_Tanks[i].getLocation());
					}
					
					// Add second agent to current collision
					currentCollision.add(m_Tanks[j]);

					// Flip collision flag for second agent
					m_Tanks[j].setColliding(true);
				}
			}
			// add current collision to collisions
			if (currentCollision != null) {
				m_Collisions.add(currentCollision);
				currentCollision = null;
			}
		}
		
		// if there are not collisions, we're done
		if (m_Collisions == null) {
			return;
		}
		
		// process collision groups
		for (int group = 0; group < m_Collisions.size(); ++group) {
			// Retrieve collision group
			currentCollision = (ArrayList)m_Collisions.get(group);
			Tank[] collidees = (Tank[])currentCollision.toArray(new Tank[0]);
			
			m_Logger.log("Processing collision group " + group + " with " + collidees.length + " collidees.");
			
			// Redistribute wealth
			int cash = 0;			
			for (int i = 0; i < collidees.length; ++i) {
				cash += collidees[i].getPoints();
			}			
			cash /= collidees.length;
			m_Logger.log("Cash to each: " + cash);
			for (int i = 0; i < collidees.length; ++i) {
				collidees[i].setPoints(cash);
			}
			
			// Remove from former location (only one of these for all eaters)
			getCell(collidees[0].getLocation()).removeTank();

			// Find new locations, update map
			for (int i = 0; i < collidees.length; ++i) {
				collidees[i].setLocation(findStartingLocation());
				// TODO: missiles
				getCell(collidees[i].getLocation()).setTank(collidees[i]);
			}
		}
		
		// clear collision groups
		m_Collisions = null;
		
		// clear colliding flags
		for (int i = 0; i < m_Tanks.length; ++i) {
			m_Tanks[i].setColliding(false);
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
