package eaters;

import java.io.IOException;
import java.util.HashMap;

public class World {
	public static final String kTagEatersWorld = "eaters-world";

	public static final String kTagFood = "food";
	public static final String kTagFoodType = "food-type";
	public static final String kParamFoodName = "food-name";
	public static final String kParamFoodValue = "food-value";

	public static final String kTagCells = "cells";
	public static final String kParamWorldWidth = "world-width";
	public static final String kParamWorldHeight = "world-height";
	public static final String kTagRow = "row";
	public static final String kTagCell = "cell";
	public static final String kParamID = "id";

	protected Logger m_Logger;
	protected int m_MapWidth = 17;
	protected int m_MapHeight = 17;
	protected int[][] m_World;
	protected FoodInfo[] m_FoodInfo;
	
	public World(String file, Logger logger) {
		m_Logger = logger;
		
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
				m_FoodInfo[i] = new FoodInfo(foodType.getAttributeThrows(kParamFoodName), 
						foodType.getAttributeIntThrows(kParamFoodValue));
			}

			// Create map
			JavaElementXML cells = root.findChildByNameThrows(kTagCells);
			
			// Get dimentions
			m_MapWidth = cells.getAttributeIntThrows(kParamWorldWidth);
			m_MapHeight = cells.getAttributeIntThrows(kParamWorldHeight);
						
			// Create map array
			m_World = new int[m_MapHeight][m_MapWidth];
			
			for(int row = 0; row < m_MapHeight; ++row) {
				String rowString = new String();
				for (int col = 0; col < m_MapWidth; ++col) {
					// TODO: create map					
					m_World[row][col] = cells.getChild(row).getChild(col).getAttributeIntThrows(kParamID);
					rowString += new Integer(m_World[row][col]).toString();
				}
				m_Logger.log(rowString);
			}
			
		} catch (Exception e) {
			m_Logger.log("Error loading map: " + e.getMessage());
			System.exit(1);
		}
		
		m_Logger.log("Map loaded.");
		
		// TODO: events
		//fireNewMapNotification(null);
	}
}

