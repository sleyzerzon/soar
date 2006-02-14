package eatersMap;

public class Map {
	private int width = 17;
	private int height = 17;
	
	private Cell[][] map = null;
	
	public Map() {
		map = new Cell[width][height];
	}
	
}
