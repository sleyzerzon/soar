package edu.umich.soar.gridmap2d.map;

import edu.umich.soar.gridmap2d.core.Direction;
import edu.umich.soar.gridmap2d.core.Simulation;

public class Tank extends Player {
	public static class Builder {
		// Required parameters
		private final String id;
		private final Simulation sim;
		
		// Optional parameters
		Integer missiles;
		Integer energy;
		Integer health;
		
		public Builder(String id, Simulation sim) {
			this.id = id;
			this.sim = sim;
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
		
		public Tank build() {
			return new Tank(sim, this);
		}
	}
	
	private TankCommander commander;
	private final TankState state;
	private final Simulation sim;
	
	private Tank(Simulation sim, Builder builder) {
		super(sim, builder.id);
		this.sim = sim;

		this.state = new TankState(sim, getName(), builder);

		state.clearRadar(); // creates the radar structure
		reset();
	}
	
	public void setCommander(TankCommander commander) {
		this.commander = commander;
	}
	
	public TankCommand getCommand() {
		TankCommand command;
//		if (commander != null) {
			command = commander.nextCommand();
//		} else {
//			command = Gridmap2D.control.getHumanCommand(this);
//		}
		
		return command;
	}
	
	@Override
	public void reset() {
		super.reset();

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

	public void update(int [] newLocation, TankSoarMap tankSoarMap) {
		super.update(newLocation);
		
		if (state.getRadarSwitch()) {
			state.setObservedPower(tankSoarMap.getRadar(state.getRadar(), newLocation, getFacing(), state.getRadarPower()));
		} else {
			state.clearRadar();
			state.setObservedPower(0);
		}
		
		state.setBlocked(tankSoarMap.getBlocked(newLocation));
		
		if (commander != null) {
			commander.update(tankSoarMap);
		}
}
	
	public void fragged() {
		state.fragged(sim.getWorldCount());
		setFacing(Direction.values()[Simulation.random.nextInt(4) + 1]);
		if (commander != null) {
			commander.fragged();
		}
	}
	
	public void playersChanged(Player[] players) {
		if (commander != null) {
			commander.playersChanged(players);
		}
	}

	public void commit(int[] location) {
		if (commander != null) {
			commander.commit();
		}
	}

	public void shutdownCommander() {
		if (commander != null) {
			commander.shutdown();
		}
	}
}
