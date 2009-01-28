package soar2d.players;

import org.apache.log4j.Logger;

import soar2d.Direction;
import soar2d.Simulation;
import soar2d.Soar2D;
import soar2d.map.TankSoarMap;

public class Tank extends Player {
	private static Logger logger = Logger.getLogger(Tank.class);

	private CommandInfo command;
	private TankCommander commander;
	private int initialMissiles;
	private int initialEnergy;
	private int initialHealth;
	private int missiles;
	private int energy;
	private int health;

	public Tank(String playerId, int initialMissiles, int initialEnergy, int initialHealth) throws Exception {
		super(playerId);
		this.initialMissiles = initialMissiles;
		this.initialEnergy = initialEnergy;
		this.initialHealth = initialHealth;

		clearRadar(); // creates the radar structure
		reset();
	}
	
	public void setCommander(TankCommander commander) {
		this.commander = commander;
	}
	
	public CommandInfo getCommand() throws Exception {
		if (commander != null) {
			command = commander.getCommand();
		} else {
			command = Soar2D.simulation.getHumanCommand(this);
		}
		
		// the facing depends on the move
		if (command.move) { 
			super.setFacing(command.moveDirection);
		}

		return command;
	}
	
	@Override
	public void reset() throws Exception {
		super.reset();

		if (initialMissiles > 0) {
			this.missiles = initialMissiles;
		} else {
			this.missiles = Soar2D.config.tanksoarConfig().default_missiles;
		}
		if (initialEnergy > 0) {
			this.health = initialEnergy;
		} else {
			this.health = Soar2D.config.tanksoarConfig().default_health;
		}
		if (initialHealth > 0) {
			this.energy = initialHealth;
		} else {
			this.energy = Soar2D.config.tanksoarConfig().default_energy;
		}
		
		shieldsUp = false;
		radarSwitch = false;
		radarPower = 0;
		resurrectFrame = Soar2D.simulation.getWorldCount();
		clearRadar();
		resetSensors();

		if (commander != null) {
			commander.reset();
		}
	}
	
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
	private String smellColor;
	
	private CommandInfo move;
	private Direction sound;
	private boolean onHealthCharger;
	private boolean onEnergyCharger;

	public boolean getRadarSwitch() {
		return radarSwitch;
	}
	
	public RadarCell[][] getRadar() {
		return radar;
	}
	
	public int getObservedDistance() {
		return observedPower;
	}
	
	public void setRadarSwitch(boolean setting) {
		if (radarSwitch == setting ) {
			return;
		}
		radarSwitch = setting;
		if (logger.isDebugEnabled()) { 
			logger.debug(getName() + " radar switched " + (setting ? "on" : "off"));
		}
	}
	
	public int getRadarPower() {
		return radarPower;
	}
	
	public void setRadarPower(int setting) {
		if (setting < 0) {
			setting = 0;
		}
		if (setting >= Soar2D.config.tanksoarConfig().radar_height) {
			setting = Soar2D.config.tanksoarConfig().radar_height - 1;
		}
		if (radarPower == setting) {
			return;
		}
		radarPower = setting;
		if (logger.isDebugEnabled()) { 
			logger.debug(getName() + " radar power set to: " + Integer.toString(this.radarPower));
		}
	}
	
	public int getMissiles() {
		return missiles;
	}
	/**
	 * @param missiles the new missile count
	 * @param comment why the change happened, keep it very brief
	 * 
	 * set the missiles to a specific value, the comment goes in the log
	 */
	public void setMissiles(int missiles, String comment) {
		this.missiles = missiles;
		if (logger.isDebugEnabled()) {
			if (comment != null) {
				logger.debug(getName() + " missiles set to: " + Integer.toString(this.missiles) + " (" + comment + ")");
			} else {
				logger.debug(getName() + " missiles set to: " + Integer.toString(this.missiles));
			}
		}
	}
	/**
	 * @param delta the change in missiles
	 * @param comment why the change happened, keep it very brief
	 * 
	 * this is a handy function for changing missiles. puts a message in the log and 
	 * why the change happened
	 */
	public void adjustMissiles(int delta, String comment) {
		int previous = this.missiles;
		this.missiles += delta;
		if (missiles < 0) {
			logger.warn(getName() + ": missiles adjusted to negative value");
			missiles = 0;
		}
		if (missiles == previous) {
			return;
		}
		if (logger.isDebugEnabled()) {
			if (comment != null) {
				logger.debug(getName() + " missiles: " + Integer.toString(previous) + " -> " + Integer.toString(this.missiles) + " (" + comment + ")");
			} else {
				logger.debug(getName() + " missiles: " + Integer.toString(previous) + " -> " + Integer.toString(this.missiles));
			}
		}
	}

	public int getEnergy() {
		return energy;
	}
	/**
	 * @param energy the new energy count
	 * @param comment why the change happened, keep it very brief
	 * 
	 * set the energy to a specific value, the comment goes in the log
	 */
	public void setEnergy(int energy, String comment) {
		// Bring down shields if out of energy

		this.energy = energy;
		if (logger.isDebugEnabled()) {
			if (comment != null) {
				logger.debug(getName() + " energy set to: " + Integer.toString(this.energy) + " (" + comment + ")");
			} else {
				logger.debug(getName() + " energy set to: " + Integer.toString(this.energy));
			}
		}
	}
	/**
	 * @param delta the change in energy
	 * @param comment why the change happened, keep it very brief
	 * 
	 * this is a handy function for changing energy. puts a message in the log and 
	 * why the change happened
	 */
	public void adjustEnergy(int delta, String comment) {
		// Bring down shields if out of energy

		int previous = this.energy;
		this.energy += delta;
		if (energy < 0) {
			energy = 0;
		}
		if (energy > Soar2D.config.tanksoarConfig().default_energy) {
			energy = Soar2D.config.tanksoarConfig().default_energy;
		}
		if (energy == previous) {
			return;
		}
		if (logger.isDebugEnabled()) {
			if (comment != null) {
				logger.debug(getName() + " energy: " + Integer.toString(previous) + " -> " + Integer.toString(this.energy) + " (" + comment + ")");
			} else {
				logger.debug(getName() + " energy: " + Integer.toString(previous) + " -> " + Integer.toString(this.energy));
			}
		}
	}

	public int getHealth() {
		return health;
	}
	/**
	 * @param health the new missile count
	 * @param comment why the change happened, keep it very brief
	 * 
	 * set the health to a specific value, the comment goes in the log
	 */
	public void setHealth(int health, String comment) {
		this.health = health;
		if (logger.isDebugEnabled()) {
			if (comment != null) {
				logger.debug(getName() + " health set to: " + Integer.toString(this.health) + " (" + comment + ")");
			} else {
				logger.debug(getName() + " health set to: " + Integer.toString(this.health));
			}
		}
	}
	/**
	 * @param delta the change in health
	 * @param comment why the change happened, keep it very brief
	 * 
	 * this is a handy function for changing health. puts a message in the log and 
	 * why the change happened
	 */
	public void adjustHealth(int delta, String comment) {
		int previous = this.health;
		this.health += delta;
		if (health < 0) {
			health = 0;
		}
		if (health > Soar2D.config.tanksoarConfig().default_health) {
			health = Soar2D.config.tanksoarConfig().default_health;
		}
		if (health == previous) {
			return;
		}
		if (logger.isDebugEnabled()) {
			if (comment != null) {
				logger.debug(getName() + " health: " + Integer.toString(previous) + " -> " + Integer.toString(this.health) + " (" + comment + ")");
			} else {
				logger.debug(getName() + " health: " + Integer.toString(previous) + " -> " + Integer.toString(this.health));
			}
		}
	}
	
	public boolean shieldsUp() {
		return shieldsUp;
	}

	public void update(int [] newLocation, TankSoarMap tankSoarMap) {
		super.update(newLocation);
		
		if (radarSwitch) {
			observedPower = tankSoarMap.getRadar(radar, newLocation, getFacing(), radarPower);
		} else {
			clearRadar();
			observedPower = 0;
		}
		
		blocked = tankSoarMap.getBlocked(newLocation);
}
	
	public int getBlocked() {
		return blocked;
	}
	
	public int getIncoming() {
		return incoming;
	}
	
	public boolean getHumanMove() {
		move = Soar2D.wm.getHumanCommand(this);

		if (move == null) {
			return false;
		}
		
		// Do not allow a move if we rotated.
		if (move.rotate) {
			if (move.move) {
				if (logger.isDebugEnabled()) {
					logger.debug(": move ignored (rotating)");
				}
				move.move = false;
			}
		}
		return true;
	}

	public CommandInfo getMove() {
		resetSensors();
		
		return move;
	}
	
	public void fragged() {
		energy = Soar2D.config.tanksoarConfig().default_energy;
		health = Soar2D.config.tanksoarConfig().default_health;
		missiles = Soar2D.config.tanksoarConfig().default_missiles;
		resurrectFrame = Soar2D.simulation.getWorldCount(); 
		setFacing(Direction.values()[Simulation.random.nextInt(4) + 1]);
		clearRadar();
		resetSensors();
	}
	
	public void setShields(boolean setting) {
		if (shieldsUp == setting) {
			return;
		}
		if (logger.isDebugEnabled()) {
			logger.debug(getName() + " shields switched " + (setting ? "on" : "off"));
		}
		shieldsUp = setting;
	}
	
	public void radarTouch(Direction fromDirection) {
		rwaves |= fromDirection.indicator();
	}
	
	public void setIncoming(Direction fromDirection) {
		incoming |= fromDirection.indicator();
		//System.out.println(getName() + ": incoming set " + incoming);
	}
	public int getRWaves() {
		return rwaves;
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

	public void setSmell(int distance, String smellColor) {
		smellDistance = distance;
		this.smellColor = smellColor;
	}
	
	public int getSmellDistance() {
		return smellDistance;
	}

	public String getSmellColor() {
		return smellColor;
	}

	public void setSound(Direction soundNear) {
		this.sound = soundNear;
	}

	public Direction getSound() {
		return sound;
	}
	
	public void setOnHealthCharger(boolean b) {
		this.onHealthCharger = b;
		
	}

	public void setOnEnergyCharger(boolean b) {
		this.onEnergyCharger = b;
	}
	
	public boolean getOnHealthCharger() {
		return this.onHealthCharger;
		
	}

	public boolean getOnEnergyCharger() {
		return this.onEnergyCharger;
	}
	
	public boolean getResurrect() {
		return Soar2D.simulation.getWorldCount() == resurrectFrame;
	}

	private void clearRadar() {
		radar = new RadarCell[Soar2D.config.tanksoarConfig().radar_width][Soar2D.config.tanksoarConfig().radar_height];
	}

	public void playersChanged() throws Exception {
		// TODO Auto-generated method stub
		
	}

	public void commit(int[] location) {
		// TODO Auto-generated method stub
		
	}

	public void shutdownCommander() throws Exception {
		if (commander != null) {
			commander.shutdown();
		}
	}
}
