package tanksoar;

import simulation.*;

public class TankSoarCell extends Cell {
	// background
	private static final int kWallInt = 0;
	private static final int kOpenInt = 1;
	private static final int kEnergyInt = 2;
	private static final int kHealthInt = 3;
	
	// contents
	private static final int kNothingInt = 0;
	private static final int kTankInt = 1;
	private static final int kMissilePackInt = 2;

	private Tank m_Tank;
	private int m_Contents = 0;
	
	// used in path searching
	private java.awt.Point m_Parent = null;
	private int distance = -1;
	
	private boolean m_Explosion = false;
	protected boolean m_RadarTouch = false;
	static boolean s_EnergyChargerCreated = false;
	static boolean s_HealthChargerCreated = false;
	private boolean m_Modified = false;

	public TankSoarCell() {
		m_Type = kOpenInt;
	}

	public boolean setType(String name) {
		if (name.equalsIgnoreCase(TankSoarWorld.kTypeWall)) {
			m_Type = kWallInt;
		} else if (name.equalsIgnoreCase(TankSoarWorld.kTypeEmpty)) {
			m_Type = kOpenInt;			
		} else if (name.equalsIgnoreCase(TankSoarWorld.kTypeEnergyRecharger)) {
			m_Type = kEnergyInt;	
			s_EnergyChargerCreated = true;
		} else if (name.equalsIgnoreCase(TankSoarWorld.kTypeHealthRecharger)) {
			m_Type = kHealthInt;	
			s_HealthChargerCreated = true;
		} else {	
			return false;
		}
		return true;
	}

	int getDistance() {
		return distance;
	}
	
	void setDistance(int distance) {
		this.distance = distance;
		if (distance < 0) {
			m_Parent = null;
		}
	}
	
	java.awt.Point getParent() {
		return m_Parent;
	}
	
	void setParent(java.awt.Point parent) {
		m_Parent = parent;
	}
	
	public boolean isBlocked() {
		return (m_Type == kWallInt) || (m_Contents == kTankInt);
	}
	
	public boolean isWall() {
		return m_Type == kWallInt;
	}
	
	public boolean isOpen() {
		return m_Type == kOpenInt;
	}
	
	public boolean isEnergyRecharger() {
		return m_Type == kEnergyInt;
	}
	
	public boolean isHealthRecharger() {
		return m_Type == kHealthInt;
	}
	
	public boolean containsTank() {
		return m_Contents == kTankInt;
	}
	
	public boolean hasContents() {
		return m_Contents != kNothingInt;
	}
	
	public void setTank(Tank tank) {
		m_Redraw = true;
		m_Contents = kTankInt;
		m_Tank = tank;
		setModified();
	}
	
	void setRedraw() {
		m_Redraw = true;
	}
	
	void setRadarTouch() {
		m_RadarTouch = true;
		m_Redraw = true;
	}
	
	public Tank getTank() {
		return m_Tank;
	}
	
	public void removeTank(Tank tankToRemove) {
		if ((m_Contents != kTankInt) || (!tankToRemove.equals(m_Tank))) {
			return;
		}
		m_Redraw = true;
		m_Contents = kNothingInt;
		m_Tank = null;
		setModified();
		return;
	}
	
	public boolean containsMissilePack() {
		return m_Contents == kMissilePackInt;
	}
	
	void setHealth() {
		m_Redraw = true;
		m_Type = kHealthInt;
		setModified();
	}
	
	void setEnergy() {
		m_Redraw = true;
		m_Type = kEnergyInt;
		setModified();
	}
	
	void setMissilePack() {
		m_Redraw = true;
		m_Contents = kMissilePackInt;
		setModified();
	}
	
	void setExplosion() {
		m_Redraw = true;
		m_Explosion = true;
	}
	
	public boolean isExplosion() {
		return m_Explosion;
	}

	public void clearRedraw() {
		if (m_RadarTouch || m_Explosion) {
			setRedraw();
			m_RadarTouch = false;
			m_Explosion = false;
		} else {
			super.clearRedraw();
		}
	}
	
	private void setModified() {
		m_Modified = true;
	}
	
	public void clearModified() {
		m_Modified = false;
	}
	
	public boolean isModified() {
		return m_Modified;
	}
}
