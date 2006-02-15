package eaters;

import org.eclipse.swt.graphics.Point;

import utilities.JavaElementXML;
import utilities.Logger;

public class World {
	public static final String kTagEatersWorld = "eaters-world";

	public static final String kTagFood = "food";
	public static final String kTagFoodType = "food-type";
	public static final String kParamFoodName = "food-name";
	public static final String kParamFoodValue = "food-value";
	public static final String kParamFoodShape = "food-shape";
	public static final String kParamFoodColor = "food-color";

	public static final String kTagCells = "cells";
	public static final String kParamWorldWidth = "world-width";
	public static final String kParamWorldHeight = "world-height";
	public static final String kTagRow = "row";
	public static final String kTagCell = "cell";
	public static final String kParamID = "id";

	protected Logger m_Logger = Logger.logger;
	protected int m_WorldWidth;
	protected int m_WorldHeight;
	protected int[][] m_World;
	protected FoodInfo[] m_FoodInfo;
	protected EatersSimulation m_Simulation;
	protected int m_FoodCount;
	
	public World(String file, EatersSimulation simulation) {
		m_Simulation = simulation;
		
		try {
			// Open file
			JavaElementXML root = JavaElementXML.ReadFromFile(file);
			
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
						foodType.getAttributeThrows(kParamFoodName), 
						foodType.getAttributeIntThrows(kParamFoodValue),
						foodType.getAttributeThrows(kParamFoodShape),
						foodType.getAttributeThrows(kParamFoodColor));
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
					// TODO: create map					
					m_World[row][col] = cells.getChild(row).getChild(col).getAttributeIntThrows(kParamID);
					if (m_World[row][col] > 1) {
						++m_FoodCount;
					}
					rowString += new Integer(m_World[row][col]).toString();
				}
				m_Logger.log(rowString);
			}
			
		} catch (Exception e) {
			m_Logger.log("Error loading map: " + e.getMessage());
			m_Simulation.shutdown(1);
		}
		
		m_Logger.log("Map loaded.");
		
		// TODO: events
		//fireNewMapNotification(null);
	}
	
	public int getWidth() {
		return this.m_WorldWidth;
	}
	
	public int getHeight() {
		return this.m_WorldHeight;
	}
	
	public boolean isWall(Point location) {
		return (m_World[location.y][location.x] == 0);
	}
	
	public boolean isEmpty(Point location) {
		return (m_World[location.y][location.x] == 1);
	}
	
	public boolean isWall(int x, int y) {
		return (m_World[y][x] == 0);
	}
	
	public boolean isEmpty(int x, int y) {
		return (m_World[y][x] == 1);
	}
	
	public FoodInfo getFoodInfo(Point location) {
		int value = m_World[location.y][location.x];
		value -= 2;
		if (value < 0) return null;
		return m_FoodInfo[value];
	}

	public FoodInfo getFoodInfo(int x, int y) {
		int value = m_World[y][x];
		value -= 2;
		if (value < 0) return null;
		return m_FoodInfo[value];
	}
	
	public int getFoodCount() {
		return m_FoodCount;
	}
}

