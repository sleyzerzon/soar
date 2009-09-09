package edu.umich.soar.gridmap2d.map;

import edu.umich.soar.gridmap2d.core.Direction;
import edu.umich.soar.gridmap2d.core.Names;

public class TaxiCommand {
	public static final TaxiCommand NULL = new Builder().build();

	public static class Builder {
		private Direction moveDirection;
		private boolean pickup = false;
		private boolean putdown = false;
		private boolean fillup = false;
		private boolean stopSim = false;

		public Builder move(Direction direction) {
			this.moveDirection = direction;
			return this;
		}
		
		public Builder pickup() {
			this.pickup = true;
			return this;
		}
		
		public Builder putdown() {
			this.putdown = true;
			return this;
		}
		
		public Builder fillup() {
			this.fillup = true;
			return this;
		}
		
		public Builder stopSim() {
			this.stopSim = true;
			return this;
		}
		
		public TaxiCommand build() {
			return new TaxiCommand(this);
		}
	}

	/**
	 * direction to move
	 */
	private final Direction moveDirection;

	/**
	 * 
	 */
	private final boolean pickup;
	
	/**
	 * 
	 */
	private final boolean putdown;
	
	/**
	 * 
	 */
	private final boolean fillup;
	
	/**
	 * 
	 */
	private final boolean stopSim;
	
	private TaxiCommand(Builder builder) {
		this.moveDirection = builder.moveDirection;
		this.pickup = builder.pickup;
		this.putdown = builder.putdown;
		this.fillup = builder.fillup;
		this.stopSim = builder.stopSim;
	}

	public boolean isMove() {
		return moveDirection != null;
	}

	public Direction getMoveDirection() {
		return moveDirection;
	}

	public boolean isPickup() {
		return pickup;
	}

	public boolean isPutdown() {
		return putdown;
	}

	public boolean isFillup() {
		return fillup;
	}

	public boolean isStopSim() {
		return stopSim;
	}
	
	@Override
	public String toString() {
		StringBuilder sb = new StringBuilder();
		Commands.memberAppend(sb, pickup, Names.kPickUpID);
		Commands.memberAppend(sb, putdown, Names.kPutDownID);
		Commands.memberAppend(sb, fillup, Names.kFillUpID);
		Commands.memberAppend(sb, stopSim, Names.kStopSimID);
		return sb.toString();
	}
}
