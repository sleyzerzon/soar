package edu.umich.soar.gridmap2d.map;

import edu.umich.soar.gridmap2d.core.Direction;
import edu.umich.soar.gridmap2d.core.Names;

public class EaterCommand {
	public static final EaterCommand NULL = new Builder().build();
	
	public static class Builder {
		private Direction moveDirection;
		private boolean open = false;
		private boolean jump = false;
		private boolean dontEat = false;
		private boolean stopSim = false;

		public Builder stopSim() {
			this.stopSim = true;
			return this;
		}

		public Builder move(Direction direction) {
			this.moveDirection = direction;
			return this;
		}

		public Builder open() {
			this.open = true;
			return this;
		}

		public Builder jump() {
			this.jump = true;
			return this;
		}

		public Builder dontEat() {
			this.dontEat = true;
			return this;
		}

		public EaterCommand build() {
			return new EaterCommand(this);
		}
	}

	/**
	 * direction to move
	 */
	private final Direction moveDirection;
	
	/**
	 * open the box on the current cell
	 */
	private final boolean open;
	
	/**
	 * jump if we move
	 */
	private final boolean jump;
	
	/**
	 * don't eat food
	 */
	private final boolean dontEat;

	/**
	 * stop the simulation by command
	 */
	private final boolean stopSim;
	
	private EaterCommand(Builder builder) {
		this.stopSim = builder.stopSim;
		this.moveDirection = builder.moveDirection;
		this.open = builder.open;
		this.jump = builder.jump;
		this.dontEat = builder.dontEat;
	}

	public boolean isStopSim() {
		return stopSim;
	}

	public boolean isMove() {
		return moveDirection != null;
	}

	public Direction getMoveDirection() {
		return moveDirection;
	}

	public boolean isOpen() {
		return open;
	}

	public boolean isJump() {
		return jump;
	}

	public boolean isDontEat() {
		return dontEat;
	}

	@Override
	public String toString() {
		StringBuilder sb = new StringBuilder();
		Commands.memberAppend(sb, isJump(), Names.kJumpID);
		if (isMove()) {
			Commands.memberAppend(sb, isMove(), Names.kMoveID, getMoveDirection().id());
		}
		Commands.memberAppend(sb, isDontEat(), Names.kDontEatID);
		Commands.memberAppend(sb, isOpen(), Names.kOpenID);
		Commands.memberAppend(sb, isStopSim(), Names.kStopSimID);
		return sb.toString();
	}
}
