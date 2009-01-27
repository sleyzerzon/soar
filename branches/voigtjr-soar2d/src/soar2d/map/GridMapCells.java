package soar2d.map;

class GridMapCells {
	private Cell[][] cells;
	
	GridMapCells(int size) {
		assert size > 0;
		
		cells = new Cell[size][];
		for(int x = 0; x < size; ++x) {
			cells[x] = new Cell[size];
		}
	}

	void setCell(int[] xy, Cell cell) {
		cells[xy[0]][xy[1]] = cell;
	}
	
	Cell getCell(int [] xy) {
		assert xy != null;
		
		return cells[xy[0]][xy[1]];
	}
	
	int size() {
		return cells.length;
	}

	boolean isInBounds(int[] location) {
		assert location != null;
		
		return (location[0] >= 0) && (location[1] >= 0) && (location[0] < size()) && (location[1] < size());
	}

}
