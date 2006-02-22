package eaters;

import java.util.*;
import org.eclipse.swt.graphics.*;

import eaters.visuals.EatersWindowManager;
import sml.*;
import utilities.*;

public class World {
	static final String kTagEatersWorld = "eaters-world";

	static final String kTagFood = "food";
	static final String kTagFoodType = "food-type";
	static final String kParamName = "name";
	static final String kParamValue = "value";
	static final String kParamShape = "shape";
	static final String kParamColor = "color";

	static final String kTagCells = "cells";
	static final String kParamWorldWidth = "world-width";
	static final String kParamWorldHeight = "world-height";
	static final String kParamRandom = "random";
	static final String kTagRow = "row";
	static final String kTagCell = "cell";
	static final String kParamType = "type";
	
	static final String kTypeWall = "wall";
	static final String kTypeEmpty = "empty";
	static final String kTypeEater = "eater";
	
	static final int kWallCell = 0;
	static final int kEmptyCell = 1;
	static final int kEaterCell = 2;
	static final int kReservedIDs = 3;
	
	static final int kWallPenalty = -5;
	static final int kJumpPenalty = -5;

	static final double kLowProbability = .15;
	static final double kHigherProbability = .65;
	
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
		
		public Cell(int foodIndex) {
			m_Type = foodIndex + kReservedIDs;
			++m_FoodCount;
		}

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
		
		public boolean removeEater() {
			if (m_Type != kEaterCell) {
				return false;
			}
			m_Type = kEmptyCell;
			m_Eater = null;
			return true;
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
			return Integer.toString(m_Type);
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
	
	boolean load(String mapFile) {
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
			
			boolean random = cells.getAttributeBooleanDefault(kParamRandom, false);
						
			// Create map array
			m_World = new Cell[m_WorldHeight][m_WorldWidth];
			
			// Reset food
			m_FoodCount = 0;
			
			// generate
			if (random) {
				generateRandomMap();
			} else {
				generateMapFromXML(cells);
			}
			
		} catch (Exception e) {
			m_Logger.log("Error loading map: " + e.getMessage());
			return false;
		}
		
		resetEaters();
		
		m_Logger.log(mapFile + " loaded.");
		return true;
	}
	
	private void generateMapFromXML(JavaElementXML cells) throws Exception {
		for(int row = 0; row < m_WorldHeight; ++row) {
			String rowString = new String();
			for (int col = 0; col < m_WorldWidth; ++col) {
				try {
					m_World[row][col] = new Cell(cells.getChild(row).getChild(col).getAttributeThrows(kParamType));
					rowString += m_World[row][col];
				} catch (Exception e) {
					throw new Exception("Error on row: " + row + ", column: " + col);
				}
			}
			m_Logger.log(rowString);
		}
	}
	
	private void generateRandomMap() throws Exception {
		// Generate perimiter wall
		for (int row = 0; row < m_WorldWidth; ++row) {
			m_World[row][0] = m_World[row][m_WorldHeight - 1] = new Cell(kTypeWall);
		}
		for (int col = 1; col < m_WorldHeight - 1; ++col) {
			m_World[0][col] = m_World[m_WorldWidth - 1][col] = new Cell(kTypeWall);
		}
		
		Random random = new Random();
		
		double probability = kLowProbability;
		for (int row = 2; row < m_WorldWidth - 2; ++row) {
			for (int col = 2; col < m_WorldHeight - 2; ++col) {
				if (noWallsOnCorners(row, col)) {
					if (wallOnAnySide(row, col)) {
						probability = kHigherProbability;					
					}
					if (random.nextDouble() < probability) {
						m_World[row][col] = new Cell(kTypeWall);
					} else {
						m_World[row][col] = new Cell(kTypeEmpty);
					}
					probability = kLowProbability;
				}
			}
		}
		fillFood();
	}
	
	private boolean noWallsOnCorners(int row, int col) {
		Cell cell = m_World[row + 1][col + 1];
		if (cell != null) {
			if (cell.isWall()) {
				return false;
			}
		}
		
		cell = m_World[row - 1][col - 1];
		if (cell != null) {
			if (cell.isWall()) {
				return false;
			}
		}
		
		cell = m_World[row + 1][col - 1];
		if (cell != null) {
			if (cell.isWall()) {
				return false;
			}
		}
		
		cell = m_World[row - 1][col + 1];
		if (cell != null) {
			if (cell.isWall()) {
				return false;
			}
		}
		return true;
	}
	
	private boolean wallOnAnySide(int row, int col) {
		Cell cell = m_World[row + 1][col];
		if (cell != null) {
			if (cell.isWall()) {
				return true;
			}
		}
		
		cell = m_World[row][col + 1];
		if (cell != null) {
			if (cell.isWall()) {
				return true;
			}
		}
		
		cell = m_World[row - 1][col];
		if (cell != null) {
			if (cell.isWall()) {
				return true;
			}
		}
		
		cell = m_World[row][col - 1];
		if (cell != null) {
			if (cell.isWall()) {
				return true;
			}
		}
		return false;
	}
	
	private void fillFood() {
		Random random = new Random();
		for (int row = 1; row < m_WorldWidth - 1; ++row) {
			for (int col = 1; col < m_WorldHeight - 1; ++col) {
				if (m_World[row][col] != null) {
					if (!m_World[row][col].isEmpty()) {
						continue;
					}
				}
				m_World[row][col] = new Cell(random.nextInt(m_Food.length));
			}
		}		
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
				if (!getCell(oldLocation).removeEater()) {
					m_Logger.log("Warning: moving eater " + m_Eaters[i].getName() + " not at old location " + oldLocation);
				}
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
	
	ArrayList m_Collisions;
	
	void handleCollisions() {
		// generate collision groups
		ArrayList currentCollision = null;
		
		for (int i = 0; i < m_Eaters.length; ++i) {
			for (int j = i+1; j < m_Eaters.length; ++j) {
				// only check eaters who aren't already colliding
				if (m_Eaters[i].isColliding()) {
					continue;
				}
				
				if (m_Eaters[i].getLocation().equals(m_Eaters[j].getLocation())) {
					
					// Create data structures
					if (m_Collisions == null) {
						m_Collisions = new ArrayList();
					}
					if (currentCollision == null) {
						currentCollision = new ArrayList();
						
						// Add first agent to current collision
						currentCollision.add(m_Eaters[i]);
						// Flipping collision flag unnecessary as first agent will not be traversed again

						m_Logger.log("Starting collision group at " + m_Eaters[i].getLocation());
					}
					
					// Add second agent to current collision
					currentCollision.add(m_Eaters[j]);

					// Flip collision flag for second agent
					m_Eaters[j].setColliding(true);
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
			Eater[] collidees = (Eater[])currentCollision.toArray(new Eater[0]);
			
			m_Logger.log("Processing collision group " + group + " with " + collidees.length + " collidees.");
			
			// Redistribute wealth
			int cash = 0;			
			for (int i = 0; i < collidees.length; ++i) {
				cash += collidees[i].getScore();
			}			
			cash /= collidees.length;
			m_Logger.log("Cash to each: " + cash);
			for (int i = 0; i < collidees.length; ++i) {
				collidees[i].setScore(cash);
			}
			
			// Remove from former location (only one of these for all eaters)
			getCell(collidees[0].getLocation()).removeEater();

			// Find new locations, update map and consume as necessary
			for (int i = 0; i < collidees.length; ++i) {
				collidees[i].setLocation(findStartingLocation());
				Food f = getCell(collidees[i].getLocation()).setEater(collidees[i]);
				if (f != null) {
					collidees[i].adjustScore(f.getValue());
				}
			}
		}
		
		// clear collision groups
		m_Collisions = null;
		
		// clear colliding flags
		for (int i = 0; i < m_Eaters.length; ++i) {
			m_Eaters[i].setColliding(false);
		}		
	}
	
	public Eater[] getEaters() {
		return m_Eaters;
	}
}

