package simulation;

import java.util.logging.*;

import org.eclipse.swt.graphics.*;

import sml.*;
import utilities.*;

public class WorldEntity {
	public final static String kNorth = "north";
	public final static String kEast = "east";
	public final static String kSouth = "south";
	public final static String kWest = "west";
	
	public final static int kNorthInt = 1;
	public final static int kEastInt = 2;
	public final static int kSouthInt = 4;
	public final static int kWestInt = 8;
	
	public static String directionToString(int direction) {
		switch (direction) {
			default:
				break;
			case kNorthInt:
				return kNorth;
			case kEastInt:
				return kEast;
			case kSouthInt:
				return kSouth;
			case kWestInt:
				return kWest;
		}
		return null;
	}
	
	private static Logger logger = Logger.getLogger("simulation");
	protected Agent m_Agent;	
	protected Color m_Color;
	protected String m_Facing;
	protected int m_FacingInt;
	
	private String m_Name;
	private int m_Points = 0;
	private MapPoint m_Location;
	private String m_ColorString;
	private String m_Productions;
	private boolean m_Colliding = false;

	public WorldEntity(Agent agent, String productions, String color, MapPoint location) {
		m_Agent = agent;
		m_Location = location;
		m_ColorString = color;
		m_Productions = productions;

		if (m_Agent == null) {
			m_Name = m_Productions;
		} else {
			m_Name = m_Agent.GetAgentName();
		}
		logger.fine("Created agent: " + m_Name);
	}
	
	public String getProductions() {
		return m_Productions;
	}
	
	public void reloadProductions() {
		if (m_Agent == null) {
			return;
		}
		m_Agent.LoadProductions(m_Productions, true);
	}
	
	public String getName() {
		return m_Name;
	}
	
	public int getPoints() {
		return m_Points;
	}
	
	public void setPoints(int score) {
		m_Points = score;
	}
	
	public String getColor() {
		return m_ColorString;
	}
	
	public Agent getAgent() {
		return m_Agent;
	}
	
	public void initSoar() {
		if (m_Agent == null) {
			return;
		}
		m_Agent.InitSoar();
	}
	
	public MapPoint getLocation() {
		return m_Location;
	}
	
	public void adjustPoints(int delta) {
		int previous = m_Points;
		m_Points += delta;
		logger.info(getName() + " score: " + Integer.toString(previous) + " -> " + Integer.toString(m_Points));
	}
	public void setLocation(MapPoint location) {
		m_Location = location;
	}
	
	public String getFacing() {
		return m_Facing;
	}
	
	protected void setFacingInt() {
		if (m_Facing == null) {
			m_FacingInt = 0;
		}
		
		if (m_Facing.equalsIgnoreCase(WorldEntity.kNorth)) {
			m_FacingInt = kNorthInt;
		} else if (m_Facing.equalsIgnoreCase(WorldEntity.kSouth)) {
			m_FacingInt = kSouthInt;
		} else if (m_Facing.equalsIgnoreCase(WorldEntity.kEast)) {
			m_FacingInt = kEastInt;
		} else if (m_Facing.equalsIgnoreCase(WorldEntity.kWest)) {
			m_FacingInt = kWestInt;
		}					
	}

	public int getFacingInt() {
		return m_FacingInt;
	}

	public boolean isColliding() {
		return m_Colliding;
	}
	
	public void setColliding(boolean colliding) {
		m_Colliding = colliding;
	}
}
