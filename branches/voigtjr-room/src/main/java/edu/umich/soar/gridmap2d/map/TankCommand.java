package edu.umich.soar.gridmap2d.map;

import edu.umich.soar.gridmap2d.core.Direction;
import edu.umich.soar.gridmap2d.core.Names;

public class TankCommand {
	public static final TankCommand NULL = new Builder().build();

	public static class Builder {
		private Direction moveDirection;
		private String rotateDirection;
		private boolean fire = false;
		private Boolean radarSwitch;
		private Integer radarPower;
		private Boolean shields;
		private boolean stopSim = false;
		
		public Builder() {
			
		}
		
		private Builder(TankCommand command) {
			this.moveDirection = command.moveDirection;
			this.rotateDirection = command.rotateDirection;
			this.fire = command.fire;
			this.radarSwitch = command.radarSwitch;
			this.radarPower = command.radarPower;
			this.shields = command.shields;
			this.stopSim = command.stopSim;
		}

		public Builder move(Direction direction) {
			this.moveDirection = direction;
			return this;
		}
		
		public Builder rotate(String direction) {
			// rotation cancels a move
			if (moveDirection != null) {
				moveDirection = null;
			}
			this.rotateDirection = direction;
			return this;
		}
		
		public Builder fire() {
			this.fire = true;
			return this;
		}

		public Builder radar(boolean value) {
			this.radarSwitch = Boolean.valueOf(value);
			return this;
		}

		public Builder radarPower(int value) {
			this.radarPower = Integer.valueOf(value);
			return this;
		}

		public Builder shields(boolean value) {
			this.shields = Boolean.valueOf(value);
			return this;
		}

		public Builder stopSim() {
			this.stopSim = true;
			return this;
		}

		public TankCommand build() {
			return new TankCommand(this);
		}
	}

	/**
	 * direction to move
	 */
	private final Direction moveDirection;
	/**
	 * direction to rotate
	 */
	private final String rotateDirection;
	/**
	 * fire the tank cannon
	 */
	private final boolean fire;
	/**
	 * status to change radar to
	 */
	private final Boolean radarSwitch;
	/**
	 * setting to change radar power to
	 */
	private final Integer radarPower;
	/**
	 * status to change shields to
	 */
	private final Boolean shields;
	/**
	 * stop the simulation
	 */
	private final boolean stopSim;

	public static TankCommand cancelMove(TankCommand command) {
		Builder builder = new Builder(command);
		builder.moveDirection = null;
		return new TankCommand(builder);
	}
	
	private TankCommand(Builder builder) {
		this.moveDirection = builder.moveDirection;
		this.rotateDirection = builder.rotateDirection;
		this.fire = builder.fire;
		this.radarSwitch = builder.radarSwitch;
		this.radarPower = builder.radarPower;
		this.shields = builder.shields;
		this.stopSim = builder.stopSim;
	}
	
	public boolean isMove() {
		return moveDirection != null;
	}
	
	public Direction getMoveDirection() {
		return moveDirection;
	}
	
	public boolean isRotate() {
		return rotateDirection != null;
	}
	
	public String getRotateDirection() {
		return rotateDirection;
	}
	
	public boolean isFire() {
		return fire;
	}
	
	public boolean isRadarSwitch() {
		return radarSwitch != null;
	}
	
	public boolean getRadarSwitch() {
		return radarSwitch;
	}
	
	public boolean isRadarPower() {
		return radarPower != null;
	}
	
	public int getRadarPower() {
		return radarPower;
	}
	
	public boolean isShields() {
		return shields != null;
	}
	
	public boolean getShields() {
		return shields;
	}
	
	public boolean isStopSim() {
		return stopSim;
	}
	
	@Override
	public String toString() {
		StringBuilder sb = new StringBuilder();
		if (isMove()) {
			Commands.memberAppend(sb, isMove(), Names.kMoveID, getMoveDirection().id());
		}
		Commands.memberAppend(sb, isRotate(), Names.kRotateID, getRotateDirection());
		Commands.memberAppend(sb, isFire(), Names.kFireID);
		if (isRadarSwitch()) {
			Commands.memberAppend(sb, isRadarSwitch(), Names.kRadarID, getRadarSwitch() ? "on" : "off");
		}
		if (isRadarPower()) {
			Commands.memberAppend(sb, isRadarPower(), Names.kRadarPowerID, Integer.toString(getRadarPower()));
		}
		if (isShields()) {
			Commands.memberAppend(sb, isShields(), Names.kShieldsID, getShields() ? "on" : "off");
		}
		Commands.memberAppend(sb, isStopSim(), Names.kStopSimID);
		return sb.toString();
	}
}
