package soar2d.map;

import java.io.File;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;

import soar2d.Names;
import soar2d.Soar2D;

public class EatersMap implements GridMap, CellObjectObserver {

	GridMapData data;
	
	public EatersMap(File mapFile) throws Exception {
		data = GridMapUtil.loadFromConfigFile(mapFile, this);
	}
	
	public int size() {
		return data.cells.size();
	}
	
	public Cell getCell(int[] xy) {
		return data.cells.getCell(xy);
	}

	public boolean isAvailable(int[] location) {
		Cell cell = data.cells.getCell(location);
		boolean enterable = !cell.hasAnyWithProperty(Names.kPropertyBlock);
		boolean noPlayer = cell.getPlayer() == null;
		return enterable && noPlayer;
	}

	public boolean isInBounds(int[] xy) {
		return data.cells.isInBounds(xy);
	}
	
	public int[] getAvailableLocationAmortized() {
		return GridMapUtil.getAvailableLocationAmortized(this);
	}

	public void addStateUpdate(int [] location, CellObject added) {
		// Update state we keep track of specific to game type
		if (added.hasProperty(Names.kPropertyEdible)) {
			foodCount += 1;
		}
		if (added.hasProperty("apply.points")) {
			scoreCount += added.getIntProperty("apply.points", 0);
		}
	}

	public void removalStateUpdate(int [] location, CellObject removed) {
		if (Soar2D.config.terminalsConfig().unopened_boxes) {
			if (isUnopenedBox(removed)) {
				unopenedBoxes.remove(removed);
			}
		}
		
		if (removed.hasProperty(Names.kPropertyEdible)) {
			foodCount -= 1;
		}
		if (removed.hasProperty(Names.kPropertyPoints)) {
			scoreCount -= removed.getIntProperty("apply.points", 0);
		}
	}

	public void updateObjects() {
		HashSet<CellObject> copy = new HashSet<CellObject>(data.updatables);
		for (CellObject cellObject : copy) {
			int [] location = data.updatablesLocations.get(cellObject);
			
			// linger
			if (cellObject.hasProperty("update.linger")) {
				int linger = cellObject.getIntProperty("update.linger", 0);
				linger -= 1;
				if (linger <= 0) {
					getCell(location).removeObject(cellObject.getName());
				} else {
					cellObject.setIntProperty("update.linger", linger);
				}
			}

			// decay
			if (cellObject.hasProperty("update.decay")) {
				int points = cellObject.getIntProperty("apply.points", 0);
				int decay = cellObject.getIntProperty("update.decay", 1);
				if (decay >= points) {
					scoreCount -= points;
					getCell(location).removeObject(cellObject.getName());
				} else {
					scoreCount -= decay;
					points -= decay;
					cellObject.setIntProperty("apply.points", points);
				}
			}
		}

		if (Soar2D.config.terminalsConfig().unopened_boxes) {
			Iterator<CellObject> iter = unopenedBoxes.iterator();
			while (iter.hasNext()) {
				CellObject box = iter.next();
				if (!isUnopenedBox(box)) {
					iter.remove();
				}
			}
		}
	}
	
	private boolean isUnopenedBox(CellObject object) {
		if (object.hasProperty(Names.kPropertyBox)) {
			String status = object.getProperty(Names.kPropertyStatus);
			if (status == null || !status.equals(Names.kOpen)) {
				return true;
			}
		}
		return false;
	}
	
	int scoreCount = 0;
	public int getScoreCount() {
		return scoreCount;
	}
	
	int foodCount = 0;
	public int getFoodCount() {
		return foodCount;
	}
	
	HashSet<CellObject> unopenedBoxes = new HashSet<CellObject>();
	public int getUnopenedBoxCount() {
		return unopenedBoxes.size();
	}
	
	public CellObject createObjectByName(String name) {
		return data.cellObjectManager.createObject(name);
	}

	public File getMetadataFile() {
		return data.metadataFile;
	}

	public ArrayList<CellObject> getTemplatesWithProperty(String name) {
		return data.cellObjectManager.getTemplatesWithProperty(name);
	}
}
