package soar2d.map;

import java.util.HashMap;
import java.util.Set;
import java.util.Map.Entry;

import org.apache.log4j.Logger;

import soar2d.Names;

/**
 * @author voigtjr
 *
 * This class represents contents of cells. Other special meta-objects are 
 * also used, like a redraw object to trigger the redrawing of a cell, or
 * an explosion object to trigger the drawing of an explosion.
 */
public class CellObject {
	private static Logger logger = Logger.getLogger(CellObject.class);

	private static final boolean kDefaultPropertyBoolean = false;	// if a bool property doesn't exist
	private static final float kDefaultPropertyFloat = 0;			// if a float property doesn't exist
	private static final int kDefaultPropertyInt = 0;				// if an int property doesn't exist

	HashMap<String, String> properties = new HashMap<String, String>();
	HashMap<String, String> propertiesApply = new HashMap<String, String>();
	String name;

	boolean removeApply = false;

	int rewardApply = 0;
	boolean rewardInfoApply = false;

	boolean resetApply = false;
	boolean pointsApply = false;
	boolean missilesApply = false;
	boolean healthApply = false;
	boolean healthApplyShieldsDown = false;
	boolean energyApply = false;
	boolean fuelApply = false;
	boolean energyApplyShieldsUp = false;
	boolean decayUpdate = false;
	boolean flyMissileUpdate = false;
	boolean lingerUpdate = false;
	
	private static long idCount = 0;
	private Long id;
	
	CellObject(CellObject cellObject) {
		this.id = new Long(idCount++);
		this.properties = new HashMap<String, String>(cellObject.properties);
		this.propertiesApply = new HashMap<String, String>(cellObject.propertiesApply);
		this.name = new String(cellObject.name);
		this.removeApply = cellObject.removeApply;
		this.rewardApply = cellObject.rewardApply;
		this.rewardInfoApply = cellObject.rewardInfoApply;
		this.resetApply = cellObject.resetApply;
		this.pointsApply = cellObject.pointsApply;
		this.missilesApply = cellObject.missilesApply;
		this.healthApply = cellObject.healthApply;
		this.healthApplyShieldsDown = cellObject.healthApplyShieldsDown;
		this.energyApply = cellObject.energyApply;
		this.energyApplyShieldsUp = cellObject.energyApplyShieldsUp;
		this.decayUpdate = cellObject.decayUpdate;
		this.flyMissileUpdate = cellObject.flyMissileUpdate;
		this.lingerUpdate = cellObject.lingerUpdate;
	}
	
	CellObject(String name) {
		if (name == null) {
			throw new NullPointerException();
		}
		this.id = new Long(idCount++);
		this.name = name;
	}
	
	public boolean equals(CellObject other) {
		return id == other.id;
	}
	
	public String getName() {
		return name;
	}
	public boolean updatable() {
		return this.decayUpdate
			|| this.flyMissileUpdate
			|| this.lingerUpdate;
	}
	
	public boolean applyable() {
		return (propertiesApply.size() > 0) 
			|| this.pointsApply 
			|| this.energyApply 
			|| this.healthApply 
			|| this.missilesApply 
			|| this.removeApply
			|| (this.rewardApply > 0)
			|| this.rewardInfoApply
			|| this.resetApply;
	}
	
	/**
	 * @param name property name
	 * @param value property value
	 * 
	 * Add a property apply, that is, a property that is added to this object
	 * when it is applied.
	 * Note, this overwrites an existing property if necessary.
	 */
	public void addPropertyApply(String name, String value) {
		propertiesApply.put(name, value);
	}
	public String getPropertyApply(String name) {
		return propertiesApply.get(name);
	}
	
	public boolean getResetApply() {
		return this.resetApply;
	}
	
	/**
	 * @param name property name
	 * @param value property value
	 * 
	 * Add a property to this object.
	 * Note, this overwrites an existing property if necessary.
	 */
	public void addProperty(String name, String value) {
		properties.put(name, value);
	}
	
	public void setRemoveApply(boolean setting) {
		removeApply = setting;
	}
	
	public void setRewardApply(int setting) {
		assert setting >= 0;
		rewardApply = setting;
	}
	
	public void setRewardInfoApply(boolean setting) {
		rewardInfoApply = setting;
	}
	
	public void setResetApply(boolean setting) {
		resetApply = setting;
	}
	
	public void setFuelApply(boolean setting) {
		fuelApply = setting;
	}
	
	public void setPointsApply(boolean setting) {
		pointsApply = setting;
	}
	
	public void setMissilesApply(boolean setting) {
		missilesApply = setting;
	}
	
	public void setLingerUpdate(boolean setting) {
		lingerUpdate = setting;
	}
	
	/**
	 * @see energyApply
	 * @see energyApplyShieldsUp
	 */
	public void setEnergyApply(boolean setting, boolean onShieldsUp) {
		this.energyApply = setting;
		this.energyApplyShieldsUp = onShieldsUp;
	}
	
	/**
	 * @see healthApply
	 * @see healthApplyShieldsDown
	 */
	public void setHealthApply(boolean setting, boolean onShieldsDown) {
		this.healthApply = setting;
		this.healthApplyShieldsDown = onShieldsDown;
	}
	
	public void setDecayUpdate(boolean setting) {
		decayUpdate = setting;
	}
	
	public void setName(String name) {
		this.name = name;
	}
	
	public void setFlyMissileUpdate(boolean setting) {
		flyMissileUpdate = setting;
	}
	
	public void propertiesApply() {
		for (String key : propertiesApply.keySet()) {
			String value = propertiesApply.get(key);
			logger.info(Names.Info.newProperty + key + " --> " + value);
			properties.put(key, value);
		}
	}

	public int pointsApply() {
		if (pointsApply) {
			assert properties.containsKey(Names.kPropertyPoints);
			return Integer.parseInt(properties.get(Names.kPropertyPoints));
		}
		return 0;
	}
		
	public int missilesApply() {
		if (missilesApply) {
			assert properties.containsKey(Names.kPropertyMissiles);
			return Integer.parseInt(properties.get(Names.kPropertyMissiles));
		}
		return 0;
	}
		
	public int energyApply(boolean shieldsUp) {
		if (energyApply) {
			if (!energyApplyShieldsUp || shieldsUp) {
				assert properties.containsKey(Names.kPropertyEnergy);
				return Integer.parseInt(properties.get(Names.kPropertyEnergy));
			}
		}
		return 0;
	}
		
	public int healthApply(boolean shieldsUp) {
		if (healthApply) {
			if (!healthApplyShieldsDown || !shieldsUp) {
				assert properties.containsKey(Names.kPropertyHealth);
				return Integer.parseInt(properties.get(Names.kPropertyHealth));
			}
		}
		return 0;
	}
		
	public static class RewardApply {
		RewardApply(int points, String message) {
			this.points = points;
			this.message = message;
		}
		public int points;
		public String message;
	}
	
	public RewardApply rewardApply() {
		// Reward apply is only true on the reward boxes, not info boxes
		if (rewardApply > 0) {
			// am I the positive box
			if (rewardBox) {
				// if the open code is not zero, get an open code
				int suppliedOpenCode = 0;
				if (openCode != 0) {
					// get the open code if any
					if (properties.containsKey(Names.kPropertyOpenCode)) {
						suppliedOpenCode = Integer.parseInt(properties.get(Names.kPropertyOpenCode));
					}
				}
				
				// see if we opened the box correctly
				if (suppliedOpenCode == openCode) {
					// reward positively
					return new RewardApply(rewardApply, "positive reward");
				} else {
					return new RewardApply(1, "small reward (wrong open code)");
				}
			} else {
				// I'm  not the positive box, set resetApply false
				resetApply = false;
				
				// reward negatively
				return new RewardApply(-1, "negative reward (wrong box)");
			}
		}
		return null;
	}
	
	public boolean removeApply() {
		return removeApply;
	}
	
	boolean rewardBox = false;
	void setRewardBox() {
		rewardBox = true;
	}
	int openCode = 0;
	void setOpenCode(int openCode) {
		this.openCode = openCode;
	}

	public boolean update() {
		if (decayUpdate) {
			assert properties.containsKey(Names.kPropertyPoints);
			int points = Integer.parseInt(properties.get(Names.kPropertyPoints));
			points -= 1;
			properties.put(Names.kPropertyPoints, Integer.toString(points));
			if (points == 0) {
				return true;	// this causes this object to be removed from the cell
			}
		}
		
		if (flyMissileUpdate) {
			int phase = this.getIntProperty(Names.kPropertyFlyPhase);
			phase += 1;
			phase %= 4;
			properties.put(Names.kPropertyFlyPhase, Integer.toString(phase));
			return true;
		}
		
		if (lingerUpdate) {
			assert properties.containsKey(Names.kPropertyLinger);
			int remaining = Integer.parseInt(properties.get(Names.kPropertyLinger));
			remaining -= 1;
			if (remaining == 0) {
				logger.trace("Linger object expired.");
				return true;
			}
			properties.put(Names.kPropertyLinger, Integer.toString(remaining));
		}
		
		return false;
	}
	
	/**
	 * @param name the property to check for
	 * @return true if that property exists
	 * 
	 * Note: property foo -> false returns true here, this doesn't return the value
	 * of the property, but rather its existence.
	 */
	public boolean hasProperty(String name) {
		return properties.containsKey(name);
	}
	
	/**
	 * @return a set of property names
	 */
	public Set< Entry<String, String> >getPropertyEntries() {
		return properties.entrySet();
	}
	/**
	 * @return a set of property names
	 */
	public Set<String> getPropertyNames() {
		return properties.keySet();
	}
	/**
	 * @param name the property name
	 * @return the property value or null if it doesn't exist
	 */
	public String getProperty(String name) {
		return properties.get(name);
	}
	/**
	 * @param name the property
	 * @return the string converted to a boolean
	 * 
	 * same as getProperty but converts it to boolean first
	 * use with care
	 */
	public boolean getBooleanProperty(String name) {
		String property = properties.get(name);
		if (property == null) {
			return kDefaultPropertyBoolean;
		}
		return Boolean.parseBoolean(property);
	}
	/**
	 * @param name the property
	 * @return the string converted to a int
	 * 
	 * same as getProperty but converts it to int first
	 * use with care
	 */
	public int getIntProperty(String name) {
		String property = properties.get(name);
		if (property == null) {
			return kDefaultPropertyInt;
		}
		return Integer.parseInt(property);
	}
	/**
	 * @param name the property
	 * @return the string converted to a float
	 * 
	 * same as getProperty but converts it to float first
	 * use with care
	 */
	public float getFloatProperty(String name) {
		String property = properties.get(name);
		if (property == null) {
			return kDefaultPropertyFloat;
		}
		return Float.parseFloat(property);
	}

	public void removeProperty(String property) {
		properties.remove(property);
	}
}
