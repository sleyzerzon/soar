package edu.umich.soar.gridmap2d.map;

public class Points {
	private int points;
	
	public int getPoints() {
		return points;
	}
	
	void setPoints(int points) {
		this.points = points;
	}
	
	void adjustPoints(int delta) {
		this.points += delta;
	}
	
	void reset() {
		this.points = 0;
	}
}
