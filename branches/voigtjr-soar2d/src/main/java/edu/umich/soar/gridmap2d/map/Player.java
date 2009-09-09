package edu.umich.soar.gridmap2d.map;

import java.util.Arrays;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.gridmap2d.config.PlayerConfig;
import edu.umich.soar.gridmap2d.core.Direction;
import edu.umich.soar.gridmap2d.core.PlayerColor;
import edu.umich.soar.gridmap2d.core.Simulation;


public class Player {
	private static Log logger = LogFactory.getLog(Player.class);
	
	private String playerID;
	private PlayerConfig playerConfig;
	private final String name;	// player name
	private int points;	// current point count
	private boolean pointsChanged;
	private int pointsDelta;
	private Direction facing;	// what direction I'm currently facing
	private PlayerColor color;	// valid color string
	private int[] location;
	protected boolean moved;
	private boolean fragged;

	public Player(Simulation sim, String playerID) {
		this.playerID = playerID;
		this.playerConfig = sim.getConfig().playerConfigs().get(playerID);
		if (!playerConfig.hasPoints()) {
			this.playerConfig.points = sim.getConfig().generalConfig().default_points;
		}
		
		assert playerConfig.name != null;
		this.name = playerConfig.name;
		
		assert playerConfig.color != null;
		this.color = playerConfig.color;

		this.reset();
	}
	
	public void reset() {
		location = new int[] { -1, -1 };
		
		if (playerConfig.facing != null) {
			this.setFacing(Direction.parse(playerConfig.facing));
		} else {
			this.setFacing(Direction.values()[Simulation.random.nextInt(4) + 1]);
		}
		
		this.points = playerConfig.points;

		pointsChanged = true;
		pointsDelta = 0;
		fragged = false;
	}

	public String getName() {
		return this.name;
	}
	
	public Direction getFacing() {
		return facing;
	}
	
	public void setFacing(Direction facing) {
		this.facing = facing;
	}
	
	public boolean pointsChanged() {
		return pointsChanged;
	}
	
	public int getPointsDelta() {
		return pointsDelta;
	}
	
	public void resetPointsChanged() {
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
		PlayerColor previous = this.color;
		this.color = color;
		logger.warn(this.name + " (" + previous + ") color changed to: " + this.color.toString().toLowerCase());
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

	public String getID() {
		return playerID;
	}

	protected void update(int[] newLocation) {
		moved = (newLocation[0] != this.location[0]) || (newLocation[1] != this.location[1]);
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
