package eaters;

import java.util.HashMap;
import java.util.Random;

import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.Point;


import sml.Agent;
import utilities.JavaElementXML;
import utilities.Logger;

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
		
		public void setEmpty() {
			m_Type = kEmptyCell;
		}
		
		public void setEater() {
			m_Type = kEaterCell;
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
		
		public boolean eat() {
			if (isFood()) {
				--m_FoodCount;
				m_Type = kEmptyCell;
				return true;
			}
			return false;
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
	HashMap m_Eaters = new HashMap();
	
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
	
	public void resetEaters() {
		Eater[] eaters = (Eater[])m_Eaters.values().toArray(new Eater[0]);
		for (int i = 0; i < eaters.length; ++i) {
			Point location = findStartingLocation();
			getCell(location).eat();
			eaters[i].setLocation(location);
			eaters[i].clearScore();
			eaters[i].initSoar();
		}
		setEaterCells();
	}

	public void createEater(Agent agent) {
		createEater(agent, null);
	}

	public void createEater(Agent agent, Point location) {
		if (location == null) {
			location = findStartingLocation();
		}
		
		getCell(location).eat();
		
		Eater eater = new Eater(agent, location);
		m_Eaters.put(agent.GetAgentName(), eater);

		setEaterCells();
		updateEaterInput();
	}
	
	public void destroyAllEaters() {
		Eater[] eaters = (Eater[])m_Eaters.values().toArray(new Eater[0]);
		for (int i = 0; i < eaters.length; ++i) {
			m_Simulation.destroyEater(eaters[i]);
		}
		m_Eaters = null;		
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
	
	void moveEater(Eater.MoveInfo move) {
		Point oldLocation = move.eater.getLocation();
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
			if (getCell(newLocation).isFood()) {
				Food f = getCell(newLocation).getFood();
				getCell(newLocation).eat();
				move.eater.adjustScore(f.getValue());
			}
			getCell(oldLocation).setEmpty();
			move.eater.setLocation(newLocation);
		} else {
			move.eater.adjustScore(kWallPenalty);
		}
	}
	
	public Cell getCell(Point location) {
		return m_World[location.y][location.x];
	}
	
	public Cell getCell(int x, int y) {
		return m_World[y][x];
	}
	
	void setEaterCells() {
		Eater[] eaters = (Eater[])m_Eaters.values().toArray(new Eater[0]);
		for (int i = 0; i < eaters.length; ++i) {
			Point location = eaters[i].getLocation();
			getCell(location).setEater();
		}
	}
	
	public boolean isInBounds(Point location) {
		return isInBounds(location.x, location.y);
	}

	public boolean isInBounds(int x, int y) {
		return (x >= 0) && (y >= 0) && (x < m_WorldWidth) && (y < m_WorldWidth);
	}
	
	void updateEaterInput() {
		Eater[] eaters = (Eater[])m_Eaters.values().toArray(new Eater[0]);
		for (int i = 0; i < eaters.length; ++i) {
			eaters[i].updateInput(this);
		}
	}
	
	void update() {
		if (getFoodCount() <= 0) {
			m_Logger.log("All of the food is gone.");
			m_Simulation.stopSimulation();
			return;
		}
		
		Eater[] eaters = (Eater[])m_Eaters.values().toArray(new Eater[0]);
		for (int i = 0; i < eaters.length; ++i) {
			moveEater(eaters[i].getMove());
		}
		setEaterCells();
		handleCollisions();	
		updateEaterInput();
	}
	
	void handleCollisions() {
		// TODO
	}
}

