package soar2d.map;

import java.io.File;
import java.util.HashMap;
import java.util.HashSet;

class GridMapData implements CellObjectObserver {
	CellObjectManager cellObjectManager;
	GridMapCells cells;
	File metadataFile;

	HashSet<CellObject> updatables = new HashSet<CellObject>();
	HashMap<CellObject, int []> updatablesLocations = new HashMap<CellObject, int []>();

	public void addStateUpdate(int [] location, CellObject added) {
		if (added.updatable()) {
			updatables.add(added);
			updatablesLocations.put(added, location);
		}
	}
	
	public void removalStateUpdate(int [] location, CellObject removed) {
		if (removed.updatable()) {
			updatables.remove(removed);
			updatablesLocations.remove(removed);
		}
	}
	
	boolean randomWalls; // TODO: move out of here?
	boolean randomFood;
	
	CellObject rewardInfoObject; // TODO: move to CellObjectObserver code
	int openCode = 0; // TODO: move out of here
	int kOpenCodeRange = 2; // 1..kOpenCodeRange (alternatively expressed as: 0..(kOpenCodeRange - 1) + 1
	int positiveRewardID = 0;
}

