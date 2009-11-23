package soar2d.player.book;

import java.awt.Point;
import java.awt.geom.Point2D;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.logging.Level;
import java.lang.Math;

import sml.Agent;
import sml.Identifier;
import soar2d.Direction;
import soar2d.Names;
import soar2d.Simulation;
import soar2d.Soar2D;
import soar2d.map.BookMap;
import soar2d.map.CellObject;
import soar2d.map.GridMap;
import soar2d.map.BookMap.Barrier;
import soar2d.player.MoveInfo;
import soar2d.player.Player;
import soar2d.player.PlayerConfig;
import soar2d.world.PlayersManager;
import soar2d.world.World;

public class SoarRobot extends Robot {
	Agent agent;	// the soar agent
	float random;	// a random number, guaranteed to change every frame
	private boolean fragged = true;
	
	SelfInputLink selfIL;
	
	int oldLocationId;
	
	private ArrayList<String> shutdownCommands;	// soar commands to run before this agent is destroyed

	/**
	 * @param agent a valid soar agent
	 * @param playerConfig the rest of the player config
	 */
	public SoarRobot(Agent agent, PlayerConfig playerConfig) {
		super(playerConfig);
		this.agent = agent;
		this.shutdownCommands = playerConfig.getShutdownCommands();
		
		agent.SetBlinkIfNoChange(false);
		oldLocationId = -1;
		random = 0;
		generateNewRandom();

		// create and initialize self input link
		selfIL = new SelfInputLink(this);
		selfIL.initialize();
		
		if (!agent.Commit()) {
			Soar2D.control.severeError("Failed to commit input to Soar agent " + this.getName());
			Soar2D.control.stopSimulation();
		}
	}
	
	/**
	 * create a new random number
	 * make sure it is different from current
	 */
	private void generateNewRandom() {
		float newRandom;
		do {
			newRandom = Simulation.random.nextFloat();
		} while (this.random == newRandom);
		this.random = newRandom;
	}
	
	public void carry(CellObject object) {
		selfIL.carry(object);
		super.carry(object);
	}
	
	public CellObject drop() {
		selfIL.drop();
		return super.drop();
	}
	
	public void update(java.awt.Point location) {
		World world = Soar2D.simulation.world;

		PlayersManager players = world.getPlayers();

		// check to see if we've moved
		super.update(location);
		
		// if we've been fragged, set move to true
		if (fragged) {
			moved = true;
		}

		BookMap map = (BookMap)world.getMap();
		
		// if we moved
		if (moved) {
			// check if we're in a new location
			if (oldLocationId != locationId) {
				oldLocationId = locationId;
				agent.Update(selfIL.area, locationId);

				// destroy old area information
				if (selfIL.areaDescription != null) {
					selfIL.destroyAreaDescription();
				}
				
				// create new area information
				ArrayList<Barrier> barrierList = map.getRoomBarrierList(locationId);
				assert barrierList != null;
				assert barrierList.size() > 0;
				
				// this is, in fact, a room
				selfIL.createAreaDescription();
				
				Iterator<Barrier> iter = barrierList.iterator();
				while(iter.hasNext()) {
					Barrier barrier = iter.next();
					if (barrier.gateway) {
						// gateway
						GatewayInputLink gateway = new GatewayInputLink(this, selfIL.createGatewayId());
						gateway.initialize(barrier, world);
						
						// add destinations
						ArrayList<Integer> gatewayDestList = map.getGatewayDestinationList(barrier.id);
						Iterator<Integer> destIter = gatewayDestList.iterator();
						while(destIter.hasNext()) {
							gateway.addDest(destIter.next().intValue());
						}
						
						selfIL.addGateway(gateway);
						
					} else {
						// wall
						BarrierInputLink wall = new BarrierInputLink(this, selfIL.createWallId());
						wall.initialize(barrier, world);
						selfIL.addWall(wall);
					}
				}
			} else {
				// barrier angle offs and range
				Iterator<BarrierInputLink> wallIter = selfIL.wallsIL.iterator();
				while (wallIter.hasNext()) {
					BarrierInputLink barrier = wallIter.next();
					agent.Update(barrier.yaw, players.angleOff(this, barrier.centerpoint));
				}
				
				Iterator<GatewayInputLink> gatewayIter = selfIL.gatewaysIL.iterator();
				while (gatewayIter.hasNext()) {
					GatewayInputLink gateway = gatewayIter.next();
					
					agent.Update(gateway.yaw, players.angleOff(this, gateway.centerpoint));
					double dx = gateway.centerpoint.x - players.getFloatLocation(this).x;
					dx *= dx;
					double dy = gateway.centerpoint.y - players.getFloatLocation(this).y;
					dy *= dy;
					double r = Math.sqrt(dx + dy);
					
					agent.Update(gateway.range, r);

				}
			}

			// update the location
			agent.Update(selfIL.col, location.x);
			agent.Update(selfIL.row, location.y);
			
			if (Soar2D.bConfig.getContinuous()) {
				Point2D.Double floatLocation = players.getFloatLocation(this);
				agent.Update(selfIL.x, floatLocation.x);
				agent.Update(selfIL.y, floatLocation.y);
			}
			
			// heading
			agent.Update(selfIL.yaw, getHeadingRadians());
			
			// update the clock
			agent.Update(selfIL.cycle, world.getWorldCount());
		}
		
		if (Soar2D.bConfig.getContinuous()) {
			// velocity
			agent.Update(selfIL.speed, Math.sqrt((getVelocity().x * getVelocity().x) + (getVelocity().y * getVelocity().y)));
			agent.Update(selfIL.dx, getVelocity().x);
			agent.Update(selfIL.dy, getVelocity().y);
			agent.Update(selfIL.rotation, this.getRotationSpeed());

			// collisions
			if (collisionX) {
				agent.Update(selfIL.collisionX, "true");
			} else {
				agent.Update(selfIL.collisionX, "false");
			}
			if (collisionY) {
				agent.Update(selfIL.collisionY, "true");
			} else {
				agent.Update(selfIL.collisionY, "false");
			}
		} else {
			// facing
			agent.Update(selfIL.direction, Direction.stringOf[getFacingInt()]);
			
			// blocked
			int blocked = world.getMap().getBlocked(location);
			String blockedForward = ((blocked & Direction.indicators[getFacingInt()]) > 0) ? "true" : "false";
			String blockedBackward = ((blocked & Direction.indicators[Direction.backwardOf[getFacingInt()]]) > 0) ? "true" : "false";
			String blockedLeft = ((blocked & Direction.indicators[Direction.leftOf[getFacingInt()]]) > 0) ? "true" : "false";
			String blockedRight = ((blocked & Direction.indicators[Direction.rightOf[getFacingInt()]]) > 0) ? "true" : "false";
			agent.Update(selfIL.blockedForward, blockedForward);
			agent.Update(selfIL.blockedBackward, blockedBackward);
			agent.Update(selfIL.blockedLeft, blockedLeft);
			agent.Update(selfIL.blockedRight, blockedRight);
		}
		
		// time
		agent.Update(selfIL.time, Soar2D.control.getTotalTime());
		
		// update the random no matter what
		float oldrandom = random;
		do {
			random = Simulation.random.nextFloat();
		} while (random == oldrandom);
		agent.Update(selfIL.random, random);
		
		// objects
		HashSet<CellObject> bookObjects = map.getBookObjects();
		Iterator<CellObject> bookObjectIter = bookObjects.iterator();
		while (bookObjectIter.hasNext()) {
			CellObject bObj = bookObjectIter.next();
			GridMap.BookObjectInfo bInfo = map.getBookObjectInfo(bObj);
			if (bInfo.area == locationId) {
				double maxAngleOff = Soar2D.bConfig.getVisionCone() / 2;
				double angleOff = players.angleOff(this, bInfo.floatLocation);
				if (Math.abs(angleOff) <= maxAngleOff) {
					selfIL.addOrUpdateObject(bInfo, world, angleOff);
					logger.finer(getName() + ": object " + bInfo.object.getProperty("object-id") + " added to input link");
				} else {
					logger.finer(getName() + ": object " + bInfo.object.getProperty("object-id") + " out of cone");
				}
			}
		}
		
		// players
		if (players.numberOfPlayers() > 1) {
			Iterator<Player> playersIter = players.iterator();
			while (playersIter.hasNext()) {
				Player player = playersIter.next();
				if (this.equals(player)) {
					continue;
				}
				selfIL.addOrUpdatePlayer(player, world, players.angleOff(this, player));
			}
		}
		
		selfIL.purge(world.getWorldCount());
		
		// Add status executing to any commands that need it
		if (moveCommandId != null) {
			if (!moveCommandExecutingAdded) {
				agent.CreateStringWME(moveCommandId, "status", "executing");
				moveCommandExecutingAdded = true;
			}
		}
		if (rotateCommandId != null) {
			if (!rotateCommandExecutingAdded) {
				agent.CreateStringWME(rotateCommandId, "status", "executing");
				rotateCommandExecutingAdded = true;
			}
		}
		assert getCommandId == null;
		assert dropCommandId == null;
		
		// commit everything
		if (!agent.Commit()) {
			Soar2D.control.severeError("Failed to commit input to Soar agent " + this.getName());
			Soar2D.control.stopSimulation();
		}
	}
	
	Identifier moveCommandId = null;
	boolean moveCommandExecutingAdded = false;
	Identifier rotateCommandId = null;
	boolean rotateCommandExecutingAdded = false;
	Identifier getCommandId = null;
	Identifier dropCommandId = null;
	
	/* (non-Javadoc)
	 * @see soar2d.player.Eater#getMove()
	 */
	public MoveInfo getMove() {
		if (Soar2D.config.getForceHuman()) {
			return super.getMove();
		}

		// if there was no command issued, that is kind of strange
		if (agent.GetNumberCommands() == 0) {
			if (logger.isLoggable(Level.FINER)) logger.finer(getName() + " issued no command.");
			return new MoveInfo();
		}

		if (Soar2D.bConfig.getContinuous()) 
			return getMoveContinuous();
		return getMoveDiscrete();
	}
	
	private MoveInfo getMoveDiscrete() {
		// go through the commands
		MoveInfo move = new MoveInfo();
		for (int i = 0; i < agent.GetNumberCommands(); ++i) {
			Identifier commandId = agent.GetCommand(i);
			String commandName = commandId.GetAttribute();
			
			if (commandName.equalsIgnoreCase(Names.kMoveID)) {
				if (move.move) {
					logger.warning(getName() + " multiple move commands issued");
					commandId.AddStatusError();
					continue;
				}
				
				String direction = commandId.GetParameterValue(Names.kDirectionID);
				
				if (direction == null) {
					logger.warning(getName() + " move command missing direction parameter");
					commandId.AddStatusError();
					continue;
				}
				
				if (direction.equalsIgnoreCase(Names.kForwardID)) {
					move.forward = true;
					
				} else if (direction.equalsIgnoreCase(Names.kBackwardID)) {
					move.backward = true;
				
				} else {
					logger.warning(getName() + "unrecognized move direction: " + direction);
					commandId.AddStatusError();
					continue;
				}

				move.move = true;
				commandId.AddStatusComplete();

			} else if (commandName.equalsIgnoreCase(Names.kRotateID)) {
				if (move.rotate) {
					logger.warning(getName() + " multiple rotate commands issued");
					commandId.AddStatusError();
					continue;
				}
				
				String direction = commandId.GetParameterValue(Names.kDirectionID);
				
				if (direction == null) {
					logger.warning(getName() + " rotate command missing direction parameter");
					commandId.AddStatusError();
					continue;
				}
				
				if (direction.equalsIgnoreCase(Names.kLeftID)) {
					move.rotateDirection = Names.kRotateLeft;
					
				} else if (direction.equalsIgnoreCase(Names.kRightID)) {
					move.rotateDirection = Names.kRotateRight;
				
				} else {
					logger.warning(getName() + "unrecognized rotate direction: " + direction);
					commandId.AddStatusError();
					continue;
					
				}

				move.rotate = true;
				commandId.AddStatusComplete();

			} else if (commandName.equalsIgnoreCase(Names.kStopSimID)) {
				if (move.stopSim) {
					logger.warning(getName() + " multiple stop-sim commands issued");
					commandId.AddStatusError();
					continue;
				}
				move.stopSim = true;
				commandId.AddStatusComplete();
				
			} else if (commandName.equalsIgnoreCase(Names.kGetID)) {
				if (move.get) {
					logger.warning(getName() + " multiple get commands issued");
					commandId.AddStatusError();
					continue;
				}

				if (move.drop) {
					logger.warning(getName() + " get: both get and drop simultaneously issued");
					commandId.AddStatusError();
					continue;
				}
				
				String idString = commandId.GetParameterValue("id");
				if (idString == null) {
					logger.warning(getName() + " get command missing id parameter");
					commandId.AddStatusError();
					continue;
				}
				try {
					move.getId = Integer.parseInt(idString);
				} catch (NumberFormatException e) {
					logger.warning(getName() + " get command id parameter improperly formatted");
					commandId.AddStatusError();
					continue;
				}
				
				ObjectInputLink oIL = selfIL.getOIL(move.getId);
				if (oIL == null) {
					logger.warning(getName() + " get command invalid id " + move.getId);
					commandId.AddStatusError();
					continue;
				}
				if (oIL.row.GetValue() != selfIL.row.GetValue() || oIL.col.GetValue() != selfIL.col.GetValue())
				if (oIL.range.GetValue() > Soar2D.bConfig.getBookCellSize()) {
					logger.warning(getName() + " get command object not in same cell");
					commandId.AddStatusError();
					continue;
				}
				
				move.get = true;
				getCommandId = commandId;
				move.getLocation = new java.awt.Point(oIL.col.GetValue(), oIL.row.GetValue());
				
			} else if (commandName.equalsIgnoreCase(Names.kDropID)) {
				if (move.drop) {
					logger.warning(getName() + " multiple drop commands issued");
					commandId.AddStatusError();
					continue;
				}

				if (move.get) {
					logger.warning(getName() + " drop: both drop and get simultaneously issued");
					commandId.AddStatusError();
					continue;
				}
				
				String idString = commandId.GetParameterValue("id");
				if (idString == null) {
					logger.warning(getName() + " drop command missing id parameter");
					commandId.AddStatusError();
					continue;
				}
				try {
					move.dropId = Integer.parseInt(idString);
				} catch (NumberFormatException e) {
					logger.warning(getName() + " drop command id parameter improperly formatted");
					commandId.AddStatusError();
					continue;
				}
				
				move.drop = true;
				commandId.AddStatusComplete();
				
			} else if (commandName.equalsIgnoreCase("communicate")) {
				MoveInfo.Communication comm = move.new Communication();
				String toString = commandId.GetParameterValue("to");
				if (toString == null) {
					logger.warning(getName() + " communicate command missing to parameter");
					commandId.AddStatusError();
					continue;
				}
				comm.to = toString;
				
				String messageString = commandId.GetParameterValue("message");
				if (messageString == null) {
					logger.warning(getName() + " communicate command missing message parameter");
					commandId.AddStatusError();
					continue;
				}
				comm.message = messageString;
				
				move.messages.add(comm);
				commandId.AddStatusComplete();
				
			} else {
				logger.warning("Unknown command: " + commandName);
				commandId.AddStatusError();
			}
		}
		agent.ClearOutputLinkChanges();
		if (!agent.Commit()) {
			Soar2D.control.severeError("Failed to commit input to Soar agent " + this.getName());
			Soar2D.control.stopSimulation();
		}
		return move;
	}
	
	private MoveInfo getMoveContinuous() {
				
		// go through the commands
		MoveInfo move = new MoveInfo();
		for (int i = 0; i < agent.GetNumberCommands(); ++i) {
			Identifier commandId = agent.GetCommand(i);
			String commandName = commandId.GetAttribute();
			
			if (commandName.equalsIgnoreCase(Names.kMoveID)) {
				if (moveCommandId != null) {
					if (moveCommandId.GetTimeTag() != commandId.GetTimeTag()) {
						moveCommandId.AddStatusComplete();
						moveCommandId = null;
					}
				}
				if (move.move) {
					logger.warning(getName() + " multiple move commands issued");
					commandId.AddStatusError();
					continue;
				}
				
				String direction = commandId.GetParameterValue(Names.kDirectionID);
				
				if (direction == null) {
					logger.warning(getName() + " move command missing direction parameter");
					commandId.AddStatusError();
					continue;
				}
				
				if (direction.equalsIgnoreCase(Names.kForwardID)) {
					move.forward = true;
					
				} else if (direction.equalsIgnoreCase(Names.kBackwardID)) {
					move.backward = true;
				
				} else if (direction.equalsIgnoreCase(Names.kStopID)) {
					move.move = true;
					move.forward = true;
					move.backward = true;
					commandId.AddStatusComplete();
					continue;
					
				} else {
					logger.warning(getName() + "unrecognized move direction: " + direction);
					commandId.AddStatusError();
					continue;
				}

				move.move = true;
				moveCommandId = commandId;
				moveCommandExecutingAdded = false;

			} else if (commandName.equalsIgnoreCase(Names.kRotateID)) {
				if (rotateCommandId != null) {
					if (rotateCommandId.GetTimeTag() != commandId.GetTimeTag()) {
						rotateCommandId.AddStatusComplete();
						rotateCommandId = null;
					}
				}
				if (move.rotate) {
					logger.warning(getName() + " multiple rotate commands issued");
					commandId.AddStatusError();
					continue;
				}
				
				String direction = commandId.GetParameterValue(Names.kDirectionID);
				
				if (direction == null) {
					logger.warning(getName() + " rotate command missing direction parameter");
					commandId.AddStatusError();
					continue;
				}
				
				if (direction.equalsIgnoreCase(Names.kLeftID)) {
					move.rotateDirection = Names.kRotateLeft;
					
				} else if (direction.equalsIgnoreCase(Names.kRightID)) {
					move.rotateDirection = Names.kRotateRight;
				
				} else if (direction.equalsIgnoreCase(Names.kStopID)) {
					move.rotate = true;
					move.rotateDirection = Names.kRotateStop;
					commandId.AddStatusComplete();
					continue;
				
				} else {
					logger.warning(getName() + "unrecognized rotate direction: " + direction);
					commandId.AddStatusError();
					continue;
					
				}

				move.rotate = true;
				rotateCommandId = commandId;
				rotateCommandExecutingAdded = false;

			} else if (commandName.equalsIgnoreCase(Names.kStopSimID)) {
				if (move.stopSim) {
					logger.warning(getName() + " multiple stop-sim commands issued");
					commandId.AddStatusError();
					continue;
				}
				move.stopSim = true;
				commandId.AddStatusComplete();
				
			} else if (commandName.equalsIgnoreCase(Names.kRotateAbsoluteID)) {
				if (rotateCommandId != null) {
					if (rotateCommandId.GetTimeTag() != commandId.GetTimeTag()) {
						rotateCommandId.AddStatusComplete();
						rotateCommandId = null;
					}
				}
				if (move.rotateAbsolute) {
					logger.warning(getName() + " multiple rotate-absolute commands issued");
					commandId.AddStatusError();
					continue;
				}
				
				String yawString = commandId.GetParameterValue("yaw");
				if (yawString == null) {
					logger.warning(getName() + " rotate-absolute command missing yaw parameter");
					commandId.AddStatusError();
					continue;
				}
				
				try {
					move.rotateAbsoluteHeading = Double.parseDouble(yawString);
				} catch (NumberFormatException e) {
					logger.warning(getName() + " rotate-absolute yaw parameter improperly formatted");
					commandId.AddStatusError();
					continue;
				}

				move.rotateAbsolute = true;
				rotateCommandId = commandId;
				rotateCommandExecutingAdded = false;
				
			} else if (commandName.equalsIgnoreCase(Names.kRotateRelativeID)) {
				if (rotateCommandId != null) {
					if (rotateCommandId.GetTimeTag() != commandId.GetTimeTag()) {
						rotateCommandId.AddStatusComplete();
						rotateCommandId = null;
					}
				}
				if (move.rotateRelative) {
					logger.warning(getName() + " multiple rotate-relative commands issued");
					commandId.AddStatusError();
					continue;
				}
				
				String amountString = commandId.GetParameterValue("yaw");
				if (amountString == null) {
					logger.warning(getName() + " rotate-relative command missing yaw parameter");
					commandId.AddStatusError();
					continue;
				}
				
				try {
					move.rotateRelativeYaw = Double.parseDouble(amountString);
				} catch (NumberFormatException e) {
					logger.warning(getName() + " rotate-relative yaw parameter improperly formatted");
					commandId.AddStatusError();
					continue;
				}

				move.rotateRelative = true;
				rotateCommandId = commandId;
				rotateCommandExecutingAdded = false;
				
			} else if (commandName.equalsIgnoreCase(Names.kGetID)) {
				if (move.get) {
					logger.warning(getName() + " multiple get commands issued");
					commandId.AddStatusError();
					continue;
				}

				if (move.drop) {
					logger.warning(getName() + " get: both get and drop simultaneously issued");
					commandId.AddStatusError();
					continue;
				}
				
				String idString = commandId.GetParameterValue("id");
				if (idString == null) {
					logger.warning(getName() + " get command missing id parameter");
					commandId.AddStatusError();
					continue;
				}
				try {
					move.getId = Integer.parseInt(idString);
				} catch (NumberFormatException e) {
					logger.warning(getName() + " get command id parameter improperly formatted");
					commandId.AddStatusError();
					continue;
				}
				
				ObjectInputLink oIL = selfIL.getOIL(move.getId);
				if (oIL == null) {
					logger.warning(getName() + " get command invalid id " + move.getId);
					commandId.AddStatusError();
					continue;
				}
				if (oIL.range.GetValue() > Soar2D.bConfig.getBookCellSize()) {
					logger.warning(getName() + " get command object out of range");
					commandId.AddStatusError();
					continue;
				}
				
				move.get = true;
				getCommandId = commandId;
				move.getLocation = new java.awt.Point(oIL.col.GetValue(), oIL.row.GetValue());
				
			} else if (commandName.equalsIgnoreCase(Names.kDropID)) {
				if (move.drop) {
					logger.warning(getName() + " multiple drop commands issued");
					commandId.AddStatusError();
					continue;
				}

				if (move.get) {
					logger.warning(getName() + " drop: both drop and get simultaneously issued");
					commandId.AddStatusError();
					continue;
				}
				
				String idString = commandId.GetParameterValue("id");
				if (idString == null) {
					logger.warning(getName() + " drop command missing id parameter");
					commandId.AddStatusError();
					continue;
				}
				try {
					move.dropId = Integer.parseInt(idString);
				} catch (NumberFormatException e) {
					logger.warning(getName() + " drop command id parameter improperly formatted");
					commandId.AddStatusError();
					continue;
				}
				
				move.drop = true;
				dropCommandId = commandId;
				
			} else if (commandName.equalsIgnoreCase("communicate")) {
				MoveInfo.Communication comm = move.new Communication();
				String toString = commandId.GetParameterValue("to");
				if (toString == null) {
					logger.warning(getName() + " communicate command missing to parameter");
					commandId.AddStatusError();
					continue;
				}
				comm.to = toString;
				
				String messageString = commandId.GetParameterValue("message");
				if (messageString == null) {
					logger.warning(getName() + " communicate command missing message parameter");
					commandId.AddStatusError();
					continue;
				}
				comm.message = messageString;
				
				move.messages.add(comm);
				commandId.AddStatusComplete();
				
			} else {
				logger.warning("Unknown command: " + commandName);
				commandId.AddStatusError();
				continue;
			}
		}
		agent.ClearOutputLinkChanges();
		if (!agent.Commit()) {
			Soar2D.control.severeError("Failed to commit input to Soar agent " + this.getName());
			Soar2D.control.stopSimulation();
		}
		return move;
	}
	
	public void receiveMessage(Player player, String message) {
		selfIL.addMessage(player, message, Soar2D.simulation.world);
	}
	
	public void updateGetStatus(boolean success) {
		if (success) {
			getCommandId.AddStatusComplete();
			getCommandId = null;
			return;
		}
		getCommandId.AddStatusError();
		getCommandId = null;
	}
	
	public void updateDropStatus(boolean success) {
		if (success) {
			dropCommandId.AddStatusComplete();
			dropCommandId = null;
			return;
		}
		dropCommandId.AddStatusError();
		dropCommandId = null;
	}
	
	public void rotateComplete() {
	
		if (rotateCommandId != null) {
			rotateCommandId.AddStatusComplete();
			rotateCommandId = null;
		}
	}
	
	/* (non-Javadoc)
	 * @see soar2d.player.Player#reset()
	 */
	public void reset() {
		super.reset();
		fragged = false;
		moved = true;
		moveCommandId = null;
		moveCommandExecutingAdded = false;
		rotateCommandId = null;
		rotateCommandExecutingAdded = false;
		oldLocationId = -1;

		if (agent == null) {
			return;
		}
		
		selfIL.destroy();

		if (!agent.Commit()) {
			Soar2D.control.severeError("Failed to commit input to Soar agent " + this.getName());
			Soar2D.control.stopSimulation();
		}

		agent.InitSoar();

		selfIL.initialize();
		agent.ClearOutputLinkChanges();
		if (!agent.Commit()) {
			Soar2D.control.severeError("Failed to commit input to Soar agent " + this.getName());
			Soar2D.control.stopSimulation();
		}
		
	}

	public void fragged() {
		fragged = true;
	}

	/* (non-Javadoc)
	 * @see soar2d.player.Eater#shutdown()
	 */
	public void shutdown() {
		assert agent != null;
		if (shutdownCommands != null) { 
			// execute the pre-shutdown commands
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
	}

}