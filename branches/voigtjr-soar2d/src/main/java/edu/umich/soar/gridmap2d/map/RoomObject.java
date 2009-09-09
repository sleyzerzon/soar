package edu.umich.soar.gridmap2d.map;

import java.awt.Color;

import edu.umich.soar.gridmap2d.core.Names;
import lcmtypes.pose_t;

public class RoomObject {
	private pose_t pose;
	private int area;
	private final int id;
	private final CellObject object;
	private Color color;

	public RoomObject(CellObject object, int id) {
		this.object = object;
		this.id = id;

		// TODO: make colors enum, use valueOf
		String color = object.getProperty("color");
		if (color.equals("red")) {
			this.color = Color.red;
		} else if (color.equals("green")) {
			this.color = Color.green;
		} else if (color.equals("gray")) {
			this.color = Color.darkGray;
		}
	}

	void update(GridMapCells cells) {
		Cell container = object.getCell();
		int[] location = container.getLocation();

		pose_t pose = new pose_t();
		pose.pos[0] = location[0] * RoomWorld.CELL_SIZE;
		pose.pos[1] = location[1] * RoomWorld.CELL_SIZE;
		pose.pos[0] += RoomWorld.CELL_SIZE / 2.0;
		pose.pos[1] += RoomWorld.CELL_SIZE / 2.0;
		setPose(pose);
		setArea(container.getFirstObjectWithProperty(Names.kRoomID)
				.getProperty(Names.kPropertyNumber, -1, Integer.class));
	}

	public int getArea() {
		return area;
	}

	public void setArea(int area) {
		this.area = area;
	}

	public int getId() {
		return id;
	}

	public void setPose(pose_t pose) {
		if (pose == null) {
			this.pose = null;
			return;
		}
		this.pose = pose.copy();
	}

	public pose_t getPose() {
		if (pose == null) {
			return null;
		}
		return pose.copy();
	}

	public CellObject getCellObject() {
		return object;
	}

	public Color getColor() {
		return color;
	}
}
