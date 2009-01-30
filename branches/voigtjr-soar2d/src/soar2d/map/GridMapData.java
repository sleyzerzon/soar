package soar2d.map;

import java.io.File;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import org.apache.log4j.Logger;

class GridMapData implements CellObjectObserver {
	private static Logger logger = Logger.getLogger(GridMapData.class);

	CellObjectManager cellObjectManager;
	GridMapCells cells;
	File metadataFile;

	Set<CellObject> updatables = new HashSet<CellObject>();
	Map<CellObject, int []> updatablesLocations = new HashMap<CellObject, int []>();

	public void addStateUpdate(int [] location, CellObject added) {
		if (added.updatable()) {
			logger.trace("Adding updatable " + added.getName() + " at " + Arrays.toString(location));
			updatables.add(added);
			updatablesLocations.put(added, location);
		}
	}
	
	public void removalStateUpdate(int [] location, CellObject removed) {
		if (removed.updatable()) {
			logger.trace("Removing updatable " + removed.getName() + " at " + Arrays.toString(location));
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

