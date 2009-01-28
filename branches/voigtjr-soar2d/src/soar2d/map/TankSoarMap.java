package soar2d.map;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;

import soar2d.Direction;
import soar2d.Names;
import soar2d.Soar2D;
import soar2d.players.Player;
import soar2d.players.Player;
import soar2d.world.TankSoarWorld;

public class TankSoarMap implements GridMap, CellObjectObserver {
	
	GridMapData data;

	public TankSoarMap(File mapFile) throws Exception {
		data = GridMapUtil.loadFromFile(mapFile, this);

		// Add ground to cells that don't have a background.
		int size = data.cells.size();
		int [] xy = new int[2];
		for (xy[0] = 0; xy[0] < size; ++xy[0]) {
			for (xy[1] = 0; xy[1] < size; ++xy[1]) {
				Cell cell = data.cells.getCell(xy);
				if (!cellHasBackground(cell)) {
					// add ground
					CellObject cellObject = data.cellObjectManager.createObject(Names.kGround);
					cell.addObject(cellObject);
				}
			}
		}
	}

	private boolean cellHasBackground(Cell cell) {
		for (CellObject cellObject : cell.getAll()) {
			if ((cellObject.getName() == Names.kGround)
					|| cellObject.hasProperty(Names.kPropertyBlock)
					|| cellObject.hasProperty(Names.kPropertyCharger)) {
				return true;
			}
		}
		return false;
	}

	public int size() {
		return data.cells.size();
	}
	
	public Cell getCell(int[] xy) {
		return data.cells.getCell(xy);
	}

	public boolean isAvailable(int[] location) {
		Cell cell = getCell(location);
		boolean enterable = !cell.hasAnyWithProperty(Names.kPropertyBlock);
		boolean noPlayer = cell.getPlayer() == null;
		boolean noMissilePack = !cell.hasAnyWithProperty(Names.kPropertyMissiles);
		boolean noCharger = !cell.hasAnyWithProperty(Names.kPropertyCharger);
		return enterable && noPlayer && noMissilePack && noCharger;
	}

	public boolean isInBounds(int[] xy) {
		return data.cells.isInBounds(xy);
	}
	
	public int[] getAvailableLocationAmortized() {
		return GridMapUtil.getAvailableLocationAmortized(this);
	}

	public void addStateUpdate(int [] location, CellObject added) {
		// Update state we keep track of specific to game type
		if (added.hasProperty(Names.kPropertyCharger)) {
			if (!health && added.hasProperty(Names.kPropertyHealth)) {
				health = true;
			}
			if (!energy && added.hasProperty(Names.kPropertyEnergy)) {
				energy = true;
			}
		}
		if (added.hasProperty(Names.kPropertyMissiles)) {
			missilePacks += 1;
		}
	}

	public void removalStateUpdate(int [] location, CellObject removed) {
		if (removed.hasProperty(Names.kPropertyCharger)) {
			if (health && removed.hasProperty(Names.kPropertyHealth)) {
				health = false;
			}
			if (energy && removed.hasProperty(Names.kPropertyEnergy)) {
				energy = false;
			}
		}
		if (removed.hasProperty(Names.kPropertyMissiles)) {
			missilePacks -= 1;
		}
	}
	
	int missilePacks = 0;	// returns the number of missile packs on the map
	public int numberMissilePacks() {
		return missilePacks;
	}
	
	boolean health = false;	// true if there is a health charger
	public boolean hasHealthCharger() {
		return health;
	}
	
	boolean energy = false;	// true if there is an energy charger
	public boolean hasEnergyCharger() {
		return energy;
	}
		
	private static class MissileData {
		MissileData(Cell cell, CellObject missile) {
			this.cell = cell;
			this.missile = missile;
		}
		Cell cell;
		CellObject missile;
	}
	
	public void updateObjects(TankSoarWorld tsWorld) {
		HashSet<CellObject> copy = new HashSet<CellObject>(data.updatables);
		ArrayList<int []> explosions = new ArrayList<int []>();
		ArrayList<MissileData> newMissiles = new ArrayList<MissileData>();
		for (CellObject cellObject : copy) {
			int [] location = Arrays.copyOf(data.updatablesLocations.get(cellObject), data.updatablesLocations.get(cellObject).length);
			
			if (cellObject.update(location)) {

				// Remove it from the cell
				getCell(location).removeObject(cellObject.getName());

				// Missiles fly, handle that
				if (cellObject.hasProperty(Names.kPropertyMissile)) {
					
					// |*  | * |  *|  <|>  |*  | * |  *|  <|>  |
					//  0    1    2    3    0    1    2    3
					// phase 3 threatens two squares
					// we're in phase 3 when detected in phase 2
	
					// what direction is it going
					Direction missileDir = Direction.parse(cellObject.getProperty(Names.kPropertyDirection));
					
					while (true) {
						int phase = cellObject.getIntProperty(Names.kPropertyFlyPhase);
						
						if (phase == 0) {
							Cell overlapCell = getCell(location);
							overlapCell = overlapCell.neighbors[missileDir.backward().index()];
							overlapCell.forceRedraw();
						}
						
						// move it
						Direction.translate(location, missileDir);
						
						// check destination
						Cell cell = getCell(location);
						
						if (cell.hasAnyWithProperty(Names.kPropertyBlock)) {
							// missile is destroyed
							explosions.add(location);
							break;
						}
						
						Player player = cell.getPlayer();
						
						if (player != null) {
							// missile is destroyed
							tsWorld.missileHit(player, this, location, cellObject, Soar2D.simulation.world.getPlayers());
							explosions.add(location);
							break;
						}
				
						// missile didn't hit anything
						
						// if the missile is not in phase 2, return
						if (phase != 2) {
							newMissiles.add(new MissileData(cell, cellObject));
							break;
						}
						
						// we are in phase 2, call update again, this will move us out of phase 2 to phase 3
						cellObject.update(location);
					}
				}
			}
		}

		for (int[] location : explosions) {
			tsWorld.setExplosion(location);
		}
		for (MissileData data : newMissiles) {
			data.cell.addObject(data.missile);
		}
	}

	public CellObject createObjectByName(String name) {
		return data.cellObjectManager.createObject(name);
	}
}
