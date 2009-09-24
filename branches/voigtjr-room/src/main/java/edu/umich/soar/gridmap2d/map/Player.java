package edu.umich.soar.gridmap2d.map;

import edu.umich.soar.gridmap2d.core.PlayerColor;

public abstract class Player {
	private final String name;	
	private final PlayerColor color;	

	public Player(String name, PlayerColor color) {
		if (name == null) {
			throw new NullPointerException("name is null");
		}
		this.name = name;
		
		if (color == null) {
			throw new NullPointerException("color is null");
		}
		this.color = color;
	}
	
	public PlayerColor getColor() {
		return this.color;
	}

	public String getName() {
		return this.name;
	}
	
	abstract void reset();
	abstract void shutdown();
	
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
}
