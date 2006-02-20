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

	public class FoodInfo {
		public static final String kRound = "round";
		public static final String kSquare = "square";
			
		protected String m_Name;
		protected int m_Value;
		protected String m_Shape;
		protected String m_ColorString;
		protected Color m_Color;
		
		protected Logger m_Logger = Logger.logger;

		public FoodInfo(String name, int value, String shape, String color) {
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

	Logger m_Logger = Logger.logger;
	int m_WorldWidth;
	int m_WorldHeight;
	int[][] m_World;
	FoodInfo[] m_FoodInfo;
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
			m_FoodInfo = new FoodInfo[food.getNumberChildren()];
			for (int i = 0; i < m_FoodInfo.length; ++i) {
				JavaElementXML foodType = food.getChild(i);
				m_FoodInfo[i] = new FoodInfo(
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
			m_World = new int[m_WorldHeight][m_WorldWidth];
			
			// Keep track of food
			m_FoodCount = 0;
			for(int row = 0; row < m_WorldHeight; ++row) {
				String rowString = new String();
				for (int col = 0; col < m_WorldWidth; ++col) {
					m_World[row][col] = getTypeIDByName(cells.getChild(row).getChild(col).getAttributeThrows(kParamType));
					if (m_World[row][col] > kEaterCell) {
						++m_FoodCount;
					}
					rowString += new Integer(m_World[row][col]).toString();
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
	
	public int getTypeIDByName(String name) throws Exception {
		if (name.equalsIgnoreCase(kTypeWall)) {
			return kWallCell;
		} else if (name.equalsIgnoreCase(kTypeEmpty)) {
			return kEmptyCell;			
		} else {	
			for (int i = 0; i < m_FoodInfo.length; ++i) {
				if (m_FoodInfo[i].getName().equalsIgnoreCase(name)) {
					return i + kReservedIDs;
				}
			}
		}
		throw new Exception("Invalid type name: " + name);
	}
	
	public int getWidth() {
		return this.m_WorldWidth;
	}
	
	public int getHeight() {
		return this.m_WorldHeight;
	}
	
	public boolean isWall(Point location) {
		return isWall(location.x, location.y);
	}
	
	public boolean isEmpty(Point location) {
		return isEmpty(location.x, location.y);
	}
	
	public boolean isEater(Point location) {
		return isEater(location.x, location.y);
	}
	
	public FoodInfo getFoodInfo(Point location) {
		return getFoodInfo(location.x, location.y);
	}

	public boolean isWall(int x, int y) {
		return (m_World[y][x] == kWallCell);
	}
	
	public boolean isEmpty(int x, int y) {
		return (m_World[y][x] == kEmptyCell);
	}
	
	public boolean isEater(int x, int y) {
		return (m_World[y][x] == kEaterCell);
	}
	
	public boolean isEnterable(Point location) {
		return isEnterable(location.x, location.y);
	}
	
	public boolean isEnterable(int x, int y) {
		if (x < 0 || y < 0 || x >= m_WorldWidth || y >= m_WorldHeight) {
			return false;
		}
		if (isWall(x, y)) {
			return false;
		}
		return true;
	}
	
	public FoodInfo getFoodInfo(int x, int y) {
		int value = m_World[y][x];
		value -= kReservedIDs;
		if (value < 0) return null;
		return m_FoodInfo[value];
	}
	
	public int getFoodCount() {
		return m_FoodCount;
	}
	
	public String getContentNameByLocation(int x, int y) {
		if (x < 0 || y < 0 || x >= m_WorldWidth || y >= m_WorldHeight) {
			return kTypeEmpty;
		}
		
		int value = m_World[y][x];
		switch (value) {
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
		return getFoodInfo(x, y).getName();
	}
	
	public void createEater(Agent agent) {
		createEater(agent, null);
	}

	public void createEater(Agent agent, Point location) {
		if (location == null) {
			// set random starting location
			Random random = new Random();
			location = new Point(random.nextInt(m_WorldWidth), random.nextInt(m_WorldHeight));
			while (isWall(location) || isEater(location)) {
				location.x = random.nextInt(m_WorldWidth);
				location.y = random.nextInt(m_WorldHeight);				
			}
		}
		
		if (getFoodInfo(location) != null) {
			--m_FoodCount;
		}
		
		Eater eater = new Eater(agent, location);
		m_Eaters.put(agent.GetAgentName(), eater);

		setEaterCells();

		eater.updateInput(this);
	}
	
	public void destroyAllEaters() {
		Object[] objects = (Object[])m_Eaters.values().toArray();
		for (int i = 0; i < objects.length; ++i) {
			m_Simulation.destroyEater((Eater)objects[i]);
		}
		m_Eaters = null;		
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
		
		if (isEnterable(newLocation)) {
			m_World[oldLocation.y][oldLocation.x] = kEmptyCell;
			FoodInfo fi = getFoodInfo(newLocation);
			if (fi != null) {
				--m_FoodCount;
				move.eater.adjustScore(fi.getValue());
			}
			move.eater.setLocation(newLocation);
		} else {
			move.eater.adjustScore(kWallPenalty);
		}
	}
	
	void setEaterCells() {
		Eater[] eaters = (Eater[])m_Eaters.values().toArray(new Eater[0]);
		for (int i = 0; i < eaters.length; ++i) {
			Point location = eaters[i].getLocation();
			m_World[location.y][location.x] = kEaterCell;
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
		
		for (int i = 0; i < eaters.length; ++i) {
			eaters[i].updateInput(this);
		}
	}
	
	void handleCollisions() {
		// TODO
	}
}

