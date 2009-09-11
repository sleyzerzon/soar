package edu.umich.soar.gridmap2d.map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.gridmap2d.core.Direction;
import edu.umich.soar.gridmap2d.core.PlayerColor;
import edu.umich.soar.gridmap2d.core.Simulation;


public class TankState {
	private static Log logger = LogFactory.getLog(TankState.class);

	public static final int RADAR_WIDTH = 3;
	public static final int RADAR_HEIGHT = 15;

	private String name;
	private int missiles;
	private int energy;
	private int health;
	private boolean shieldsUp;
	private boolean radarSwitch;
	private int radarPower;
	private int observedPower;
	private RadarCell[][] radar;
	private int rwaves;
	private int blocked;
	private int incoming;
	private int resurrectFrame = 0;
	private int smellDistance;
	private PlayerColor smellColor;
	private Direction sound;
	private boolean onHealthCharger;
	private boolean onEnergyCharger;
	private int initialMissiles;
	private int initialEnergy;
	private int initialHealth;
	private int maxMissiles;
	private int maxEnergy;
	private int maxHealth;
	private Direction initialFacing;	
	private Direction facing;	// what direction I'm currently facing

	TankState(Simulation sim, String name, Tank.Builder builder) {
		
		this.name = name;
		
		maxMissiles = sim.getConfig().tanksoarConfig().max_missiles;
		if (builder.missiles == null) {
			builder.missiles = Integer.valueOf(maxMissiles);
		}
		missiles = builder.missiles;
		initialMissiles = builder.missiles;
		
		maxEnergy = sim.getConfig().tanksoarConfig().max_energy;
		if (builder.energy == null) {
			builder.energy = Integer.valueOf(maxEnergy);
		}
		energy = builder.energy;
		initialEnergy = builder.energy;
		
		maxHealth = sim.getConfig().tanksoarConfig().max_health;
		if (builder.health == null) {
			builder.health = Integer.valueOf(maxHealth);
		}
		health = builder.health;
		initialHealth = builder.health;
		
		initialFacing = builder.facing;
		if (initialFacing != null) {
			setFacing(initialFacing);
		} else {
			setFacing(Direction.values()[Simulation.random.nextInt(4) + 1]);
		}
	}
	
	void update(TankSoarMap tankSoarMap, int[] location) {
		if (getRadarSwitch()) {
			setObservedPower(tankSoarMap.getRadar(getRadar(), location, getFacing(), getRadarPower()));
		} else {
			clearRadar();
			setObservedPower(0);
		}
		
		setBlocked(tankSoarMap.getBlocked(location));
	}
	
	public void resetSensors() {
		rwaves = 0;
		incoming = 0;
		blocked = 0;
		smellDistance = 0;
		smellColor = null;
		sound = Direction.NONE;
		onHealthCharger = false;
		onEnergyCharger = false;
		
	}

	public int getEnergy() {
		return energy;
	}
	public void setEnergy(int energy) {
		setEnergy(energy, null);
	}
	public void setEnergy(int energy, String comment) {
		// Bring down shields if out of energy

		this.energy = energy;
		if (logger.isDebugEnabled()) {
			if (comment != null) {
				logger.debug(name + " energy set to: " + Integer.toString(energy) + " (" + comment + ")");
			} else {
				logger.debug(name + " energy set to: " + Integer.toString(energy));
			}
		}
	}
	public void adjustEnergy(int delta) {
		adjustEnergy(delta, null);
	}
	public void adjustEnergy(int delta, String comment) {
		// Bring down shields if out of energy

		int previous = energy;
		energy += delta;
		if (energy < 0) {
			energy = 0;
		}
		if (energy > maxEnergy) {
			energy = maxEnergy;
		}
		if (energy == previous) {
			return;
		}
		if (logger.isDebugEnabled()) {
			if (comment != null) {
				logger.debug(name + " energy: " + Integer.toString(previous) + " -> " + Integer.toString(energy) + " (" + comment + ")");
			} else {
				logger.debug(name + " energy: " + Integer.toString(previous) + " -> " + Integer.toString(energy));
			}
		}
	}
	public int getBlocked() {
		return blocked;
	}
	public void setBlocked(int blocked) {
		this.blocked = blocked;
	}
	public int getMissiles() {
		return missiles;
	}
	public void setMissiles(int missiles) {
		setMissiles(missiles, null);
	}
	public void setMissiles(int missiles, String comment) {
		this.missiles = missiles;
		if (logger.isDebugEnabled()) {
			if (comment != null) {
				logger.debug(name + " missiles set to: " + Integer.toString(missiles) + " (" + comment + ")");
			} else {
				logger.debug(name + " missiles set to: " + Integer.toString(missiles));
			}
		}
	}
	public void adjustMissiles(int delta) {
		adjustMissiles(delta, null);
	}
	public void adjustMissiles(int delta, String comment) {
		int previous = missiles;
		missiles += delta;
		if (missiles < 0) {
			logger.warn(name + ": missiles adjusted to negative value");
			missiles = 0;
		}
		if (missiles == previous) {
			return;
		}
		if (logger.isDebugEnabled()) {
			if (comment != null) {
				logger.debug(name + " missiles: " + Integer.toString(previous) + " -> " + Integer.toString(missiles) + " (" + comment + ")");
			} else {
				logger.debug(name + " missiles: " + Integer.toString(previous) + " -> " + Integer.toString(missiles));
			}
		}
	}
	public int getHealth() {
		return health;
	}
	public void setHealth(int health) {
		setHealth(health, null);
	}
	public void setHealth(int health, String comment) {
		this.health = health;
		if (logger.isDebugEnabled()) {
			if (comment != null) {
				logger.debug(name + " health set to: " + Integer.toString(health) + " (" + comment + ")");
			} else {
				logger.debug(name + " health set to: " + Integer.toString(health));
			}
		}
	}
	public void adjustHealth(int delta) {
		adjustHealth(delta, null);
	}
	public void adjustHealth(int delta, String comment) {
		int previous = health;
		health += delta;
		if (health < 0) {
			health = 0;
		}
		if (health > maxHealth) {
			health = maxHealth;
		}
		if (health == previous) {
			return;
		}
		if (logger.isDebugEnabled()) {
			if (comment != null) {
				logger.debug(name + " health: " + Integer.toString(previous) + " -> " + Integer.toString(health) + " (" + comment + ")");
			} else {
				logger.debug(name + " health: " + Integer.toString(previous) + " -> " + Integer.toString(health));
			}
		}
	}
	public boolean getShieldsUp() {
		return shieldsUp;
	}
	public void setShieldsUp(boolean shieldsUp) {
		if (this.shieldsUp == shieldsUp) {
			return;
		}
		if (logger.isDebugEnabled()) {
			logger.debug(name + " shields switched " + (shieldsUp ? "on" : "off"));
		}
		this.shieldsUp = shieldsUp;
	}
	public boolean getRadarSwitch() {
		return radarSwitch;
	}
	public void setRadarSwitch(boolean radarSwitch) {
		if (this.radarSwitch == radarSwitch ) {
			return;
		}
		this.radarSwitch = radarSwitch;
		if (logger.isDebugEnabled()) { 
			logger.debug(name + " radar switched " + (radarSwitch ? "on" : "off"));
		}
	}
	public int getRadarPower() {
		return radarPower;
	}
	public void setRadarPower(int radarPower) {
		if (radarPower < 0) {
			radarPower = 0;
		}
		if (radarPower >= RADAR_HEIGHT) {
			radarPower = RADAR_HEIGHT - 1;
		}
		if (this.radarPower == radarPower) {
			return;
		}
		this.radarPower = radarPower;
		if (logger.isDebugEnabled()) { 
			logger.debug(name + " radar power set to: " + Integer.toString(radarPower));
		}
	}
	public int getObservedPower() {
		return observedPower;
	}
	public void setObservedPower(int observedPower) {
		this.observedPower = observedPower;
	}
	public RadarCell[][] getRadar() {
		return radar;
	}
	public void setRadar(RadarCell[][] radar) {
		this.radar = radar;
	}
	public void clearRadar() {
		radar = new RadarCell[RADAR_WIDTH][RADAR_HEIGHT];
	}
	public int getRwaves() {
		return rwaves;
	}
	public void setRwaves(int rwaves) {
		this.rwaves = rwaves;
	}
	public void radarTouch(Direction fromDirection) {
		rwaves |= fromDirection.indicator();
	}
	public int getIncoming() {
		return incoming;
	}
	public void setIncoming(int incoming) {
		this.incoming = incoming;
	}
	public void setIncoming(Direction fromDirection) {
		incoming |= fromDirection.indicator();
	}
	public int getResurrectFrame() {
		return resurrectFrame;
	}
	public void setResurrectFrame(int resurrectFrame) {
		this.resurrectFrame = resurrectFrame;
	}
	public int getSmellDistance() {
		return smellDistance;
	}
	public void setSmellDistance(int smellDistance) {
		this.smellDistance = smellDistance;
	}
	public PlayerColor getSmellColor() {
		return smellColor;
	}
	public void setSmellColor(PlayerColor smellColor) {
		this.smellColor = smellColor;
	}
	public Direction getSound() {
		return sound;
	}
	public void setSound(Direction sound) {
		this.sound = sound;
	}
	public boolean getOnHealthCharger() {
		return onHealthCharger;
	}
	public void setOnHealthCharger(boolean onHealthCharger) {
		this.onHealthCharger = onHealthCharger;
	}
	public boolean getOnEnergyCharger() {
		return onEnergyCharger;
	}
	public void setOnEnergyCharger(boolean onEnergyCharger) {
		this.onEnergyCharger = onEnergyCharger;
	}

	public Direction getFacing() {
		return facing;
	}
	
	public void setFacing(Direction facing) {
		this.facing = facing;
	}

	public void reset(int worldCount) {
		if (initialMissiles > 0) {
			missiles = initialMissiles;
		} else {
			missiles = maxMissiles;
		}
		if (initialEnergy > 0) {
			health = initialEnergy;
		} else {
			health = maxHealth;
		}
		if (initialHealth > 0) {
			energy = initialHealth;
		} else {
			energy = maxEnergy;
		}
		
		shieldsUp = false;
		radarSwitch = false;
		radarPower = 0;
		resurrectFrame = worldCount;
		clearRadar();
		resetSensors();
		if (initialFacing != null) {
			setFacing(initialFacing);
		} else {
			setFacing(Direction.values()[Simulation.random.nextInt(4) + 1]);
		}
	}

	public void fragged(int worldCount) {
		energy = maxEnergy;
		health = maxHealth;
		missiles = maxMissiles;
		resurrectFrame = worldCount;
		clearRadar();
		resetSensors();
		setFacing(Direction.values()[Simulation.random.nextInt(4) + 1]);
	}
}

