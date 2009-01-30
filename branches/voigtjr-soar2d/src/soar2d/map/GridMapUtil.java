package soar2d.map;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.apache.log4j.Logger;

import soar2d.Direction;
import soar2d.Simulation;
import soar2d.Soar2D;
import soar2d.config.Config;
import soar2d.config.ConfigFile;

class GridMapUtil {
	private static Logger logger = Logger.getLogger(GridMapUtil.class);

	static GridMapData loadFromConfigFile(String mapPath, CellObjectObserver observer) throws Exception {
		return loadFromConfigFile(mapPath, observer, 0, 0);
	}
		
	static GridMapData loadFromConfigFile(String mapPath, CellObjectObserver observer, double lowProbability, double highProbability) throws Exception {
		File mapFile = new File(mapPath);
		if (!mapFile.exists()) {
			throw new Exception("Map file doesn't exist: " + mapFile.getAbsolutePath());
		}

		GridMapData data = new GridMapData();
		data.cellObjectManager = new CellObjectManager();
		
		String mapFilePath = mapFile.getAbsolutePath();
		Config mapConfig = new Config(new ConfigFile(mapFilePath));
		
		File objectsFile = new File(mapConfig.getString("objects_file"));
		String mapFileDirectory = mapFilePath.substring(0, mapFilePath.lastIndexOf(File.separatorChar) + 1);
		
		Config objectsConfig = new Config(new ConfigFile(mapFileDirectory + objectsFile.getName()));
		for (String id : mapConfig.getStrings("objects")) {
			CellObject template = new CellObject(objectsConfig.getChild("objects." + id));
			data.cellObjectManager.registerTemplate(template);
		}
		
		cellsConfig(data, mapConfig.getChild("cells"), objectsConfig, observer);
		
		if (mapConfig.hasKey("metadata")) {
			data.metadataFile = new File(mapConfig.getString("metadata"));
		}
		
		buildReferenceMap(data);
		return data;
	}

	private static void cellsConfig(GridMapData data, Config cellsConfig, Config objectsConfig, CellObjectObserver observer) throws Exception {
		cellsConfig(data, cellsConfig, objectsConfig, observer, 0, 0);
	}
	
	private static void cellsConfig(GridMapData data, Config cellsConfig, Config objectsConfig, CellObjectObserver observer, double lowProbability, double highProbability) throws Exception {
		data.cells = new GridMapCells(cellsConfig.requireInt("size"));
		
		data.randomWalls = cellsConfig.getBoolean("random_walls", false);
		data.randomFood = cellsConfig.getBoolean("random_food", false);
		
		// Generate map unless both are true
		if (!data.randomWalls || !data.randomFood) {
			Config rows = cellsConfig.getChild("rows");
			int[] xy = new int[2];
			for (xy[1] = 0; xy[1] < data.cells.size(); ++xy[1]) {

				String[] cellStrings = rows.getStrings(Integer.toString(xy[1]));
				if (cellStrings.length != data.cells.size()) {
					throw new Exception("Not enough cells, row " + xy[1]);
				}
				
				for (xy[0] = 0; xy[0] < data.cells.size(); ++xy[0]) {
					Cell newCell = Cell.createCell(xy);
					newCell.addObserver(data);
					newCell.addObserver(observer);
					data.cells.setCell(xy, newCell);
					
					// TODO: multiple objects
					String objectName = objectsConfig.getString("objects." + cellStrings[xy[0]] + ".name");
					
					logger.trace(Arrays.toString(xy) + ": " + objectName);
					
					if (!data.cellObjectManager.hasTemplate(objectName)) {
						throw new Exception("object \"" + objectName + "\" does not map to a cell object");
					}
					
					CellObject cellObject = data.cellObjectManager.createObject(objectName);
					newCell.addObject(cellObject);
				}
			}
		}
		
		// override walls if necessary
		if (data.randomWalls) {
			generateRandomWalls(data, observer, lowProbability, highProbability);
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
			logger.trace("reward-info.positive-id: " + data.positiveRewardID);
			data.rewardInfoObject.setIntProperty("apply.reward-info.positive-id", data.positiveRewardID);
			
			// assigning colors like this helps us see the task happen
			// colors[0] = info box (already assigned)
			// colors[1] = positive reward box
			// colors[2..n] = negative reward boxes
			int negativeColor = 2;
			for (CellObject aBox : data.cellObjectManager.rewardObjects) {
				if (aBox.getIntProperty("box-id", 0) == data.positiveRewardID) {
					aBox.setProperty("apply.reward.correct", "ignored");
					aBox.setProperty(soar2d.Names.kPropertyColor, Soar2D.simulation.kColors[1]);
				} else {
					aBox.setProperty(soar2d.Names.kPropertyColor, Soar2D.simulation.kColors[negativeColor]);
					negativeColor += 1;
					assert negativeColor < Soar2D.simulation.kColors.length;
				}
			}

			// if using an open code, assign that
			if (data.openCode != 0) {
				data.rewardInfoObject.setIntProperty("apply.reward-info.code", data.openCode);
			}
		}
	}
	
	private static void buildReferenceMap(GridMapData data) {
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
	}

	private static void generateRandomWalls(GridMapData data, CellObjectObserver observer, double lowProbability, double highProbability) throws Exception {
		if (!data.cellObjectManager.hasTemplatesWithProperty(soar2d.Names.kPropertyBlock)) {
			throw new Exception("tried to generate random walls with no blocking types");
		}
		
		logger.trace("Generating random walls.");
		
		assert data.cells != null;
		int size = data.cells.size();
		
		// Generate perimiter wall and remove existing walls
		int[] xy = new int[2];
		for (xy[0] = 0; xy[0] < size; ++xy[0]) {
			for (xy[1] = 0; xy[1] < size; ++xy[1]) {
				if (xy[0] == 0 || xy[0] == size - 1 || xy[1] == 0 || xy[1] == size - 1) {
					logger.trace(Arrays.toString(xy) + ": Confirming perimeter wall.");
					removeFoodAndAddWall(data, xy, observer);
					continue;
				}
				// not on the edge
				if (data.cells.getCell(xy).removeAllByProperty(soar2d.Names.kPropertyBlock) != null) {
					logger.trace(Arrays.toString(xy) + ": Removed non-perimiter wall.");
				}
			}
		}

		for (xy[0] = 2; xy[0] < size - 3; ++xy[0]) {
			for (xy[1] = 2; xy[1] < size - 3; ++xy[1]) {

				if (noWallsOnCorners(data, xy)) {
					double probability = lowProbability;
					if (wallOnAnySide(data, xy)) {
						logger.trace(Arrays.toString(xy) + ": High probability.");
						probability = highProbability;					
					}
					if (Simulation.random.nextDouble() < probability) {
						removeFoodAndAddWall(data, xy, observer);
					}
				}
			}
		}
	}
	
	private static void removeFoodAndAddWall(GridMapData data, int[] xy, CellObjectObserver observer) {
		logger.trace(Arrays.toString(xy) + ": Changing to wall.");
		Cell cell = data.cells.getCell(xy);
		if (cell == null) {
			cell = Cell.createCell(xy);
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

		logger.trace("Generating random food.");
		
		int[] xy = new int[2];
		for (xy[0] = 1; xy[0] < data.cells.size() - 1; ++xy[0]) {
			for (xy[1] = 1; xy[1] < data.cells.size() - 1; ++xy[1]) {
				Cell cell = data.cells.getCell(xy);
				if (cell == null) {
					cell = Cell.createCell(xy);
					cell.addObserver(data);
					cell.addObserver(observer);
				}
				
				if (!cell.hasAnyWithProperty(soar2d.Names.kPropertyBlock)) {
					logger.trace(Arrays.toString(xy) + "Adding random food.");
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
		List<int []> locations = new ArrayList<int []>();
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
				output.append(xy[0]);
				output.append(",");
				output.append(xy[1]);
				output.append(":\n");
				
				Cell cell = data.cells.getCell(xy);
				for (CellObject object : cell.getAll()) {
					output.append("\t");
					output.append(object);
					output.append("\n");
				}
			}
		}
		return output.toString();
	}

	static void lingerUpdate(CellObject cellObject, Cell cell) {
		if (cellObject.hasProperty("update.linger")) {
			int linger = cellObject.getIntProperty("update.linger", 0);
			linger -= 1;
			if (linger <= 0) {
				cell.removeObject(cellObject.getName());
			} else {
				cellObject.setIntProperty("update.linger", linger);
			}
		}
	}
}
