package org.msoar.gridmap2d.map;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Queue;
import java.util.Set;

import org.apache.log4j.Logger;
import org.msoar.gridmap2d.Direction;
import org.msoar.gridmap2d.Names;
import org.msoar.gridmap2d.players.CommandInfo;
import org.msoar.gridmap2d.players.RadarCell;
import org.msoar.gridmap2d.players.Tank;
import org.msoar.gridmap2d.players.TankState;
import org.msoar.gridmap2d.world.PlayersManager;
import org.msoar.gridmap2d.world.TankSoarWorld;


public class TankSoarMap implements GridMap, CellObjectObserver {
	private static Logger logger = Logger.getLogger(TankSoarMap.class);

	String mapPath;
	GridMapData data;
	boolean energy;
	int missilePacks;
	boolean health;
	int maxSoundDistance;
	
	public TankSoarMap(String mapPath, int maxSoundDistance) throws Exception {
		this.mapPath = new String(mapPath);
		this.maxSoundDistance = maxSoundDistance;
		
		reset();
	}

	public String getCurrentMapName() {
		return GridMapUtil.getMapName(this.mapPath);
	}

	public void reset() throws Exception {
		energy = false;
		missilePacks = 0;
		health = false;
		data = new GridMapData();
		GridMapUtil.loadFromConfigFile(data, mapPath, this);
		
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
		boolean noMissilePack = !cell.hasObject("missiles");
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
		if (added.getName().equals("missiles")) {
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
		if (removed.getName().equals("missiles")) {
			missilePacks -= 1;
		}
	}
	
	public int numberMissilePacks() {
		return missilePacks;
	}
	
	public boolean hasHealthCharger() {
		return health;
	}
	
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
		Set<CellObject> copy = new HashSet<CellObject>(data.updatables);
		List<int []> explosions = new ArrayList<int []>();
		List<MissileData> newMissiles = new ArrayList<MissileData>();
		for (CellObject cellObject : copy) {
			int [] location = org.msoar.gridmap2d.Arrays.copyOf(data.updatablesLocations.get(cellObject), data.updatablesLocations.get(cellObject).length);
			Cell cell = getCell(location);

			if (cellObject.hasProperty("update.fly-missile")) {
				// Remove it from the cell
				cell.removeObject(cellObject.getName());

				// what direction is it going
				Direction missileDir = Direction.parse(cellObject.getProperty(Names.kPropertyDirection));

				int phase = cellObject.getIntProperty("update.fly-missile", 0);

				while (true) {
					// increment its phase
					phase += 1;
					phase %= 4;
					
					// |*  | * |  *|  <|>  |*  | * |  *|  <|>  |
					//  0    1    2    3    0    1    2    3
					// phase 3 threatens two squares
					// we're in phase 3 when detected in phase 2

					if (phase == 0) {
						Cell overlapCell = getCell(location);
						overlapCell = overlapCell.neighbors[missileDir.backward().index()];
						overlapCell.forceRedraw();
					}
					
					// move it
					Direction.translate(location, missileDir);
					
					logger.trace("Flying missile " + cellObject.getProperty("owner") + "-" + cellObject.getProperty("missile-id") + " entering " + Arrays.toString(location));
					
					// check destination
					cell = getCell(location);
					
					if (cell.hasAnyWithProperty(Names.kPropertyBlock)) {
						// missile is destroyed
						explosions.add(location);
						phase = -1; // don't reset phase on object
						break;
					}
					
					Tank tank = (Tank)cell.getPlayer();
					
					if (tank != null) {
						// missile is destroyed
						tsWorld.missileHit(tank, cellObject);
						explosions.add(location);
						phase = -1; // don't reset phase on object
						break;
					}
			
					// missile didn't hit anything
					
					// if the missile is not in phase 2, done
					if (phase != 2) {
						newMissiles.add(new MissileData(cell, cellObject));
						break;
					}
					
					// we are in phase 2, update again, this will move us out of phase 2 to phase 3
				}
							
				// done flying, update phase unless destroyed
				if (phase >= 0) {
					cellObject.setIntProperty("update.fly-missile", phase);
				}
			}

			GridMapUtil.lingerUpdate(cellObject, cell);
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

	public File getMetadataFile() {
		return data.metadataFile;
	}

	public List<CellObject> getTemplatesWithProperty(String name) {
		return data.cellObjectManager.getTemplatesWithProperty(name);
	}

	public Direction getSoundNear(Tank tank, PlayersManager<Tank> players) {
		if (players.numberOfPlayers() < 2) {
			return Direction.NONE;
		}
		
		// Set all cells unexplored.
		// TODO: never set wall cells unexplored
		int[] xy = new int[2];
		for (xy[0] = 0; xy[0] < data.cells.size(); ++xy[0]) {
			for (xy[1] = 0; xy[1] < data.cells.size(); ++xy[1]) {
				data.cells.getCell(xy).explored = false;
			}
		}
		
		Queue<Cell> searchList = new LinkedList<Cell>();
		{
			Cell start = getCell(players.getLocation(tank));
			start.explored = true;
			start.distance = 0;
			start.parent = null;
			searchList.add(start);
		}
		
		Direction finalDirection = Direction.NONE;
		
		while (searchList.size() > 0) {
			Cell parentCell = searchList.poll();

			if (logger.isTraceEnabled()) {
				logger.trace("Sound: new parent " + parentCell);
			}
			
			// subtract 1 because we add one later (exploring neighbors)
			if (parentCell.distance >= maxSoundDistance) {
				if (logger.isTraceEnabled()) {
					logger.trace("Sound: parent distance " + parentCell.distance + " is too far");
				}
				continue;
			}

			// Explore cell.
			for (Direction exploreDir : Direction.values()) {
				if (exploreDir == Direction.NONE) {
					continue;
				}
				
				Cell neighbor = parentCell.neighbors[exploreDir.index()];
				if (neighbor == null) {
					continue;
				}

				if (neighbor.explored) {
					continue;
				}

				if (logger.isTraceEnabled()) {
					logger.trace("Sound: exploring " + neighbor);
				}
				neighbor.explored = true;
				
				if (neighbor.hasAnyWithProperty(Names.kPropertyBlock)) {
					logger.trace("Sound: blocked");
					continue;
				}
							
				neighbor.distance = parentCell.distance + 1;
				
				if (logger.isTraceEnabled()) {
					logger.trace("Sound: distance " + neighbor.distance);
				}
				
				Tank targetPlayer = (Tank)neighbor.getPlayer(); // TODO: cast
				if ((targetPlayer != null) && recentlyMovedOrRotated(targetPlayer, players)) {
					if (logger.isTraceEnabled()) {
						logger.trace("Sound: found recently moved player " + targetPlayer.getName());
					}
					
					// found a sound! walk home
					// I'm its parent, so see if I'm the top here
					while(parentCell.parent != null) {
						// the new cell becomes me
						neighbor = parentCell;
						
						// I become my parent
						parentCell = parentCell.parent;
					}

					// Find direction to new sound
					boolean found = false;
					for (Direction dir : Direction.values()) {
						if (dir == Direction.NONE) {
							continue;
						}
						if (neighbor == parentCell.neighbors[dir.index()]) {
							finalDirection = dir;
							found = true;
							break;
						}
					}
					
					// shouldn't happen
					if (found) {
						if (logger.isTraceEnabled()) {
							logger.trace("Sound: done, originated from " + finalDirection.id());
						}
					} else {
						// didn't find direction to new sound
						logger.trace("Sound: error: didn't find direction to sound!");
						assert false;
						finalDirection = Direction.NONE;
					}
					
				}
				
				// end condition: this is not Direction.NONE if we found someone
				if (finalDirection != Direction.NONE) {
					break;
				}

				neighbor.parent = parentCell;
				
				// add the new cell to the search list
				searchList.add(neighbor);
			}
			
			// end condition: this is not Direction.NONE if we found someone
			if (finalDirection != Direction.NONE) {
				break;
			}
		}
		return finalDirection;
	}

	private boolean recentlyMovedOrRotated(Tank tank, PlayersManager<Tank> players) {
		CommandInfo command = players.getCommand(tank);
		return command != null && (command.move || command.rotate);
	}

	public CellObject createRandomObjectWithProperty(String property) {
		return data.cellObjectManager.createRandomObjectWithProperty(property);
	}

	public void handleIncoming() {
		// TODO: a couple of optimizations possible here
		// like marking cells that have been checked, depends on direction though
		// probably more work than it is worth as this should only be slow when there are
		// a ton of missiles flying
		
		for (CellObject missile : data.updatables) {
			if (!missile.hasProperty(Names.kPropertyMissile)) {
				continue;
			}
	
			Cell threatenedCell = getCell(data.updatablesLocations.get(missile));
			Direction direction = Direction.parse(missile.getProperty(Names.kPropertyDirection));
			while (true) {
				threatenedCell = threatenedCell.neighbors[direction.index()];
				
				// stops at wall
				if (threatenedCell.hasAnyWithProperty(Names.kPropertyBlock)) {
					break;
				}
				
				Tank tank = (Tank)threatenedCell.getPlayer();
				if (tank != null) {
					TankState state = tank.getState();
					state.setIncoming(direction.backward());
					break;
				}
			}
		}
	}

	public CellObject createRandomObjectWithProperties(String p1, String p2) {
		return data.cellObjectManager.createRandomObjectWithProperties(p1, p2);
	}

	public int getRadar(RadarCell[][] radar, int[] newLocation, Direction facing, int radarPower) {
		if (radarPower == 0) {
			return 0;
		}
		
		assert radar.length == 3;

		int distance = 0;
		
		distance = radarProbe(radar, newLocation, facing, distance, radarPower);
		
		return distance;
	}

	private int radarProbe(RadarCell[][] radar, int [] myLocation, Direction facing, int distance, int maxDistance) {
		assert maxDistance < radar[1].length;
		assert distance >= 0;
		assert distance + 1 < radar[1].length;
		assert distance < maxDistance;
		assert facing != Direction.NONE;
		
		int [] location;
		
		location = org.msoar.gridmap2d.Arrays.copyOf(myLocation, myLocation.length);
		Direction.translate(location, facing.left());
		radar[0][distance] = getRadarCell(location);
		if (radar[0][distance].player != null) {
			Tank tank = (Tank)radar[0][distance].player; // TODO: eliminate cast
			
			if (distance != 0) {
				tank.getState().radarTouch(facing.backward());
			} else {
				tank.getState().radarTouch(facing.right());
			}
		}
		
		location = org.msoar.gridmap2d.Arrays.copyOf(myLocation, myLocation.length);
		Direction.translate(location, facing.right());
		radar[2][distance] = getRadarCell(location);
		if (radar[2][distance].player != null) {
			Tank tank = (Tank)radar[2][distance].player; // TODO: eliminate cast
			
			if (distance != 0) {
				tank.getState().radarTouch(facing.backward());
			} else {
				tank.getState().radarTouch(facing.left());
			}
		}

		distance += 1;

		location = org.msoar.gridmap2d.Arrays.copyOf(myLocation, myLocation.length);
		Direction.translate(location, facing);
		radar[1][distance] = getRadarCell(location);
		if (radar[1][distance].player != null) {
			Tank tank = (Tank)radar[1][distance].player; // TODO: eliminate cast
			tank.getState().radarTouch(facing.backward());
		}

		boolean enterable = radar[1][distance].obstacle == false;
		boolean noPlayer = radar[1][distance].player == null;
		
		if (enterable && noPlayer) {
			CellObject radarWaves = data.cellObjectManager.createObject("radar-" + facing.id());
			radarWaves.setProperty(Names.kPropertyDirection, facing.id());
			logger.trace("Adding " + radarWaves.getName() + " to " + Arrays.toString(location));
			data.cells.getCell(location).addObject(radarWaves);
		}

		if (distance == maxDistance) {
			return distance;
		}
		
		if (enterable && noPlayer) {
			return radarProbe(radar, location, facing, distance, maxDistance);
		}
		
		return distance;
	}

	private RadarCell getRadarCell(int [] location) {
		// TODO: cache these each frame!!
		
		Cell cell;
		RadarCell radarCell;

		cell = getCell(location);
		radarCell = new RadarCell();
		radarCell.player = cell.getPlayer();
		if (!cell.hasAnyWithProperty(Names.kPropertyBlock)) {
			for (CellObject object : cell.getAllWithProperty(Names.kPropertyMiniImage)) {
				if (object.getName().equals(Names.kEnergy)) {
					radarCell.energy = true;
				} else if (object.getName().equals(Names.kHealth)) {
					radarCell.health = true;
				} else if (object.getName().equals(Names.kMissiles)) {
					radarCell.missiles = true;
				} 
			}
		} else {
			radarCell.obstacle = true;
		}
		return radarCell;
	}
	
	public int getBlocked(int[] location) {
		Cell cell;
		int blocked = 0;
		
		cell = getCell(new int [] { location[0]+1, location[1] });
		if (cell.hasAnyWithProperty(Names.kPropertyBlock) || cell.getPlayer() != null) {
			blocked |= Direction.EAST.indicator();
		}
		cell = getCell(new int [] { location[0]-1, location[1] });
		if (cell.hasAnyWithProperty(Names.kPropertyBlock) || cell.getPlayer() != null) {
			blocked |= Direction.WEST.indicator();
		}
		cell = getCell(new int [] { location[0], location[1]+1 });
		if (cell.hasAnyWithProperty(Names.kPropertyBlock) || cell.getPlayer() != null) {
			blocked |= Direction.SOUTH.indicator();
		}
		cell = getCell(new int [] { location[0], location[1]-1 });
		if (cell.hasAnyWithProperty(Names.kPropertyBlock) || cell.getPlayer() != null) {
			blocked |= Direction.NORTH.indicator();
		}
		return blocked;
	}

}
