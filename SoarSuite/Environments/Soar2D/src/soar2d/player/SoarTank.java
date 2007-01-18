package soar2d.player;

import java.util.*;
import java.util.logging.*;

import sml.*;
import soar2d.*;

public class SoarTank extends Tank implements Agent.RunEventInterface {
	private Agent agent;
	private ArrayList<String> shutdownCommands;

	private Identifier m_InputLink;
	private Identifier m_BlockedWME;
	private StringElement m_BlockedBackwardWME;
	private StringElement m_BlockedForwardWME;
	private StringElement m_BlockedLeftWME;
	private StringElement m_BlockedRightWME;
	private IntElement m_ClockWME;
	private Identifier m_CurrentScoreWME;
	
	private IntElement m_BlueScore;
	private IntElement m_RedScore;
	private IntElement m_YellowScore;
	private IntElement m_GreenScore;
	private IntElement m_PurpleScore;
	private IntElement m_OrangeScore;
	private IntElement m_BlackScore;
	
	private StringElement m_DirectionWME;
	private IntElement m_EnergyWME;
	private StringElement m_EnergyRechargerWME;
	private IntElement m_HealthWME;
	private StringElement m_HealthRechargerWME;
	private Identifier m_IncomingWME;
	private StringElement m_IncomingBackwardWME;
	private StringElement m_IncomingForwardWME;
	private StringElement m_IncomingLeftWME;
	private StringElement m_IncomingRightWME;
	private IntElement m_MissilesWME;
	private StringElement m_MyColorWME;
	private StringElement m_RadarStatusWME;
	private IntElement m_RadarDistanceWME;
	private IntElement m_RadarSettingWME;
	private Identifier m_RadarWME;
	private FloatElement m_RandomWME;
	private StringElement m_ResurrectWME;
	private Identifier m_RWavesWME;
	private StringElement m_RWavesBackwardWME;
	private StringElement m_RWavesForwardWME;
	private StringElement m_RWavesLeftWME;
	private StringElement m_RWavesRightWME;
	private StringElement m_ShieldStatusWME;
	private Identifier m_SmellWME;
	private StringElement m_SmellColorWME;
	private IntElement m_SmellDistanceWME;
	private StringElement m_SmellDistanceStringWME;
	private StringElement m_SoundWME;
	private IntElement m_xWME;
	private IntElement m_yWME;			

	private Identifier[][] radarCellIDs = new Identifier[Soar2D.config.kRadarWidth][Soar2D.config.kRadarHeight];
	private StringElement[][] radarColors = new StringElement[Soar2D.config.kRadarWidth][Soar2D.config.kRadarHeight];

	private float random = 0;
	private boolean m_Reset = true;
	private int m_ResurrectFrame = 0;
	
	private boolean playersChanged = true;
	private boolean attemptedMove = false;
	
	public SoarTank(Agent agent, PlayerConfig playerConfig) {
		super(playerConfig);
		this.agent = agent;
		this.shutdownCommands = playerConfig.getShutdownCommands();

		assert agent != null;
		
		agent.RegisterForRunEvent(smlRunEventId.smlEVENT_AFTER_INTERRUPT, this, null);
		m_InputLink = agent.GetInputLink();

		previousLocation = new java.awt.Point(-1, -1);
	}
	
	public void runEventHandler(int eventID, Object data, Agent agent, int phase) {
		assert eventID == smlRunEventId.smlEVENT_AFTER_INTERRUPT.swigValue();
		if (!Soar2D.control.isStopped()) {
			logger.warning(getName() + ": agent interrupted");
		}
	}
	
	private void DestroyWME(WMElement wme) {
		assert wme != null;
		agent.DestroyWME(wme);
	}

	private void Update(StringElement wme, String value) {
		assert wme != null;
		assert value != null;
		agent.Update(wme, value);
	}

	private void Update(IntElement wme, int value) {
		assert wme != null;
		agent.Update(wme, value);
	}

	private void Update(FloatElement wme, float value) {
		assert wme != null;
		agent.Update(wme, value);
	}
	
	private IntElement CreateIntWME(Identifier id, String attribute, int value) {
		assert id != null;
		assert attribute != null;
		return agent.CreateIntWME(id, attribute, value);
	}

	private StringElement CreateStringWME(Identifier id, String attribute, String value) {
		assert id != null;
		assert attribute != null;
		assert value != null;
		return agent.CreateStringWME(id, attribute, value);
	}

	private FloatElement CreateFloatWME(Identifier id, String attribute, float value) {
		assert id != null;
		assert attribute != null;
		return agent.CreateFloatWME(id, attribute, value);
	}

	public void update(World world, java.awt.Point location) {
		super.update(world, location);
	}
	
	public MoveInfo getMove() {
		resetSensors();

		attemptedMove = false;

		assert agent != null;
		int numberOfCommands = agent.GetNumberCommands();
		if (numberOfCommands == 0) {
			if (logger.isLoggable(Level.FINE)) logger.fine(getName() + " issued no command.");
			return new MoveInfo();
		}
		
		Identifier moveId = null;
		MoveInfo move = new MoveInfo();
		
		for (int i = 0; i < numberOfCommands; ++i) {
			Identifier commandId = agent.GetCommand(i);
			String commandName = commandId.GetAttribute();

			if (commandName.equalsIgnoreCase(Names.kMoveID)) {
				if (move.move == true) {
					if (logger.isLoggable(Level.FINE)) logger.fine(getName() + ": extra move commands");
					commandId.AddStatusError();
					continue;
				}

				String moveDirection = commandId.GetParameterValue(Names.kDirectionID);
				if (moveDirection == null) {
					if (logger.isLoggable(Level.FINE)) logger.fine(getName() + ": null move direction");
					commandId.AddStatusError();
					continue;
				}
				
				if (moveDirection.equalsIgnoreCase(Names.kForwardID)) {
					move.moveDirection = getFacingInt();
				} else if (moveDirection.equalsIgnoreCase(Names.kBackwardID)) {
					move.moveDirection = Direction.backwardOf[this.getFacingInt()];
				} else if (moveDirection.equalsIgnoreCase(Names.kLeftID)) {
					move.moveDirection = Direction.leftOf[this.getFacingInt()];
				} else if (moveDirection.equalsIgnoreCase(Names.kRightID)) {
					move.moveDirection = Direction.rightOf[this.getFacingInt()];
				} else {
					if (logger.isLoggable(Level.FINE)) logger.fine(getName() + ": illegal move direction: " + moveDirection);
					commandId.AddStatusError();
					continue;
				}
				moveId = commandId;
				move.move = true;
				attemptedMove = true;
				
			} else if (commandName.equalsIgnoreCase(Names.kFireID)) {
				if (move.fire == true) {
					if (logger.isLoggable(Level.FINE)) logger.fine(getName() + ": extra fire commands");
					commandId.AddStatusError();
					continue;
				}
	 			move.fire = true;

	 			// Weapon ignored
				
			} else if (commandName.equalsIgnoreCase(Names.kRadarID)) {
				if (move.radar == true) {
					if (logger.isLoggable(Level.FINE)) logger.fine(getName() + ": extra radar commands");
					commandId.AddStatusError();
					continue;
				}
				
				String radarSwitch = commandId.GetParameterValue(Names.kSwitchID);
				if (radarSwitch == null) {
					if (logger.isLoggable(Level.FINE)) logger.fine(getName() + ": null radar switch");
					commandId.AddStatusError();
					continue;
				}
				move.radar = true;
				move.radarSwitch = radarSwitch.equalsIgnoreCase(Names.kOn) ? true : false;  
				
			} else if (commandName.equalsIgnoreCase(Names.kRadarPowerID)) {
				if (move.radarPower == true) {
					if (logger.isLoggable(Level.FINE)) logger.fine(getName() + ": extra radar power commands");
					commandId.AddStatusError();
					continue;
				}
				
				String powerValue = commandId.GetParameterValue(Names.kSettingID);
				if (powerValue == null) {
					if (logger.isLoggable(Level.FINE)) logger.fine(getName() + ": null radar power");
					commandId.AddStatusError();
					continue;
				}
				
				try {
					move.radarPowerSetting = Integer.decode(powerValue).intValue();
				} catch (NumberFormatException e) {
					if (logger.isLoggable(Level.FINE)) logger.fine(getName() + ": unable to parse radar power setting " + powerValue + ": " + e.getMessage());
					commandId.AddStatusError();
					continue;
				}
				move.radarPower = true;
				
			} else if (commandName.equalsIgnoreCase(Names.kShieldsID)) {
				if (move.shields == true) {
					if (logger.isLoggable(Level.FINE)) logger.fine(getName() + ": extra shield commands");
					commandId.AddStatusError();
					continue;
				}
				
				String shieldsSetting = commandId.GetParameterValue(Names.kSwitchID);
				if (shieldsSetting == null) {
					if (logger.isLoggable(Level.FINE)) logger.fine(getName() + ": null shields setting");
					commandId.AddStatusError();
					continue;
				}
				move.shields = true;
				move.shieldsSetting = shieldsSetting.equalsIgnoreCase(Names.kOn) ? true : false; 
				
			} else if (commandName.equalsIgnoreCase(Names.kRotateID)) {
				if (move.rotate == true) {
					if (logger.isLoggable(Level.FINE)) logger.fine(getName() + ": extra rotate commands");
					commandId.AddStatusError();
					continue;
				}
				
				move.rotateDirection = commandId.GetParameterValue(Names.kDirectionID);
				if (move.rotateDirection == null) {
					if (logger.isLoggable(Level.FINE)) logger.fine(getName() + ": null rotation direction");
					commandId.AddStatusError();
					continue;
				}
				
				move.rotate = true;
				
			} else {
				logger.warning(getName() + ": unknown command: " + commandName);
				commandId.AddStatusError();
				continue;
			}
			commandId.AddStatusComplete();
		}
		
    	agent.ClearOutputLinkChanges();
		agent.Commit();
		
		// Do not allow a move if we rotated.
		if (move.rotate) {
			if (move.move) {
				logger.info(": move ignored (rotating)");
				assert moveId != null;
				moveId.AddStatusError();
				moveId = null;
				move.move = false;
			}
		}

		return move;
	}
	
	public void reset() {
		super.reset();
		
		if (agent == null) {
			return;
		}
		
		agent.InitSoar();
	}
	
	public void fragged() {
		super.fragged();
		
		if (m_Reset == true) {
			return;
		}
		
		clearWMEs();
		
		m_Reset = true;
	}
	
	void clearWMEs() {
		DestroyWME(m_BlockedWME);
		m_BlockedWME = null;
		
		DestroyWME(m_ClockWME);
		m_ClockWME = null;
		
		DestroyWME(m_CurrentScoreWME);
		m_CurrentScoreWME = null;
		m_BlueScore = null;
		m_RedScore = null;
		m_YellowScore = null;
		m_GreenScore = null;
		m_PurpleScore = null;
		m_OrangeScore = null;
		m_BlackScore = null;

		DestroyWME(m_DirectionWME);
		m_DirectionWME = null;
		
		DestroyWME(m_EnergyWME);
		m_EnergyWME = null;
		
		DestroyWME(m_EnergyRechargerWME);
		m_EnergyRechargerWME = null;
		
		DestroyWME(m_HealthWME);
		m_HealthWME = null;
		
		DestroyWME(m_HealthRechargerWME);
		m_HealthRechargerWME = null;
		
		DestroyWME(m_IncomingWME);
		m_IncomingWME = null;
		
		DestroyWME(m_MissilesWME);
		m_MissilesWME = null;
		
		DestroyWME(m_MyColorWME);
		m_MyColorWME = null;
		
		DestroyWME(m_RadarStatusWME);
		m_RadarStatusWME = null;
		
		DestroyWME(m_RadarDistanceWME);
		m_RadarDistanceWME = null;
		
		DestroyWME(m_RadarSettingWME);
		m_RadarSettingWME = null;
		
		if (m_RadarWME != null) {
			DestroyWME(m_RadarWME);
			m_RadarWME = null;
		}
		DestroyWME(m_RandomWME);
		m_RandomWME = null;
		
		DestroyWME(m_ResurrectWME);
		m_ResurrectWME = null;
		
		DestroyWME(m_RWavesWME);
		m_RWavesWME = null;
		
		DestroyWME(m_ShieldStatusWME);
		m_ShieldStatusWME = null;
		
		DestroyWME(m_SmellWME);
		m_SmellWME = null;
		
		DestroyWME(m_SoundWME);
		m_SoundWME = null;
		
		DestroyWME(m_xWME);
		m_xWME = null;
		
		DestroyWME(m_yWME);
		m_yWME = null;
		
		agent.Commit();

		clearRadar();
	}
	
	void initScoreWMEs() {
		if (m_CurrentScoreWME == null) {
			return;
		}
		
		boolean blueSeen = false;
		boolean redSeen = false;
		boolean yellowSeen = false;
		boolean greenSeen = false;
		boolean purpleSeen = false;
		boolean orangeSeen = false;
		boolean blackSeen = false;
		
		ArrayList<Player> players = Soar2D.simulation.world.getPlayers();
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			Player player = iter.next();

			if (player.getColor().equals("blue")) {
				blueSeen = true;
				if (m_BlueScore == null) {
					m_BlueScore = agent.CreateIntWME(m_CurrentScoreWME, "blue", player.getPoints());
				}
			} else if (player.getColor().equals("red")) {
				redSeen = true;
				if (m_RedScore == null) {
					m_RedScore = agent.CreateIntWME(m_CurrentScoreWME, "red", player.getPoints());
				}
			} else if (player.getColor().equals("yellow")) {
				yellowSeen = true;
				if (m_YellowScore == null) {
					m_YellowScore = agent.CreateIntWME(m_CurrentScoreWME, "yellow", player.getPoints());
				}
			} else if (player.getColor().equals("green")) {
				greenSeen = true;
				if (m_GreenScore == null) {
					m_GreenScore = agent.CreateIntWME(m_CurrentScoreWME, "green", player.getPoints());
				}
			} else if (player.getColor().equals("purple")) {
				purpleSeen = true;
				if (m_PurpleScore == null) {
					m_PurpleScore = agent.CreateIntWME(m_CurrentScoreWME, "purple", player.getPoints());
				}
			} else if (player.getColor().equals("orange")) {
				orangeSeen = true;
				if (m_OrangeScore == null) {
					m_OrangeScore = agent.CreateIntWME(m_CurrentScoreWME, "orange", player.getPoints());
				}
			} else if (player.getColor().equals("black")) {
				blackSeen = true;
				if (m_BlackScore == null) {
					m_BlackScore = agent.CreateIntWME(m_CurrentScoreWME, "black", player.getPoints());
				}
			}
			player.resetPointsChanged();
		}
		
		if (blueSeen == false) {
			if (m_BlueScore != null) {
				DestroyWME(m_BlueScore);
				m_BlueScore = null;
			}
		}
		if (redSeen == false) {
			if (m_RedScore != null) {
				DestroyWME(m_RedScore);
				m_RedScore = null;
			}
		}
		if (yellowSeen == false) {
			if (m_YellowScore != null) {
				DestroyWME(m_YellowScore);
				m_YellowScore = null;
			}
		}
		if (greenSeen == false) {
			if (m_GreenScore != null) {
				DestroyWME(m_GreenScore);
				m_GreenScore = null;
			}
		}
		if (purpleSeen == false) {
			if (m_PurpleScore != null) {
				DestroyWME(m_PurpleScore);
				m_PurpleScore = null;
			}
		}
		if (orangeSeen == false) {
			if (m_OrangeScore != null) {
				DestroyWME(m_OrangeScore);
				m_OrangeScore = null;
			}
		}
		if (blackSeen == false) {
			if (m_BlackScore != null) {
				DestroyWME(m_BlackScore);
				m_BlackScore = null;
			}
		}
		
		playersChanged = false;
	}

	public void playersChanged() {
		playersChanged = true;
	}

	public void commit(java.awt.Point location) {
		int facing = getFacingInt();
		String facingString = Direction.stringOf[facing];
		World world = Soar2D.simulation.world;
		String shieldStatus = shieldsUp ? Names.kOn : Names.kOff;
		String blockedForward = ((blocked & Direction.indicators[facing]) > 0) ? Names.kYes : Names.kNo;
		String blockedBackward = ((blocked & Direction.indicators[Direction.backwardOf[facing]]) > 0) ? Names.kYes : Names.kNo;
		String blockedLeft = ((blocked & Direction.indicators[Direction.leftOf[facing]]) > 0) ? Names.kYes : Names.kNo;
		String blockedRight = ((blocked & Direction.indicators[Direction.rightOf[facing]]) > 0) ? Names.kYes : Names.kNo;
		String incomingForward = ((incoming & Direction.indicators[facing]) > 0) ? Names.kYes : Names.kNo;
		String incomingBackward = ((incoming & Direction.indicators[Direction.backwardOf[facing]]) > 0) ? Names.kYes : Names.kNo;
		String incomingLeft = ((incoming & Direction.indicators[Direction.leftOf[facing]]) > 0) ? Names.kYes : Names.kNo;
		String incomingRight = ((incoming & Direction.indicators[Direction.rightOf[facing]]) > 0) ? Names.kYes : Names.kNo;
		String smellColorString = (smellColor == null) ? Names.kNone : smellColor;
		String soundString;
		if (sound == facing) {
			soundString = Names.kForwardID;
		} else if (sound == Direction.backwardOf[facing]) {
			soundString = Names.kBackwardID;
		} else if (sound == Direction.leftOf[facing]) {
			soundString = Names.kLeftID;
		} else if (sound == Direction.rightOf[facing]) {
			soundString = Names.kRightID;
		} else {
			soundString = Names.kSilentID;
		}
		int worldCount = world.getWorldCount();
		String radarStatus = radarSwitch ? Names.kOn : Names.kOff;
		float oldrandom = random;
		do {
			random = Simulation.random.nextFloat();
		} while (random == oldrandom);
		String rwavesForward = (rwaves & facing) > 0 ? Names.kYes : Names.kNo;
		String rwavesBackward = (rwaves & Direction.indicators[Direction.backwardOf[facing]]) > 0 ? Names.kYes : Names.kNo;;
		String rwavesLeft = (rwaves & Direction.indicators[Direction.leftOf[facing]]) > 0 ? Names.kYes : Names.kNo;
		String rwavesRight = (rwaves & Direction.indicators[Direction.rightOf[facing]]) > 0 ? Names.kYes : Names.kNo;

		if (m_Reset) {
			// location
			m_xWME = CreateIntWME(m_InputLink, Names.kXID, location.x);
			m_yWME = CreateIntWME(m_InputLink, Names.kYID, location.y);
			
			// charger detection
			String energyRecharger = onEnergyCharger ? Names.kYes : Names.kNo;
			m_EnergyRechargerWME = CreateStringWME(m_InputLink, Names.kEnergyRechargerID, energyRecharger);

			String healthRecharger = onHealthCharger ? Names.kYes : Names.kNo;
			m_HealthRechargerWME = CreateStringWME(m_InputLink, Names.kHealthRechargerID, healthRecharger);

			// facing
			m_DirectionWME = CreateStringWME(m_InputLink, Names.kDirectionID, facingString);

			// energy and health status
			m_EnergyWME = CreateIntWME(m_InputLink, Names.kEnergyID, energy);
			m_HealthWME = CreateIntWME(m_InputLink, Names.kHealthID, health);

			// shield status
			m_ShieldStatusWME = CreateStringWME(m_InputLink, Names.kShieldStatusID, shieldStatus);

			// blocked sensor
			m_BlockedWME = agent.CreateIdWME(m_InputLink, Names.kBlockedID);
			m_BlockedForwardWME = CreateStringWME(m_BlockedWME, Names.kForwardID, blockedForward);
			m_BlockedBackwardWME = CreateStringWME(m_BlockedWME, Names.kBackwardID, blockedBackward);
			m_BlockedLeftWME = CreateStringWME(m_BlockedWME, Names.kLeftID, blockedLeft);
			m_BlockedRightWME = CreateStringWME(m_BlockedWME, Names.kRightID, blockedRight);				

			// score status
			m_CurrentScoreWME = agent.CreateIdWME(m_InputLink, Names.kCurrentScoreID);
			initScoreWMEs();

			// incoming sensor
			m_IncomingWME = agent.CreateIdWME(m_InputLink, Names.kIncomingID);
			m_IncomingBackwardWME = CreateStringWME(m_IncomingWME, Names.kBackwardID, incomingForward);
			m_IncomingForwardWME = CreateStringWME(m_IncomingWME, Names.kForwardID, incomingBackward);
			m_IncomingLeftWME = CreateStringWME(m_IncomingWME, Names.kLeftID, incomingLeft);
			m_IncomingRightWME = CreateStringWME(m_IncomingWME, Names.kRightID, incomingRight);
			
			// smell sensor
			m_SmellWME = agent.CreateIdWME(m_InputLink, Names.kSmellID);
			m_SmellColorWME = CreateStringWME(m_SmellWME, Names.kColorID, smellColorString);
			if (smellColor == null) {
				m_SmellDistanceWME = null;
				m_SmellDistanceStringWME = CreateStringWME(m_SmellWME, Names.kDistanceID, Names.kNone);
			} else {
				m_SmellDistanceWME = CreateIntWME(m_SmellWME, Names.kDistanceID, smellDistance);
				m_SmellDistanceStringWME = null;
			}

			// sound sensor
			m_SoundWME = CreateStringWME(m_InputLink, Names.kSoundID, soundString);			

			// missile quantity indicator
			m_MissilesWME = CreateIntWME(m_InputLink, Names.kMissilesID, missiles);

			// my color
			m_MyColorWME = CreateStringWME(m_InputLink, Names.kMyColorID, getColor());

			// clock (world count)
			m_ClockWME = CreateIntWME(m_InputLink, Names.kClockID, worldCount);

			// resurrect sensor
			m_ResurrectFrame = worldCount;
			m_ResurrectWME = CreateStringWME(m_InputLink, Names.kResurrectID, Names.kYes);

			// radar sensors
			m_RadarStatusWME = CreateStringWME(m_InputLink, Names.kRadarStatusID, radarStatus);
			if (radarSwitch) {
				m_RadarWME = agent.CreateIdWME(m_InputLink, Names.kRadarID);
				generateNewRadar();
			} else {
				m_RadarWME = null;
			}
			m_RadarDistanceWME = CreateIntWME(m_InputLink, Names.kRadarDistanceID, observedPower);
			m_RadarSettingWME = CreateIntWME(m_InputLink, Names.kRadarSettingID, radarPower);

			// random indicator
			m_RandomWME = CreateFloatWME(m_InputLink, Names.kRandomID, random);

			// rwaves sensor
			m_RWavesWME = agent.CreateIdWME(m_InputLink, Names.kRWavesID);
			m_RWavesForwardWME = CreateStringWME(m_RWavesWME, Names.kForwardID, rwavesBackward);
			m_RWavesBackwardWME = CreateStringWME(m_RWavesWME, Names.kBackwardID, rwavesForward);
			m_RWavesLeftWME = CreateStringWME(m_RWavesWME, Names.kLeftID, rwavesLeft);
			m_RWavesRightWME = CreateStringWME(m_RWavesWME, Names.kRightID, rwavesRight);

		} else {
			if (moved) {
				// location
				if (location.x != m_xWME.GetValue()) {
					Update(m_xWME, location.x);
				}
				
				if (location.y != m_yWME.GetValue()) {
					Update(m_yWME, location.y);
				}
				
				// charger detection
				String energyRecharger = onEnergyCharger ? Names.kYes : Names.kNo;
				Update(m_EnergyRechargerWME, energyRecharger);

				String healthRecharger = onHealthCharger ? Names.kYes : Names.kNo;
				Update(m_HealthRechargerWME, healthRecharger);
			}
			
			boolean rotated = !m_DirectionWME.GetValue().equalsIgnoreCase(facingString);
			if (rotated) {
				// facing
				Update(m_DirectionWME, facingString);
			}

			// charger detection
			if (m_EnergyWME.GetValue() != energy) {
				Update(m_EnergyWME, energy);
			}
			if (m_HealthWME.GetValue() != health) {
				Update(m_HealthWME, health);
			}			

			// shield status
			if (!m_ShieldStatusWME.GetValue().equalsIgnoreCase(shieldStatus)) {
				Update(m_ShieldStatusWME, shieldStatus);
			}
			
			// blocked sensor
			if (attemptedMove || rotated || !m_BlockedForwardWME.GetValue().equalsIgnoreCase(blockedForward)) {
				Update(m_BlockedForwardWME, blockedForward);
			}
			if (attemptedMove || rotated || !m_BlockedBackwardWME.GetValue().equalsIgnoreCase(blockedBackward)) {
				Update(m_BlockedBackwardWME, blockedBackward);
			}
			if (attemptedMove || rotated || !m_BlockedLeftWME.GetValue().equalsIgnoreCase(blockedLeft)) {
				Update(m_BlockedLeftWME, blockedLeft);
			}
			if (attemptedMove || rotated || !m_BlockedRightWME.GetValue().equalsIgnoreCase(blockedRight)) {
				Update(m_BlockedRightWME, blockedRight);
			}

			// scores
			if (playersChanged) {
				initScoreWMEs();
			}
			Iterator<Player> playerIter = world.getPlayers().iterator();
			while (playerIter.hasNext()) {
				Player player = playerIter.next();
				if (player.pointsChanged()) {
					if (player.getColor().equals("blue")) {
						Update(m_BlueScore, player.getPoints());
					} else if (player.getColor().equals("red")) {
						Update(m_RedScore, player.getPoints());
					} else if (player.getColor().equals("yellow")) {
						Update(m_YellowScore, player.getPoints());
					} else if (player.getColor().equals("green")) {
						Update(m_GreenScore, player.getPoints());
					} else if (player.getColor().equals("purple")) {
						Update(m_PurpleScore, player.getPoints());
					} else if (player.getColor().equals("orange")) {
						Update(m_OrangeScore, player.getPoints());
					} else if (player.getColor().equals("black")) {
						Update(m_BlackScore, player.getPoints());
					}
					player.resetPointsChanged();
				}
			}

			// incoming sensor
			if (!m_IncomingForwardWME.GetValue().equalsIgnoreCase(incomingForward)) {
				Update(m_IncomingForwardWME, incomingForward);
			}
			if (!m_IncomingBackwardWME.GetValue().equalsIgnoreCase(incomingBackward)) {
				Update(m_IncomingBackwardWME, incomingBackward);
			}
			if (!m_IncomingLeftWME.GetValue().equalsIgnoreCase(incomingLeft)) {
				Update(m_IncomingLeftWME, incomingLeft);
			}
			if (!m_IncomingRightWME.GetValue().equalsIgnoreCase(incomingRight)) {
				Update(m_IncomingRightWME, incomingRight);
			}

			// smell sensor
			if (!m_SmellColorWME.GetValue().equalsIgnoreCase(smellColorString)) {
				Update(m_SmellColorWME, smellColorString);
			}
			if (smellColor == null) {
				if (m_SmellDistanceWME != null) {
					DestroyWME(m_SmellDistanceWME);
					m_SmellDistanceWME = null;
				}
				if (m_SmellDistanceStringWME == null) {
					m_SmellDistanceStringWME = CreateStringWME(m_SmellWME, Names.kDistanceID, Names.kNone);
				}
			} else {
				if (m_SmellDistanceWME == null) {
					m_SmellDistanceWME = CreateIntWME(m_SmellWME, Names.kDistanceID, smellDistance);
				} else {
					if (m_SmellDistanceWME.GetValue() != smellDistance) {
						Update(m_SmellDistanceWME, smellDistance);
					}
				}
				if (m_SmellDistanceStringWME != null) {
					DestroyWME(m_SmellDistanceStringWME);
					m_SmellDistanceStringWME = null;
				}
			}

			// sound sensor
			if (moved || rotated || !m_SoundWME.GetValue().equalsIgnoreCase(soundString)) {
				Update(m_SoundWME, soundString);
			}

			// missile quantity indicator
			if (m_MissilesWME.GetValue() != missiles) {
				Update(m_MissilesWME, missiles);
			}

			// clock (world count)
			Update(m_ClockWME, worldCount);

			// resurrect sensor
			if (worldCount != m_ResurrectFrame) {
				if (!m_ResurrectWME.GetValue().equalsIgnoreCase(Names.kNo)) {
					Update(m_ResurrectWME, Names.kNo);
				}
			}

			// radar sensors
			if (!m_RadarStatusWME.GetValue().equalsIgnoreCase(radarStatus)) {
				Update(m_RadarStatusWME, radarStatus);
			}
			if (radarSwitch) {
				if (m_RadarWME == null) {
					m_RadarWME = agent.CreateIdWME(m_InputLink, Names.kRadarID);
					generateNewRadar();
				} else {
					updateRadar(moved || rotated);
				}
			} else {
				if (m_RadarWME != null) {
					clearRadar();
					DestroyWME(m_RadarWME);
					m_RadarWME = null;
				}
			}
			if (m_RadarDistanceWME.GetValue() != observedPower) {
				Update(m_RadarDistanceWME, observedPower);
			}
			if (m_RadarSettingWME.GetValue() != radarPower) {
				Update(m_RadarSettingWME, radarPower);
			}

			// random indicator
			Update(m_RandomWME, random);

			// rwaves sensor
			if (!m_RWavesForwardWME.GetValue().equalsIgnoreCase(rwavesForward)) {
				Update(m_RWavesForwardWME, rwavesForward);
			}
			if (!m_RWavesBackwardWME.GetValue().equalsIgnoreCase(rwavesBackward)) {
				Update(m_RWavesBackwardWME, rwavesBackward);
			}
			if (!m_RWavesLeftWME.GetValue().equalsIgnoreCase(rwavesLeft)) {
				Update(m_RWavesLeftWME, rwavesLeft);
			}
			if (!m_RWavesRightWME.GetValue().equalsIgnoreCase(rwavesRight)) {
				Update(m_RWavesRightWME, rwavesRight);
			}
		}
		
		m_Reset = false;
		agent.Commit();
	}
	
	private void generateNewRadar() {
		int height;
		for (height = 0; height < Soar2D.config.kRadarHeight; ++height) {
			boolean done = false;
			for (int width = 0; width < Soar2D.config.kRadarWidth; ++width) {
				// Always skip self, this screws up the tanks.
				if (width == 1 && height == 0) {
					continue;
				}
				if (radar[width][height] == null) {
					// if center is null, we're done
					if (width == 1) {
						done = true;
						break;
					}
				} else {
					// Create a new WME
					radarCellIDs[width][height] = agent.CreateIdWME(m_RadarWME, getCellID(radar[width][height]));
					CreateIntWME(radarCellIDs[width][height], Names.kDistanceID, height);
					CreateStringWME(radarCellIDs[width][height], Names.kPositionID, getPositionID(width));
					if (radar[width][height].player != null) {
						radarColors[width][height] = CreateStringWME(radarCellIDs[width][height], Names.kColorID, radar[width][height].player.getColor());
					}
				}
			}
			if (done == true) {
				break;
			}
		}
		assert (height - 1) == this.observedPower;
	}
	
	private void updateRadar(boolean movedOrRotated) {
		for (int width = 0; width < Soar2D.config.kRadarWidth; ++width) {
			for (int height = 0; height < Soar2D.config.kRadarHeight; ++height) {
				// Always skip self, this screws up the tanks.
				if (width == 1 && height == 0) {
					continue;
				}
				if (radar[width][height] == null || (height > observedPower) || ((height == observedPower) && (width != 1))) {
					// Unconditionally delete the WME
					if (radarCellIDs[width][height] != null) {
						DestroyWME(radarCellIDs[width][height]);
						radarCellIDs[width][height] = null;
						radarColors[width][height] = null;
					}
					
				} else {
					
					if (radarCellIDs[width][height] == null) {
						radarCellIDs[width][height] = agent.CreateIdWME(m_RadarWME, getCellID(radar[width][height]));
						CreateIntWME(radarCellIDs[width][height], Names.kDistanceID, height);
						CreateStringWME(radarCellIDs[width][height], Names.kPositionID, getPositionID(width));
						if (radar[width][height].player != null) {
							radarColors[width][height] = CreateStringWME(radarCellIDs[width][height], Names.kColorID, radar[width][height].player.getColor());
						}
					} else {
						boolean changed = !radarCellIDs[width][height].GetAttribute().equals(getCellID(radar[width][height]));

						// Update if relevant change
						if (movedOrRotated || changed) {
							DestroyWME(radarCellIDs[width][height]);
							radarCellIDs[width][height] = agent.CreateIdWME(m_RadarWME, getCellID(radar[width][height]));
							CreateIntWME(radarCellIDs[width][height], Names.kDistanceID, height);
							CreateStringWME(radarCellIDs[width][height], Names.kPositionID, getPositionID(width));
							if (radar[width][height].player != null) {
								radarColors[width][height] = CreateStringWME(radarCellIDs[width][height], Names.kColorID, radar[width][height].player.getColor());
							}
						}
					}
				}
			}
		}
	}

	private void clearRadar() {
		for (int width = 0; width < Soar2D.config.kRadarWidth; ++width) {
			for (int height = 0; height < Soar2D.config.kRadarHeight; ++height) {
				radarCellIDs[width][height] = null;
				radarColors[width][height] = null;
			}
		}
	}
	
	private String getCellID(RadarCell cell) {
		if (cell.player != null) {
			return Names.kTankID;
		}
		if (cell.obstacle) {
			return Names.kObstacleID;
		}
		if (cell.energy) {
			return Names.kEnergyID;
		}
		if (cell.health) {
			return Names.kHealthID;
		}
		if (cell.missiles) {
			return Names.kMissilesID;
		}
		return Names.kOpenID;
	}
	
	public String getPositionID(int i) {
		switch (i) {
		case 0:
			return Names.kLeftID;
		default:
		case 1:
			return Names.kCenterID;
		case 2:
			return Names.kRightID;
		}
	}

	public void shutdown() {
		assert agent != null;
		if (shutdownCommands != null) { 
			Iterator<String> iter = shutdownCommands.iterator();
			while(iter.hasNext()) {
				String command = iter.next();
				String result = getName() + ": result: " + agent.ExecuteCommandLine(command, true);
				Soar2D.logger.info(getName() + ": shutdown command: " + command);
				if (agent.HadError()) {
					Soar2D.control.severeError(result);
				} else {
					Soar2D.logger.info(getName() + ": result: " + result);
				}
			}
		}

		clearWMEs();
	}
}
