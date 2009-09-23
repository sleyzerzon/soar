package edu.umich.soar.gridmap2d.map;

import java.util.List;

import edu.umich.soar.gridmap2d.core.Direction;
import edu.umich.soar.gridmap2d.core.PlayerColor;
import edu.umich.soar.gridmap2d.core.Simulation;

public class Tank extends Player {
	public static class Builder {
		// Required parameters
		private final Simulation sim;
		private final String name;
		private final PlayerColor color;
		
		// Optional parameters
		Integer missiles;
		Integer energy;
		Integer health;
		Direction facing;
		
		public Builder(Simulation sim, String name, PlayerColor color) {
			this.sim = sim;
			this.name = name;
			this.color = color;
		}
		
		public Builder missiles(int missiles) {
			this.missiles = Integer.valueOf(missiles);
			return this;
		}
		
		public Builder energy(int energy) {
			this.energy = Integer.valueOf(energy);
			return this;
		}
		
		public Builder health(int health) {
			this.health = Integer.valueOf(health);
			return this;
		}
		
		public Builder facing(Direction facing) {
			this.facing = facing;
			return this;
		}
		
		public Tank build() {
			return new Tank(sim, this);
		}
	}
	
	private TankCommander commander;
	private final TankState state;
	private final Simulation sim;
	private Tank(Simulation sim, Builder builder) {
	
		super(sim, builder.name, builder.color);
		this.sim = sim;

		this.state = new TankState(sim, getName(), builder);

		state.clearRadar(); // creates the radar structure
		reset();
	}
	
	public void setCommander(TankCommander commander) {
		this.commander = commander;
	}
	
	public TankCommand getCommand() {
		return commander.nextCommand();
	}
	
	@Override
	public void reset() {
		if (state != null) {
			state.reset(sim.getWorldCount());
		}

		if (commander != null) {
			commander.reset();
		}
	}
	
	public TankState getState() {
		return state;
	}

	public void fragged() {
		state.fragged(sim.getWorldCount());
		if (commander != null) {
			commander.fragged();
		}
	}
	
	public void playersChanged(List<? extends Player> players) {
		if (commander != null) {
			commander.playersChanged(players);
		}
	}

	public void shutdownCommander() {
		if (commander != null) {
			commander.shutdown();
		}
	}
}
