package soar2d.map;

import java.awt.Point;
import java.util.ArrayList;

public class Gateway {

	ArrayList<Integer> locationIDs;
	Point leftExtent;
	Point rightExtent;

	Gateway(Point leftExtent, Point rightExtent) {
		this.leftExtent = new Point(leftExtent);
		this.rightExtent = new Point(rightExtent);
	}
	
	void addLocationID(int id) {
		locationIDs.add(id);
	}
	
	public ArrayList<Integer> getLocationIDs() {
		return locationIDs;
	}
}
