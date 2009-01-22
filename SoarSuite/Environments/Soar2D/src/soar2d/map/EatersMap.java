package soar2d.map;

import java.util.HashSet;
import java.util.Iterator;

import soar2d.Names;
import soar2d.Soar2D;
import soar2d.world.TankSoarWorld;

public class EatersMap extends GridMap {

	public EatersMap() {
	}

	@Override
	public void addObjectToCell(int [] location, CellObject object) {
		Cell cell = getCell(location);
		if (cell.hasObject(object.getName())) {
			CellObject old = cell.removeObject(object.getName());
			assert old != null;
			updatables.remove(old);
			updatablesLocations.remove(old);
			removalStateUpdate(old);
		}
		if (object.updatable()) {
			updatables.add(object);
			updatablesLocations.put(object, location);
		}
		if (Soar2D.config.terminalsConfig().unopened_boxes) {
			if (isUnopenedBox(object)) {
				unopenedBoxes.add(object);
			}
		}
		
		// Update state we keep track of specific to game type
		if (object.hasProperty(Names.kPropertyEdible)) {
			foodCount += 1;
		}
		if (object.hasProperty(Names.kPropertyPoints)) {
			scoreCount += object.getIntProperty(Names.kPropertyPoints);
		}

		cell.addCellObject(object);
		setRedraw(cell);

	}

	@Override
	public void updateObjects(TankSoarWorld tsWorld) {
		if (!updatables.isEmpty()) {
			Iterator<CellObject> iter = updatables.iterator();
			
			while (iter.hasNext()) {
				CellObject cellObject = iter.next();
				int [] location = updatablesLocations.get(cellObject);
				assert location != null;
				
				int previousScore = setPreviousScore(cellObject);

				if (cellObject.update(location)) {
					Cell cell = getCell(location);
					
					cellObject = cell.removeObject(cellObject.getName());
					assert cellObject != null;
					
					setRedraw(cell);
					
					iter.remove();
					updatablesLocations.remove(cellObject);
					removalStateUpdate(cellObject);
				}
				if (cellObject.hasProperty(Names.kPropertyPoints)) {
					scoreCount += cellObject.getIntProperty(Names.kPropertyPoints) - previousScore;
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
	
	@Override
	public boolean isAvailable(int [] location) {
		Cell cell = getCell(location);
		boolean enterable = !cell.hasAnyWithProperty(Names.kPropertyBlock);
		boolean noPlayer = cell.getPlayer() == null;
		return enterable && noPlayer;
	}
	
	@Override
	public void setExplosion(int [] location) {
		CellObject explosion = new CellObject(Names.kExplosion);
		explosion.addProperty(Names.kPropertyLinger, "2");
		explosion.setLingerUpdate(true);
		addObjectToCell(location, explosion);
	}
	
	private int setPreviousScore(CellObject cellObject) {
		if (cellObject.hasProperty(Names.kPropertyPoints)) {
			return cellObject.getIntProperty(Names.kPropertyPoints);
		}
		return 0;
	}
	
	@Override
	void removalStateUpdate(CellObject object) {
		if (Soar2D.config.terminalsConfig().unopened_boxes) {
			if (isUnopenedBox(object)) {
				unopenedBoxes.remove(object);
			}
		}
		
		if (object.hasProperty(Names.kPropertyEdible)) {
			foodCount -= 1;
		}
		if (object.hasProperty(Names.kPropertyPoints)) {
			scoreCount -= object.getIntProperty(Names.kPropertyPoints);
		}
	}
}
