package eaters;

import java.util.*;
import org.eclipse.swt.graphics.*;

import eaters.visuals.EatersWindowManager;
import sml.*;
import utilities.*;

public class World {
	public static final String kTagEatersWorld = "eaters-world";

	public static final String kTagFood = "food";
	public static final String kTagFoodType = "food-type";
	public static final String kParamName = "name";
	public static final String kParamValue = "value";
	public static final String kParamShape = "shape";
	public static final String kParamColor = "color";

	public static final String kTagCells = "cells";
	public static final String kParamWorldWidth = "world-width";
	public static final String kParamWorldHeight = "world-height";
	public static final String kTagRow = "row";
	public static final String kTagCell = "cell";
	public static final String kParamType = "type";
	
	public static final String kTypeWall = "wall";
	public static final String kTypeEmpty = "empty";
	public static final String kTypeEater = "eater";
	
	public static final int kWallCell = 0;
	public static final int kEmptyCell = 1;
	public static final int kEaterCell = 2;
	public static final int kReservedIDs = 3;
	
	public static final int kWallPenalty = -5;
	public static final int kJumpPenalty = -5;

	public class Food {
		public static final String kRound = "round";
		public static final String kSquare = "square";
			
		String m_Name;
		int m_Value;
		String m_Shape;
		String m_ColorString;
		Color m_Color;
		
		protected Logger m_Logger = Logger.logger;

		public Food(String name, int value, String shape, String color) {
			m_Name = name;
			m_Value = value;
			m_Shape = shape;
			m_ColorString = color;
		}
		
		public String getName() {
			return m_Name;
		}
		
		public int getValue() {
			return m_Value;
		}
		
		public String getShape() {
			return m_Shape;
		}
		
		public Color getColor() {
			if (m_Color == null) {
				m_Color = EatersWindowManager.getColor(m_ColorString);
			}
			return m_Color;
		}
	}
	
	public class Cell {
		int m_Type;
		Eater m_Eater;

		public Cell(String name) throws Exception {
			if (name.equalsIgnoreCase(kTypeWall)) {
				m_Type = kWallCell;
				return;
			} else if (name.equalsIgnoreCase(kTypeEmpty)) {
				m_Type = kEmptyCell;			
				return;
			} else {	
				for (int i = 0; i < m_Food.length; ++i) {
					if (m_Food[i].getName().equalsIgnoreCase(name)) {
						m_Type = i + kReservedIDs;
						++m_FoodCount;
						return;
					}
				}
			}
			throw new Exception("Invalid type name: " + name);
		}
		
		public boolean isWall() {
			return m_Type == kWallCell;
		}
		
		public boolean isEmpty() {
			return m_Type == kEmptyCell;
		}
		
		public boolean isEater() {
			return m_Type == kEaterCell;
		}
		
		public boolean isFood() {
			return m_Type >= kReservedIDs;
		}
		
		public void removeEater() {
			if (m_Type != kEaterCell) {
				m_Logger.log("Warning: tried to remove eater from cell it wasn't at.");
				return;
			}
			m_Type = kEmptyCell;
			m_Eater = null;
		}
		
		public Food setEater(Eater eater) {
			Food f = null;
			if (isFood()) {
				f = removeFood();
			}
			m_Type = kEaterCell;
			m_Eater = eater;
			return f;
		}
		
		public Eater getEater() {
			return m_Eater;
		}
		
		public String getName() {
			switch (m_Type) {
			case kWallCell:
				return kTypeWall;
			case kEmptyCell:
				return kTypeEmpty;
			case kEaterCell:
				return kTypeEater;
			default:
				break;
			}
			
			// TODO: risking null exception here
			return getFood().getName();
		}
		
		public Food getFood() {
			if ((m_Type - kReservedIDs) < 0) return null;
			return m_Food[(m_Type - kReservedIDs)];
		}
		
		public Food removeFood() {
			if (isFood()) {
				Food f = getFood();
				m_Type = kEmptyCell;
				--m_FoodCount;
				return f;
			}
			return null;
		}
		
		public String toString() {
			return new Integer(m_Type).toString();
		}	
	}

	Logger m_Logger = Logger.logger;
	int m_WorldWidth;
	int m_WorldHeight;
	Cell[][] m_World;
	Food[] m_Food;
	EatersSimulation m_Simulation;
	int m_FoodCount;
	Eater[] m_Eaters;
	
	public World(EatersSimulation simulation) {
		m_Simulation = simulation;
	}
	
	public boolean load(String mapFile) {
		try {
			// Open file
			JavaElementXML root = JavaElementXML.ReadFromFile(mapFile);
			
			if (!root.getTagName().equalsIgnoreCase(kTagEatersWorld)) {
				throw new Exception("Not an eaters map!");
			}
			// TODO: Check version
			
			// Read food types from file
			JavaElementXML food = root.findChildByNameThrows(kTagFood);
			m_Food = new Food[food.getNumberChildren()];
			for (int i = 0; i < m_Food.length; ++i) {
				JavaElementXML foodType = food.getChild(i);
				m_Food[i] = new Food(
						foodType.getAttributeThrows(kParamName), 
						foodType.getAttributeIntThrows(kParamValue),
						foodType.getAttributeThrows(kParamShape),
						foodType.getAttributeThrows(kParamColor));
			}

			// Create map
			JavaElementXML cells = root.findChildByNameThrows(kTagCells);
			
			// Get dimentions
			m_WorldWidth = cells.getAttributeIntThrows(kParamWorldWidth);
			m_WorldHeight = cells.getAttributeIntThrows(kParamWorldHeight);
						
			// Create map array
			m_World = new Cell[m_WorldHeight][m_WorldWidth];
			
			// Reset food
			m_FoodCount = 0;
			
			for(int row = 0; row < m_WorldHeight; ++row) {
				String rowString = new String();
				for (int col = 0; col < m_WorldWidth; ++col) {
					m_World[row][col] = new Cell(cells.getChild(row).getChild(col).getAttributeThrows(kParamType));
					rowString += m_World[row][col];
				}
				m_Logger.log(rowString);
			}
			
		} catch (Exception e) {
			m_Logger.log("Error loading map: " + e.getMessage());
			return false;
		}
		
		resetEaters();
		
		m_Logger.log(mapFile + " loaded.");
		return true;
	}
	
	public int getWidth() {
		return this.m_WorldWidth;
	}
	
	public int getHeight() {
		return this.m_WorldHeight;
	}
	
	public Food getFood(int x, int y) {
		return getCell(x,y).getFood();
	}
	
	public int getFoodCount() {
		return m_FoodCount;
	}
	
	public String getContentNameByLocation(int x, int y) {
		if (this.isInBounds(x,y)) {
			return getCell(x,y).getName();
		}
		return kTypeEmpty;
	}
	
	void resetEaters() {
		if (m_Eaters == null) {
			return;
		}
		for (int i = 0; i < m_Eaters.length; ++i) {
			Point location = findStartingLocation();
			m_Eaters[i].setLocation(location);
			// Put eater on map, ignore food
			getCell(location).setEater(m_Eaters[i]);
			m_Eaters[i].setScore(0);
			m_Eaters[i].initSoar();
		}
	}

	public void createEater(Agent agent, String productions, String color) {
		createEater(agent, productions, color, null);
	}

	public void createEater(Agent agent, String productions, String color, Point location) {
		if (location == null) {
			location = findStartingLocation();
		}
		
		Eater eater = new Eater(agent, productions, color, location);
		// Put eater on map, ignore food
		getCell(location).setEater(eater);

		if (m_Eaters == null) {
			m_Eaters = new Eater[1];
			m_Eaters[0] = eater;
		} else {
			Eater[] original = m_Eaters;
			m_Eaters = new Eater[original.length + 1];
			for (int i = 0; i < original.length; ++i) {
				m_Eaters[i] = original[i];
			}
			m_Eaters[original.length] = eater;
		}

		updateEaterInput();
	}
	
	public void destroyAllEaters() {
		if (m_Eaters == null) {
			return;
		}
		for (int i = 0; i < m_Eaters.length; ++i) {
			m_Simulation.destroyEater(m_Eaters[i]);
		}
		m_Eaters = null;		
	}
	
	public void destroyEater(Eater eater) {
		if (m_Eaters == null) {
			return;
		}
		for (int i = 0; i < m_Eaters.length; ++i) {
			if (eater == m_Eaters[i]) {
				if (m_Eaters.length == 1) {
					m_Eaters = null;
				} else {
					Eater[] original = m_Eaters;
					m_Eaters = new Eater[original.length - 1];
					for (int j = 0; j < m_Eaters.length; ++j) {
						if (j < i) {
							m_Eaters[j] = original[j];
						} else {
							m_Eaters[j] = original[j+1];
						}
					}
				}
				getCell(eater.getLocation()).removeEater();
				m_Simulation.destroyEater(eater);
				if (m_Eaters == null) {
					break;
				}
			}
		}
	}

	public Point findStartingLocation() {
		// set random starting location
		Random random = new Random();
		Point location = new Point(random.nextInt(m_WorldWidth), random.nextInt(m_WorldHeight));
		while (getCell(location).isWall() || getCell(location).isEater()) {
			location.x = random.nextInt(m_WorldWidth);
			location.y = random.nextInt(m_WorldHeight);				
		}
		
		return location;
	}
	
	void moveEaters() {
		for (int i = 0; i < m_Eaters.length; ++i) {
			Eater.MoveInfo move = m_Eaters[i].getMove();
			if (move == null) {
				continue;
			}

			Point oldLocation = m_Eaters[i].getLocation();
			Point newLocation;
			if (move.direction.equalsIgnoreCase(Eater.kNorth)) {
				newLocation = new Point(oldLocation.x, oldLocation.y - 1);
			} else if (move.direction.equalsIgnoreCase(Eater.kEast)) {
				newLocation = new Point(oldLocation.x + 1, oldLocation.y);
				
			} else if (move.direction.equalsIgnoreCase(Eater.kSouth)) {
				newLocation = new Point(oldLocation.x, oldLocation.y + 1);
				
			} else if (move.direction.equalsIgnoreCase(Eater.kWest)) {
				newLocation = new Point(oldLocation.x - 1, oldLocation.y);
				
			} else {
				m_Logger.log("Invalid move direction: " + move.direction);
				return;
			}
			
			if (isInBounds(newLocation) && !getCell(newLocation).isWall()) {
				getCell(oldLocation).removeEater();
				m_Eaters[i].setLocation(newLocation);
				if (move.jump) {
					m_Eaters[i].adjustScore(kJumpPenalty);
				}
			} else {
				m_Eaters[i].adjustScore(kWallPenalty);
			}
		}
	}
	
	public Cell getCell(Point location) {
		return m_World[location.y][location.x];
	}
	
	public Cell getCell(int x, int y) {
		return m_World[y][x];
	}
	
	void updateMapAndEatFood() {
		for (int i = 0; i < m_Eaters.length; ++i) {
			Food f = getCell(m_Eaters[i].getLocation()).setEater(m_Eaters[i]);
			if (f != null) {
				m_Eaters[i].adjustScore(f.getValue());
			}
		}
	}
	
	public boolean isInBounds(Point location) {
		return isInBounds(location.x, location.y);
	}

	public boolean isInBounds(int x, int y) {
		return (x >= 0) && (y >= 0) && (x < m_WorldWidth) && (y < m_WorldWidth);
	}
	
	void updateEaterInput() {
		for (int i = 0; i < m_Eaters.length; ++i) {
			m_Eaters[i].updateInput(this);
		}
	}
	
	void update() {
		if (getFoodCount() <= 0) {
			m_Logger.log("All of the food is gone.");
			m_Simulation.stopSimulation();
			return;
		}

		if (m_Eaters == null) {
			m_Logger.log("Update called with no eaters.");
			return;
		}
		
		moveEaters();
		updateMapAndEatFood();
		handleCollisions();	
		updateEaterInput();
	}
	
	void handleCollisions() {
		for (int i = 0; i < m_Eaters.length; ++i) {
			for (int j = i+1; j < m_Eaters.length; ++j) {
				if (m_Eaters[i].getLocation().equals(m_Eaters[j].getLocation())) {
					
					m_Logger.log("Handling collison at " + m_Eaters[i].getLocation());
					
					// Sum cash
					int cash = m_Eaters[i].getScore();
					cash += m_Eaters[j].getScore();
					cash /= 2;
					m_Eaters[i].setScore(cash);
					m_Eaters[j].setScore(cash);
					
					// Remove from former location
					getCell(m_Eaters[i].getLocation()).removeEater();
					
					// Find new location for eater 1
					m_Eaters[i].setLocation(findStartingLocation());
					// Update map, consume as necesary
					Food f = getCell(m_Eaters[i].getLocation()).setEater(m_Eaters[i]);
					if (f != null) {
						m_Eaters[i].adjustScore(f.getValue());
					}

					// Find new location for eater 2
					m_Eaters[j].setLocation(findStartingLocation());
					// Update map, consume as necesary
					f = getCell(m_Eaters[j].getLocation()).setEater(m_Eaters[j]);
					if (f != null) {
						m_Eaters[j].adjustScore(f.getValue());
					}					
				}
			}
		}
	}
	
	public Eater[] getEaters() {
		return m_Eaters;
	}
}

