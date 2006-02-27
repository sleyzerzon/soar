package tanksoar;

import org.eclipse.swt.graphics.*;

import sml.*;

public class Tank {
	
	private final static String kBlockedID = "blocked";
	private final static String kClockID = "clock";
	private final static String kColorID = "color";
	private final static String kDirectionID = "direction";
	private final static String kDistanceID = "distance";
	private final static String kEnergyID = "energy";
	private final static String kEnergyRechargerID = "energyrecharger";
	private final static String kFireID = "fire";
	private final static String kHealthID = "health";
	private final static String kHealthRechargerID = "healthrecharger";
	private final static String kIncomingID = "incoming";
	private final static String kMissilesID = "missiles";
	private final static String kMoveID = "move";
	private final static String kMyColorID = "my-color";
	private final static String kObstacleID = "obstacle";
	private final static String kOpenID = "open";
	private final static String kTankID = "tank";
	private final static String kRadarID = "radar";
	private final static String kRadarDistanceID = "radar-distance";
	private final static String kRadarPowerID = "radar-power";
	private final static String kRadarSettingID = "radar-setting";
	private final static String kRadarStatusID = "radar-status";
	private final static String kRandomID = "random";
	private final static String kResurrectID = "resurrect";
	private final static String kRotateID = "rotate";
	private final static String kRWavesID = "rwaves";
	private final static String kSettingID = "setting";
	private final static String kShields = "shields";
	private final static String kShieldStatusID = "shield-status";
	private final static String kSmellID = "smell";
	private final static String kSoundID = "sound";
	private final static String kSwitchID = "switch";
	private final static String kWeaponID = "missile";
	private final static String kXID = "x";
	private final static String kYID = "y";
	
	private final static String kBackwardID = "backward";
	private final static String kForwardID = "forward";
	private final static String kLeftID = "left";
	private final static String kRightID = "right";
	private final static String kSilentID = "silent";
	private final static String kMissileID = "missile";
	
	public final static String kEast = "east";
	public final static String kNorth = "north";
	public final static String kSouth = "south";
	public final static String kWest = "west";
	
	private final static String kYes = "yes";
	private final static String kNo = "no";
	
	private final static String kOff = "off";
	private final static String kOn = "on";
	
	// The different types of commands
	public final static int kFireInt = 0;
	public final static int kMoveInt = 1;
	public final static int kRadarInt = 2;
	public final static int kRadarPowerInt = 3;
	public final static int kRotateInt = 4;
	public final static int kShieldsInt = 5;
	
	public Tank(Agent agent, String productions, String color, Point location) {
		
	}
	
	public void setScore(int newScore) {
		
	}
	
	public void initSoar() {
		
	}
	
	public void setLocation(Point location) {
		
	}
	
	public void updateInput(World world) {
		
	}
	
	public Point getLocation() {
		return null;
	}
	
	public class MoveInfo {
		int type;
		String weapon; // fire (this is kind of unnecessary, missile only) 
		String direction; // move, rotate
		boolean toggle; // radar, shields
		int setting; // radar-power
	}
	
	public MoveInfo[] getMoves() {
		return null;
	}
}
