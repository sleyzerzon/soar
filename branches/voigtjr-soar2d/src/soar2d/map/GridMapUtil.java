package soar2d.map;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.apache.log4j.Logger;
import org.jdom.Document;
import org.jdom.Element;
import org.jdom.JDOMException;
import org.jdom.input.SAXBuilder;

import soar2d.Direction;
import soar2d.Simulation;
import soar2d.Soar2D;
import soar2d.map.Names;

class GridMapUtil {
	private static Logger logger = Logger.getLogger(GridMapUtil.class);

	static GridMapData loadFromFile(File mapFile, CellObjectObserver observer) throws Exception {
		GridMapData data = new GridMapData();
		data.cellObjectManager = new CellObjectManager();
		
		try {
			SAXBuilder builder = new SAXBuilder();
			Document doc = builder.build(mapFile);
			Element root = doc.getRootElement();
			if (root == null || !root.getName().equalsIgnoreCase(Names.kTagMap)) {
				throw new Exception("Couldn't find map tag in map file.");
			}
			
			List<Element> children = (List<Element>)root.getChildren();
			Iterator<Element> iter = children.iterator();
			while (iter.hasNext()) {
				Element child = iter.next();
				
				if (child.getName().equalsIgnoreCase(Names.kTagCellObject)) {
					cellObject(data, child);
				} else if (child.getName().equalsIgnoreCase(Names.kTagCells)) {
					cells(data, child, observer);
				} else if (child.getName().equalsIgnoreCase(Names.kTagMetadata)) {
					data.metadataFile = new File(child.getTextTrim());
				} else {
					throw new Exception("unrecognized tag: " + child.getName());
				}
			}
		} catch (IOException e) {
			throw new Exception("I/O exception: " + e.getMessage());
		} catch (JDOMException e) {
			throw new Exception("Error during parsing: " + e.getMessage());
		} catch (IllegalStateException e) {
			throw new Exception("Illegal state: " + e.getMessage());
		}
		
		// Build cell reference map
		assert data.cells != null;
		int [] xy = new int[2];
		for (xy[0] = 0; xy[0] < data.cells.size(); ++xy[0]) {
			for (xy[1] = 0; xy[1] < data.cells.size(); ++xy[1]) {
				Cell cell = data.cells.getCell(xy);
				logger.trace(cell);
				for (Direction dir : Direction.values()) {
					if (dir == Direction.NONE) {
						continue;
					}
					int[] neighborLoc = Direction.translate(cell.location, dir, new int[2]);
					if (data.cells.isInBounds(neighborLoc)) {
						logger.trace(dir);
						Cell neighbor = data.cells.getCell(neighborLoc);
						cell.neighbors[dir.index()] = neighbor;
					}
				}
			}
		}
		
		return data;
	}

	private static void cellObject(GridMapData data, Element cellObject) throws Exception {
		String name = cellObject.getAttributeValue(Names.kAttrName);
		if (name == null || name.length() <= 0) {
			throw new Exception("cell-object must have name");
		}
		
		CellObject template = new CellObject(name);

		List<Element> children = (List<Element>)cellObject.getChildren();
		Iterator<Element> iter = children.iterator();
		while (iter.hasNext()) {
			Element child = iter.next();
			
			if (child.getName().equalsIgnoreCase(Names.kTagProperty)) {
				property(data, child, template, false);
				
			} else if (child.getName().equalsIgnoreCase(Names.kTagApply)) {
				apply(data, child, template);

			} else if (child.getName().equalsIgnoreCase(Names.kTagUpdate)) {
				update(data, child, template);

			} else {
				throw new Exception("Unrecognized tag: " + child.getName());
			}
		}		
		
		data.cellObjectManager.registerTemplate(template);
	}
	
	private static void cells(GridMapData data, Element cells, CellObjectObserver observer) throws Exception {
		String sizeString = cells.getAttributeValue(Names.kAttrWorldSize);
		if (sizeString == null || sizeString.length() <= 0) {
			throw new Exception("world-size required with cells tag");
		}
		
		int size = -1;
		try {
			size = Integer.parseInt(sizeString);
		} catch (NumberFormatException e) {
			throw new Exception("error parsing world-size");
		}

		data.cells = new GridMapCells(size);
		
		data.randomWalls = Boolean.parseBoolean(cells.getAttributeValue(Names.kAttrRandomWalls, "false"));
		data.randomFood = Boolean.parseBoolean(cells.getAttributeValue(Names.kAttrRandomFood, "false"));
		
		// Generate map from XML unless both are true
		if (!data.randomWalls || !data.randomFood) {
			if (cells.getChildren().size() != size) {
				throw new Exception("there does not seem to be the " +
						"correct amount of row tags (" + cells.getChildren().size() +
						") for the specified map size (" + size + ")");
			}
			
			List<Element> children = (List<Element>)cells.getChildren();
			Iterator<Element> iter = children.iterator();
			int rowIndex = 0;
			while (iter.hasNext()) {
				Element child = iter.next();

				if (!child.getName().equalsIgnoreCase(Names.kTagRow)) {
					throw new Exception("unrecognized tag: " + child.getName());
				}
				
				row(data, child, rowIndex, observer);
				
				rowIndex += 1;
			}
		}
		
		// override walls if necessary
		if (data.randomWalls) {
			generateRandomWalls(data, observer);
		}
		
		// override food if necessary
		if (data.randomFood) {
			generateRandomFood(data, observer);
		}
		
		// pick positive box
		if (data.rewardInfoObject != null) {
			assert data.positiveRewardID == 0;
			data.positiveRewardID = Simulation.random.nextInt(data.cellObjectManager.rewardObjects.size());
			data.positiveRewardID += 1;
			data.rewardInfoObject.addPropertyApply(soar2d.Names.kPropertyPositiveBoxID, Integer.toString(data.positiveRewardID));
			
			// assigning colors like this helps us see the task happen
			// colors[0] = info box (already assigned)
			// colors[1] = positive reward box
			// colors[2..n] = negative reward boxes
			int negativeColor = 2;
			for (CellObject aBox : data.cellObjectManager.rewardObjects) {
				if (aBox.getIntProperty(soar2d.Names.kPropertyBoxID) == data.positiveRewardID) {
					aBox.setRewardBox();
					aBox.addProperty(soar2d.Names.kPropertyColor, Soar2D.simulation.kColors[1]);
				} else {
					aBox.addProperty(soar2d.Names.kPropertyColor, Soar2D.simulation.kColors[negativeColor]);
					negativeColor += 1;
					assert negativeColor < Soar2D.simulation.kColors.length;
				}
			}

			// if using an open code, assign that
			if (data.openCode != 0) {
				data.rewardInfoObject.setOpenCode(data.openCode);
				data.rewardInfoObject.addPropertyApply(soar2d.Names.kPropertyOpenCode, Integer.toString(data.openCode));
			}
		}
	}
	
	private static void property(GridMapData data, Element property, CellObject template, boolean apply) throws Exception {
		String name = property.getAttributeValue(Names.kAttrName);
		if (name == null || name.length() <= 0) {
			throw new Exception("property must have name");
		}
		
		String value = property.getAttributeValue(Names.kAttrValue);
		if (value == null || value.length() <= 0) {
			throw new Exception("property must have value");
		}
		
		if (apply) {
			template.addPropertyApply(name, value);
		} else {
			template.addProperty(name, value);
		}
	}
	
	private static void apply(GridMapData data, Element apply, CellObject template) throws Exception {
		List<Element> children = (List<Element>)apply.getChildren();
		Iterator<Element> iter = children.iterator();
		while (iter.hasNext()) {
			Element child = iter.next();
			
			if (child.getName().equalsIgnoreCase(Names.kTagProperty)) {
				property(data, child, template, true);
				
			} else if (child.getName().equalsIgnoreCase(Names.kTagPoints)) {
				template.setPointsApply(true);

			} else if (child.getName().equalsIgnoreCase(Names.kTagEnergy)) {
				energy(child, template);

			} else if (child.getName().equalsIgnoreCase(Names.kTagHealth)) {
				health(child, template);
				
			} else if (child.getName().equalsIgnoreCase(Names.kTagMissiles)) {
				template.setMissilesApply(true);

			} else if (child.getName().equalsIgnoreCase(Names.kTagRemove)) {
				template.setRemoveApply(true);

			} else if (child.getName().equalsIgnoreCase(Names.kTagRewardInfo)) {
				template.setRewardInfoApply(true);
				if (child.getChild(Names.kTagUseOpenCode) != null) {
					data.openCode = Simulation.random.nextInt(data.kOpenCodeRange) + 1;
					logger.info("The correct open code is: " + data.openCode);
				}

			} else if (child.getName().equalsIgnoreCase(Names.kTagReward)) {
				reward(child, template);

			} else if (child.getName().equalsIgnoreCase(Names.kTagReset)) {
				template.setResetApply(true);

			} else if (child.getName().equalsIgnoreCase(Names.kTagFuel)) {
				template.setFuelApply(true);

			} else {
				throw new Exception("Unrecognized tag: " + child.getName());
			}
		}		
	}
	
	private static void update(GridMapData data, Element update, CellObject template) throws Exception {
		List<Element> children = (List<Element>)update.getChildren();
		Iterator<Element> iter = children.iterator();
		while (iter.hasNext()) {
			Element child = iter.next();
			
			if (child.getName().equalsIgnoreCase(Names.kTagDecay)) {
				template.setDecayUpdate(true);
				
			} else if (child.getName().equalsIgnoreCase(Names.kTagFlyMissile)) {
				template.setFlyMissileUpdate(true);

			} else if (child.getName().equalsIgnoreCase(Names.kTagLinger)) {
				template.setLingerUpdate(true);

			} else {
				throw new Exception("Unrecognized tag: " + child.getName());
			}
		}		
	}

	private static void row(GridMapData data, Element row, int rowIndex, CellObjectObserver observer) throws Exception {
		if (row.getChildren().size() != data.cells.size()) {
			throw new Exception("there does not seem to be the " +
					"correct amount of cell tags (" + row.getChildren().size() +
					") for the specified map size (" + data.cells.size() + ")");
		}
		
		
		List<Element> children = (List<Element>)row.getChildren();
		Iterator<Element> iter = children.iterator();
		int colIndex = 0;
		while (iter.hasNext()) {
			Element child = iter.next();

			if (!child.getName().equalsIgnoreCase(Names.kTagCell)) {
				throw new Exception("unrecognized tag: " + child.getName());
			}

			int[] xy = new int[] {colIndex, rowIndex};
			Cell newCell = Cell.createCell(Soar2D.config.generalConfig().headless, xy);
			newCell.addObserver(data);
			newCell.addObserver(observer);
			data.cells.setCell(new int[] {rowIndex, colIndex}, newCell);
			cell(data, child, new int [] { colIndex, rowIndex }, newCell);
			
			colIndex += 1;
		}
	}
	
	private static void cell(GridMapData data, Element cellElement, int [] location, Cell cell) throws Exception {
		List<Element> children = (List<Element>)cellElement.getChildren();
		Iterator<Element> iter = children.iterator();
		while (iter.hasNext()) {
			Element child = iter.next();
			
			if (!child.getName().equalsIgnoreCase(Names.kTagObject)) {
				throw new Exception("unrecognized tag: " + child.getName());
			}
			
			object(data, child, location, cell);
		}
	}
	
	private static void object(GridMapData data, Element object, int [] location, Cell cell) throws Exception {
		String name = object.getTextTrim();
		if (name.length() <= 0) {
			throw new Exception("object doesn't have name");
		}
		
		if (!data.cellObjectManager.hasTemplate(name)) {
			throw new Exception("object \"" + name + "\" does not map to a cell object");
		}
		
		CellObject cellObject = data.cellObjectManager.createObject(name);
		cell.addObject(cellObject);

		// TODO: move to CellObjectObserver
		if (cellObject.rewardInfoApply) {
			if (data.rewardInfoObject != null) {
				throw new Exception("Multiple reward info objects detected.");
			}
			data.rewardInfoObject = cellObject;
		}
	}

	private static void energy(Element energy, CellObject template) {
		boolean shields = Boolean.parseBoolean(energy.getAttributeValue(Names.kAttrShields, "false"));
		template.setEnergyApply(true, shields);
	}

	private static void health(Element health, CellObject template) {
		boolean shieldsDown = Boolean.parseBoolean(health.getAttributeValue(Names.kAttrShieldsDown, "false"));
		template.setHealthApply(true, shieldsDown);
	}


	private static void reward(Element reward, CellObject template) throws Exception {
		String quantityString = reward.getAttributeValue(Names.kAttrQuantity);
		if (quantityString == null) {
			throw new Exception("No reward quantity specified.");
		}
		try {
			template.setRewardApply(Integer.parseInt(quantityString));
		} catch (NumberFormatException e) {
			throw new Exception("Invalid reward quantity specified.");
		}
	}

	private static void generateRandomWalls(GridMapData data, CellObjectObserver observer) throws Exception {
		if (!data.cellObjectManager.hasTemplatesWithProperty(soar2d.Names.kPropertyBlock)) {
			throw new Exception("tried to generate random walls with no blocking types");
		}
		
		assert data.cells != null;
		int size = data.cells.size();
		
		// Generate perimiter wall
		int[] xy = new int[2];
		for (xy[0] = 0; xy[0] < size; ++xy[0]) {
			for (xy[1] = 0; xy[1] < size; ++xy[1]) {
				if (xy[0] != 0 || xy[0] != size - 1 || xy[1] != 0 || xy[1] != size - 1) {
					// not on the edge
					continue;
				}
				removeFoodAndAddWall(data, xy, observer);
			}
		}

		double probability = Soar2D.config.eatersConfig().low_probability;
		for (xy[0] = 2; xy[0] < size - 3; ++xy[0]) {
			for (xy[1] = 2; xy[1] < size - 3; ++xy[1]) {

				if (noWallsOnCorners(data, xy)) {
					if (wallOnAnySide(data, xy)) {
						probability = Soar2D.config.eatersConfig().high_probability;					
					}
					if (Simulation.random.nextDouble() < probability) {
						removeFoodAndAddWall(data, xy, observer);
					}
					probability = Soar2D.config.eatersConfig().low_probability;
				}
			}
		}
	}
	
	private static void removeFoodAndAddWall(GridMapData data, int[] xy, CellObjectObserver observer) {
		Cell cell = data.cells.getCell(xy);
		if (cell == null) {
			cell = Cell.createCell(Soar2D.config.generalConfig().headless, xy);
			cell.addObserver(data);
			cell.addObserver(observer);
		}
		cell.removeAllByProperty(soar2d.Names.kPropertyEdible);
		CellObject wall = data.cellObjectManager.createRandomObjectWithProperty(soar2d.Names.kPropertyBlock);
		cell.addObject(wall);
	}
	
	private static boolean noWallsOnCorners(GridMapData data, int[] xy) {
		Cell cell;
		
		cell = data.cells.getCell(new int[] {xy[0] + 1, xy[1] + 1});
		if (cell != null && cell.hasAnyWithProperty(soar2d.Names.kPropertyBlock)) {
			return false;
		}
		cell = data.cells.getCell(new int[] {xy[0] - 1, xy[1] + 1});
		if (cell != null && cell.hasAnyWithProperty(soar2d.Names.kPropertyBlock)) {
			return false;
		}
		cell = data.cells.getCell(new int[] {xy[0] - 1, xy[1] - 1});
		if (cell != null && cell.hasAnyWithProperty(soar2d.Names.kPropertyBlock)) {
			return false;
		}
		cell = data.cells.getCell(new int[] {xy[0] + 1, xy[1] - 1});
		if (cell != null && cell.hasAnyWithProperty(soar2d.Names.kPropertyBlock)) {
			return false;
		}
		return true;
	}
	
	private static boolean wallOnAnySide(GridMapData data, int[] xy) {
		Cell cell;
		
		cell = data.cells.getCell(new int[] {xy[0] + 1, xy[1]});
		if (cell != null && cell.hasAnyWithProperty(soar2d.Names.kPropertyBlock)) {
			return true;
		}
		cell = data.cells.getCell(new int[] {xy[0], xy[1] + 1});
		if (cell != null && cell.hasAnyWithProperty(soar2d.Names.kPropertyBlock)) {
			return true;
		}
		cell = data.cells.getCell(new int[] {xy[0] - 1, xy[1]});
		if (cell != null && cell.hasAnyWithProperty(soar2d.Names.kPropertyBlock)) {
			return true;
		}
		cell = data.cells.getCell(new int[] {xy[0], xy[1] - 1});
		if (cell != null && cell.hasAnyWithProperty(soar2d.Names.kPropertyBlock)) {
			return true;
		}
		return false;
	}
	
	private static void generateRandomFood(GridMapData data, CellObjectObserver observer) throws Exception {
		if (!data.cellObjectManager.hasTemplatesWithProperty(soar2d.Names.kPropertyEdible)) {
			throw new Exception("tried to generate random walls with no food types");
		}
		
		int[] xy = new int[2];
		for (xy[0] = 1; xy[0] < data.cells.size() - 1; ++xy[0]) {
			for (xy[1] = 1; xy[1] < data.cells.size() - 1; ++xy[1]) {
				Cell cell = data.cells.getCell(xy);
				if (cell == null) {
					cell = Cell.createCell(Soar2D.config.generalConfig().headless, xy);
					cell.addObserver(data);
					cell.addObserver(observer);
				}
				
				if (!cell.hasAnyWithProperty(soar2d.Names.kPropertyBlock)) {
					cell.removeAllByProperty(soar2d.Names.kPropertyEdible);
					CellObject wall = data.cellObjectManager.createRandomObjectWithProperty(soar2d.Names.kPropertyEdible);
					cell.addObject(wall);
				}
			}
		}		
	}
	
	static int[] getAvailableLocationAmortized(GridMap map) {
		int size = map.size();
		
		// Loop in case there are no free spots, the 100 is totally arbitrary
		int [] xy = new int [2];
		for (int counter = 0; counter < 100; ++counter) {
			xy[0] = Simulation.random.nextInt(size - 2) + 1;
			xy[1] = Simulation.random.nextInt(size - 2) + 1;
			
			if (map.isAvailable(xy)) {
				return xy;
			}
		}
		ArrayList<int []> locations = new ArrayList<int []>();
		for (xy[0] = 0; xy[0] < size; ++xy[0]) {
			for (xy[1] = 0; xy[1] < size; ++ xy[1]) {
				if (map.isAvailable(xy)) {
					locations.add(xy);
				}
			}
		}
		if (locations.size() == 0) {
			return null;
		}
		return locations.get(Simulation.random.nextInt(locations.size()));
	}
	
	static String toString(GridMapData data) {
		StringBuilder output = new StringBuilder();
		int size = data.cells.size();

		int [] xy = new int [2];
		for (xy[0] = 0; xy[0] < size; ++xy[0]) {
			for (xy[1] = 0; xy[1] < size; ++ xy[1]) {
				StringBuilder cellString = new StringBuilder();
				cellString.append(xy[0]);
				cellString.append(",");
				cellString.append(xy[1]);
				cellString.append(":\n");
				
				Cell cell = data.cells.getCell(xy);
				Iterator<CellObject> iter = cell.getAll().iterator();
				while (iter.hasNext()) {
					CellObject object = iter.next();
					cellString.append("\t");
					cellString.append(object.getName());
					cellString.append(": ");
					
					Iterator<String> propIter = object.properties.keySet().iterator();
					while (propIter.hasNext()) {
						String key = propIter.next();
						cellString.append(key);
						cellString.append(":");
						cellString.append(object.properties.get(key));
						cellString.append(", ");
					}
					cellString.append("\n");
				}
				output.append(cellString);
			}
		}
		return output.toString();
	}
}
