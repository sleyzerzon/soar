package edu.umich.soar.gridmap2d.map;

import java.util.Arrays;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.gridmap2d.core.PlayerColor;
import edu.umich.soar.gridmap2d.core.Simulation;


public class Player {
	private static Log logger = LogFactory.getLog(Player.class);
	
	private final String name;	// player name
	private Integer initialPoints;
	private int points;	// current point count
	private boolean pointsChanged;
	private int pointsDelta;
	private PlayerColor color;	// valid color string
	private int[] location;
	protected boolean moved;
	private boolean fragged;

	public Player(Simulation sim, String name, PlayerColor color) {
		if (name == null) {
			throw new NullPointerException("name is null");
		}
		this.name = name;
		
		if (color == null) {
			throw new NullPointerException("color is null");
		}
		this.color = color;
		
		this.reset();
	}
	
	public void setInitialPoints(Integer initialPoints) {
		this.initialPoints = initialPoints;
		defaultPoints();
	}
	
	private void defaultPoints() {
		points = initialPoints == null ? 0 : initialPoints;
	}
	
	public void reset() {
		location = new int[] { -1, -1 };
		
		defaultPoints();

		pointsChanged = true;
		pointsDelta = 0;
		fragged = false;
	}

	public String getName() {
		return this.name;
	}
	
	public boolean pointsChanged() {
		return pointsChanged;
	}
	
	public int getPointsDelta() {
		return pointsDelta;
	}
	
	public void resetPointsChanged() {
		// this state needs to be saved by soar side
		pointsChanged = false;
		pointsDelta = 0;
	}
		
	public int getPoints() {
		return points;
	}

	public void setPoints(int points, String comment) {
		pointsChanged = true;
		pointsDelta = points - this.points;
		
		this.points = points;
		if (comment != null) {
			logger.info(this.name + " score set to: " + Integer.toString(this.points) + " (" + comment + ")");
		} else {
			logger.info(this.name + " score set to: " + Integer.toString(this.points));
		}
	}

	public void adjustPoints(int delta, String comment) {
		pointsChanged = (delta != 0);
		pointsDelta = delta;
		
		int previous = this.points;
		this.points += delta;
		if (comment != null) {
			logger.info(this.name + " score: " + Integer.toString(previous) + " -> " + Integer.toString(this.points) + " (" + comment + ")");
		} else {
			logger.info(this.name + " score: " + Integer.toString(previous) + " -> " + Integer.toString(this.points));
		}
	}
	
	public PlayerColor getColor() {
		return this.color;
	}

	public void setColor(PlayerColor color) {
		this.color = color;
	}
	
	@Override
	public int hashCode() {
		return name.hashCode();
	}

	@Override
	public boolean equals(Object o) {
		if (this == o) {
			return true;
		}
		
		if (!(o instanceof Player)) {
			return false;
		}
		Player player = (Player)o;
		
		return name.equals(player.name);
	}

	@Override
	public String toString() {
		return getName();
	}

	void setLocation(int[] newLocation) {
		moved = !Arrays.equals(newLocation, this.location);
		if (moved) {
			this.location = Arrays.copyOf(newLocation, newLocation.length);
		}
	}
	
	public int[] getLocation() {
		return Arrays.copyOf(location, location.length);
	}
	
	public boolean getMoved() {
		return moved;
	}
	
	public void setFragged(boolean fragged) {
		this.fragged = fragged;
		this.moved = true;
	}
	
	public boolean getFragged() {
		return fragged;
	}
}
