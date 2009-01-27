package soar2d.map;

public interface GridMap {
	public int size();
	public Cell getCell(int[] xy);
	public boolean isAvailable(int[] location);
	public int[] getAvailableLocationAmortized();
	public boolean isInBounds(int[] xy);
	public CellObject createObjectByName(String name);
}

/*

public boolean mapUsesOpenCode() {
	return rewardInfoObject.hasProperty(Names.kPropertyOpenCode);
}

public class BookObjectInfo {
	public CellObject object;
	public int [] location;
	public double [] floatLocation;
	public int area = -1;
}

public CellObject createObjectByName(String name) {
	return cellObjectManager.createObject(name);
}

public CellObject createRandomObjectWithProperty(String property) {
	return cellObjectManager.createRandomObjectWithProperty(property);
}

public CellObject createRandomObjectWithProperties(String property1, String property2) {
	return cellObjectManager.createRandomObjectWithProperties(property1, property2);
}

public void updateObjects(TankSoarWorld tsWorld) {
	HashSet<CellObject> copy = new HashSet<CellObject>(updatables);
	for (CellObject cellObject : copy) {
		int [] location = updatablesLocations.get(cellObject);
		
		if (cellObject.update(location)) {
			removalStateUpdate(getCell(location).removeObject(cellObject.getName()));
		}
	}
}

public void setPlayer(int [] location, Player player) {
	getCell(location).setPlayer(player);
}

public int pointsCount(int [] location) {
	Cell cell = getCell(location);
	ArrayList<CellObject> list = cell.getAllWithProperty(Names.kPropertyEdible);
	if (list == null) {
		return 0;
	}
	int count = 0;
	for (CellObject object : list) {
		count += object.getIntProperty(Names.kPropertyPoints);
	}
	return count;
}

public abstract boolean isAvailable(int [] location);

public boolean exitable(int [] location, Direction direction) {
	ArrayList<CellObject> wallList = getAllWithProperty(location, "block");
	if (wallList != null) {
		for (CellObject object : wallList) {
			if (direction == Direction.parse(object.getProperty("direction"))) {
				return false;
			}
		}
	}
	return true;
}

public CellObject removeObject(int [] location, String objectName) {
	CellObject object = getCell(location).removeObject(objectName);
	if (object == null) return null;
	removalStateUpdate(object);
	return object;
}

public Player getPlayer(int [] location) {
	return getCell(location).getPlayer();
}

public boolean hasObject(int [] location, String name) {
	return getCell(location).hasObject(name);
}

public CellObject getObject(int [] location, String name) {
	return getCell(location).getObject(name);
}

public void handleIncoming() {
	// TODO: a couple of optimizations possible here
	// like marking cells that have been checked, depends on direction though
	// probably more work than it is worth as this should only be slow when there are
	// a ton of missiles flying
	
	for (CellObject missile : updatables) {
		if (!missile.hasProperty(Names.kPropertyMissile)) {
			continue;
		}

		Cell threatenedCell = getCell(updatablesLocations.get(missile));
		Direction direction = Direction.parse(missile.getProperty(Names.kPropertyDirection));
		while (true) {
			threatenedCell = threatenedCell.neighbors[direction.index()];
			
			// stops at wall
			if (threatenedCell.hasAnyWithProperty(Names.kPropertyBlock)) {
				break;
			}
			
			Player player = threatenedCell.getPlayer();
			if (player != null) {
				player.setIncoming(direction.backward());
				break;
			}
		}
	}
}

void addStateUpdate(int [] location, CellObject added) {
	assert added != null;
}

void removalStateUpdate(CellObject removed) {
	if (removed == null) {
		return;
	}
}

public ArrayList<CellObject> getAllWithProperty(int [] location, String name) {
	return getCell(location).getAllWithProperty(name);
}

public boolean hasAnyWithProperty(int [] location, String name) {
	return getCell(location).hasAnyWithProperty(name);
}

public void removeAll(int [] location) {
	ArrayList<CellObject> removed = getCell(location).removeAll();
	for (CellObject object : removed) {
		removalStateUpdate(object);
	}
}

public void shutdown() {
	mapCells = null;
	cellObjectManager = null;
	size = 0;
}

public int getRadar(RadarCell[][] radar, int [] location, Direction facing, int radarPower) {
	if (radarPower == 0) {
		return 0;
	}
	
	assert radar.length == 3;

	int distance = 0;
	
	distance = radarProbe(radar, location, facing, distance, radarPower);
	
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

private int radarProbe(RadarCell[][] radar, int [] myLocation, Direction facing, int distance, int maxDistance) {
	assert maxDistance < radar[1].length;
	assert distance >= 0;
	assert distance + 1 < radar[1].length;
	assert distance < maxDistance;
	assert facing != Direction.NONE;
	
	int [] location;
	
	location = Arrays.copyOf(myLocation, myLocation.length);
	Direction.translate(location, facing.left());
	radar[0][distance] = getRadarCell(location);
	if (radar[0][distance].player != null) {
		if (distance != 0) {
			radar[0][distance].player.radarTouch(facing.backward());
		} else {
			radar[0][distance].player.radarTouch(facing.right());
		}
	}
	
	location = Arrays.copyOf(myLocation, myLocation.length);
	Direction.translate(location, facing.right());
	radar[2][distance] = getRadarCell(location);
	if (radar[2][distance].player != null) {
		if (distance != 0) {
			radar[2][distance].player.radarTouch(facing.backward());
		} else {
			radar[2][distance].player.radarTouch(facing.left());
		}
	}

	distance += 1;

	location = Arrays.copyOf(myLocation, myLocation.length);
	Direction.translate(location, facing);
	radar[1][distance] = getRadarCell(location);
	if (radar[1][distance].player != null) {
		radar[1][distance].player.radarTouch(facing.backward());
	}

	boolean enterable = radar[1][distance].obstacle == false;
	boolean noPlayer = radar[1][distance].player == null;
	
	if (enterable && noPlayer) {
		CellObject radarWaves = new CellObject("radar-" + facing);
		radarWaves.addProperty(Names.kPropertyRadarWaves, "true");
		radarWaves.addProperty(Names.kPropertyDirection, facing.id());
		radarWaves.addProperty(Names.kPropertyLinger, "1");
		radarWaves.setLingerUpdate(true);
		//System.out.println("Adding radar waves to " + location);
		addObjectToCell(location, radarWaves);
	}

	if (distance == maxDistance) {
		return distance;
	}
	
	if (enterable && noPlayer) {
		return radarProbe(radar, location, facing, distance, maxDistance);
	}
	
	return distance;
}

public int getBlocked(int [] location) {
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

public boolean isBlocked(int [] location) {
	Cell cell = getCell(location);
	return cell.hasAnyWithProperty(Names.kPropertyBlock) || cell.getPlayer() != null;
}

public Direction getSoundNear(int [] location) {
	if (Soar2D.simulation.world.getPlayers().numberOfPlayers() < 2) {
		return Direction.NONE;
	}
	
	// Set all cells unexplored.
	for(Cell[] cols : mapCells) {
		for (Cell cell : cols) {
			cell.explored = false;
		}
	}
	
	LinkedList<Cell> searchList = new LinkedList<Cell>();
	{
		Cell start = getCell(location);
		start.explored = true;
		start.distance = 0;
		start.parent = null;
		searchList.addLast(start);
	}
	
	Direction finalDirection = Direction.NONE;
	
	while (searchList.size() > 0) {
		Cell parentCell = searchList.getFirst();
		searchList.removeFirst();

		if (logger.isTraceEnabled()) {
			logger.trace("Sound: new parent " + parentCell);
		}
		
		// subtract 1 because we add one later (exploring neighbors)
		if (parentCell.distance >= Soar2D.config.tanksoarConfig().max_sound_distance) {
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
			
			Player targetPlayer = neighbor.getPlayer();
			if ((targetPlayer != null) && Soar2D.simulation.world.recentlyMovedOrRotated(targetPlayer)) {
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
			searchList.addLast(neighbor);
		}
		
		// end condition: this is not Direction.NONE if we found someone
		if (finalDirection != Direction.NONE) {
			break;
		}
	}
	return finalDirection;
}	
}
*/