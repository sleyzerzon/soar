package eaters;

import java.util.HashMap;
import java.util.Random;

import org.eclipse.swt.graphics.Point;

import sml.Agent;
import utilities.JavaElementXML;
import utilities.Logger;

public class World implements SimulationListener {
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
	
	public static final int kWallCell = 0;
	public static final int kEmptyCell = 1;
	public static final int kEaterCell = 2;
	public static final int kReservedIDs = 3;

	protected Logger m_Logger = Logger.logger;
	protected int m_WorldWidth;
	protected int m_WorldHeight;
	protected int[][] m_World;
	protected FoodInfo[] m_FoodInfo;
	protected EatersSimulation m_Simulation;
	protected int m_FoodCount;
	protected HashMap m_Eaters = new HashMap();
	
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
			m_Simulation.shutdown(1);
		}
		
		m_Logger.log("Map loaded.");
		
		// TODO: events
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
	
	public FoodInfo getFoodInfo(int x, int y) {
		int value = m_World[y][x];
		value -= kReservedIDs;
		if (value < 0) return null;
		return m_FoodInfo[value];
	}
	
	public int getFoodCount() {
		return m_FoodCount;
	}
	
	public void simulationEventHandler(int type, Object object) {
		if (type == SimulationListener.kNewEaterEvent) {
			EatersSimulation.EaterInfo info = (EatersSimulation.EaterInfo)object;
			if (info.location == null) {
				// set random starting location
				Random random = new Random();
				info.location = new Point(random.nextInt(m_WorldWidth), random.nextInt(m_WorldHeight));
				while (isWall(info.location) || isEater(info.location)) {
					info.location.x = random.nextInt(m_WorldWidth);
					info.location.y = random.nextInt(m_WorldHeight);				
				}
			}
			
			m_World[info.location.y][info.location.x] = kEaterCell;
			--m_FoodCount;
			
			Eater eater = new Eater(info.agent, this, info.location);
			m_Eaters.put(info.name, eater);
		} 
	}
}

