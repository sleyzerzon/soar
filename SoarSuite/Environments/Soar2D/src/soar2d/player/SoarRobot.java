package soar2d.player;

import java.awt.geom.Point2D;
import java.awt.Point;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.logging.Level;
import java.lang.Math;

import sml.Agent;
import sml.FloatElement;
import sml.Identifier;
import sml.IntElement;
import sml.StringElement;
import soar2d.Direction;
import soar2d.Names;
import soar2d.Simulation;
import soar2d.Soar2D;
import soar2d.World;
import soar2d.world.CellObject;
import soar2d.world.GridMap;
import soar2d.world.GridMap.Barrier;

class SelfInputLink {
	SoarRobot robot;
	Identifier self;
	Identifier angle;
	FloatElement yaw;
	IntElement area;
	Identifier areaDescription;
	ArrayList<BarrierInputLink> walls = new ArrayList<BarrierInputLink>();
	ArrayList<GatewayInputLink> gateways = new ArrayList<GatewayInputLink>();
	private HashMap<Integer, ObjectInputLink> objects = new HashMap<Integer, ObjectInputLink>();
	Identifier collision;
	StringElement collisionX;
	StringElement collisionY;
	IntElement cycle;
	IntElement score;
	Identifier position;
	FloatElement x;
	FloatElement y;
	IntElement row;
	IntElement col;
	FloatElement random;
	FloatElement time;
	Identifier velocity;
	FloatElement speed;
	FloatElement dx;
	FloatElement dy;
	Identifier carry;
	StringElement carryType;
	IntElement carryId;
	
	SelfInputLink(SoarRobot robot) {
		this.robot = robot;
	}
	
	void initialize() {
		Identifier il = robot.agent.GetInputLink();

		assert il != null;
		assert self == null;

		self = robot.agent.CreateIdWME(il, "self");
		angle = robot.agent.CreateIdWME(self, "angle");
		{
			yaw = robot.agent.CreateFloatWME(angle, "yaw", robot.getHeadingRadians());
		}
		area = robot.agent.CreateIntWME(self, "area", -1);
		collision = robot.agent.CreateIdWME(self, "collision");
		{
			collisionX = robot.agent.CreateStringWME(collision, "x", "false");
			collisionY = robot.agent.CreateStringWME(collision, "y", "false");
		}
		cycle = robot.agent.CreateIntWME(self, "cycle", 0);
		score = robot.agent.CreateIntWME(self, "score", 0);
		position = robot.agent.CreateIdWME(self, "position");
		{
			x = robot.agent.CreateFloatWME(position, "x", 0);
			y = robot.agent.CreateFloatWME(position, "y", 0);
			row = robot.agent.CreateIntWME(position, "row", 0);
			col = robot.agent.CreateIntWME(position, "col", 0);
		}
		random = robot.agent.CreateFloatWME(self, "random", robot.random);
		time = robot.agent.CreateFloatWME(self, "time", 0);
		velocity = robot.agent.CreateIdWME(self, "velocity");
		{
			speed = robot.agent.CreateFloatWME(velocity, "speed", 0);
			dx = robot.agent.CreateFloatWME(velocity, "dx", 0);
			dy = robot.agent.CreateFloatWME(velocity, "dy", 0);
		}
	}
	
	void createAreaDescription() {
		assert areaDescription == null;
		areaDescription = robot.agent.CreateIdWME(self, "area-description");
	}
	
	Identifier createWallId() {
		assert areaDescription != null;
		return robot.agent.CreateIdWME(areaDescription, "wall");
	}
	
	Identifier createGatewayId() {
		assert areaDescription != null;
		return robot.agent.CreateIdWME(areaDescription, "gateway");
	}
	
	void addWall(BarrierInputLink wall) {
		walls.add(wall);
	}
	
	void addGateway(GatewayInputLink gateway) {
		gateways.add(gateway);
	}
	
	void addOrUpdateObject(GridMap.BookObjectInfo objectInfo, World world, double angleOffDouble) {
		ObjectInputLink oIL = objects.get(objectInfo.object.getIntProperty("object-id"));

		double dx = objectInfo.floatLocation.x - world.getFloatLocation(robot).x;
		dx *= dx;
		double dy = objectInfo.floatLocation.y - world.getFloatLocation(robot).y;
		dy *= dy;
		double range = Math.sqrt(dx + dy);
		
		if (oIL == null) {
			// create new object
			Identifier parent = robot.agent.CreateIdWME(robot.agent.GetInputLink(), "object");
			oIL = new ObjectInputLink(robot, parent);
			oIL.initialize(objectInfo, world, range, angleOffDouble);
			objects.put(objectInfo.object.getIntProperty("object-id"), oIL);
		
		} else {
			if (oIL.area.GetValue() != objectInfo.area) {
				robot.agent.Update(oIL.area, objectInfo.area);
			}
			if (oIL.row.GetValue() != objectInfo.location.y) {
				robot.agent.Update(oIL.row, objectInfo.location.y);
			}
			if (oIL.col.GetValue() != objectInfo.location.x) {
				robot.agent.Update(oIL.col, objectInfo.location.x);
			}
			if (oIL.x.GetValue() != objectInfo.floatLocation.x) {
				robot.agent.Update(oIL.x, objectInfo.location.x);
			}
			if (oIL.y.GetValue() != objectInfo.floatLocation.y) {
				robot.agent.Update(oIL.y, objectInfo.location.y);
			}
			if (oIL.range.GetValue() != range) {
				robot.agent.Update(oIL.range, range);
			}
			if (oIL.angleOff.GetValue() != angleOffDouble) {
				robot.agent.Update(oIL.angleOff, angleOffDouble);
			}
			oIL.touch(world.getWorldCount());
		}
	}
	
	void purge(int cycle) {
		Iterator<ObjectInputLink> iter = objects.values().iterator();
		while (iter.hasNext()) {
			ObjectInputLink oIL = iter.next();
			if (oIL.cycleTouched < cycle) {
				if (oIL.cycleTouched > cycle - 3) {
					oIL.makeInvisible();
				} else {
					robot.agent.DestroyWME(oIL.parent);
					iter.remove();
				}
			}
		}
	}
	
	ObjectInputLink getOIL(int id) {
		return objects.get(id);
	}
	
	void destroyAreaDescription() {
		walls = new ArrayList<BarrierInputLink>();
		gateways = new ArrayList<GatewayInputLink>();

		if (areaDescription == null) {
			return;
		}
		robot.agent.DestroyWME(areaDescription);
		areaDescription = null;
		
	}
	
	void destroy() {
		assert self != null;
		robot.agent.DestroyWME(self);
		self = areaDescription = carry = null;
		destroyAreaDescription();
	}
	
	void carry(CellObject object) {
		assert carry == null;
		carry = robot.agent.CreateIdWME(self, "carry");
		carryType = robot.agent.CreateStringWME(carry, "type", object.getProperty("id"));
		carryId = robot.agent.CreateIntWME(carry, "id", object.getIntProperty("object-id"));
	}
	
	void drop() {
		assert carry != null;
		robot.agent.DestroyWME(carry);
		carry = null;
	}
}

class BarrierInputLink {
	SoarRobot robot;
	IntElement id;
	Identifier parent, left, right, center;
	IntElement leftRow, leftCol, rightRow, rightCol;
	FloatElement x, y, angleOff;
	StringElement direction;
	
	Point2D.Double centerpoint;
	
	BarrierInputLink(SoarRobot robot, Identifier parent) {
		this.robot = robot;
		this.parent = parent;
	}
	
	void initialize(Barrier barrier, World world) {
		id = robot.agent.CreateIntWME(parent, "id", barrier.id);
		left = robot.agent.CreateIdWME(parent, "left");
		{
			leftRow = robot.agent.CreateIntWME(left, "row", barrier.left.y);
			leftCol = robot.agent.CreateIntWME(left, "col", barrier.left.x);
		}
		right = robot.agent.CreateIdWME(parent, "right");
		{
			rightRow = robot.agent.CreateIntWME(right, "row", barrier.right.y);
			rightCol = robot.agent.CreateIntWME(right, "col", barrier.right.x);
		}
		center = robot.agent.CreateIdWME(parent, "center");
		{
			centerpoint = barrier.centerpoint();
			x = robot.agent.CreateFloatWME(center, "x", centerpoint.x);
			y = robot.agent.CreateFloatWME(center, "y", centerpoint.y);
			angleOff = robot.agent.CreateFloatWME(center, "angle-off", world.angleOff(robot, centerpoint));
		}
		direction = robot.agent.CreateStringWME(parent, "direction", Direction.stringOf[barrier.direction]);
	}
}

class GatewayInputLink extends BarrierInputLink {
	ArrayList<IntElement> toList = new ArrayList<IntElement>();
	
	GatewayInputLink(SoarRobot robot, Identifier parent) {
		super(robot, parent);
	}
	
	void addDest(int id) {
		IntElement dest = robot.agent.CreateIntWME(parent, "to", id);
		toList.add(dest);
	}
}

class ObjectInputLink {
	SoarRobot robot;
	Identifier parent;
	
	IntElement area;
	StringElement type;
	Identifier position;
	FloatElement angleOff;
	FloatElement x, y;
	IntElement row, col;
	StringElement visible;
	FloatElement range;
	IntElement id;
	
	int cycleTouched;
	
	ObjectInputLink(SoarRobot robot, Identifier parent) {
		this.robot = robot;
		this.parent = parent;
	}
	
	void initialize(GridMap.BookObjectInfo info, World world, double range, double angleOffDouble) {
		this.id = robot.agent.CreateIntWME(parent, "id", info.object.getIntProperty("object-id"));
		this.type = robot.agent.CreateStringWME(parent, "type", info.object.getProperty("id"));
		this.area = robot.agent.CreateIntWME(parent, "area", info.area);
		this.position = robot.agent.CreateIdWME(parent, "position");
		{
			this.col = robot.agent.CreateIntWME(position, "col", info.location.x);
			this.row = robot.agent.CreateIntWME(position, "row", info.location.y);
			this.x = robot.agent.CreateFloatWME(position, "x", info.floatLocation.x);
			this.y = robot.agent.CreateFloatWME(position, "y", info.floatLocation.y);
			angleOff = robot.agent.CreateFloatWME(position, "angle-off", angleOffDouble);
		}
		this.range = robot.agent.CreateFloatWME(parent, "range", range);
		this.visible = robot.agent.CreateStringWME(parent, "visible", "yes");
		
		touch(world.getWorldCount());
	}
	
	void touch(int cycle) {
		cycleTouched = cycle;
		if (visible.GetValue().equals("no")) {
			robot.agent.Update(visible, "yes");
		}
	}
	
	void makeInvisible() {
		if (visible.GetValue().equals("yes")) {
			robot.agent.Update(visible, "no");
		}
	}
}

public class SoarRobot extends Robot {
	Agent agent;	// the soar agent
	float random;	// a random number, guaranteed to change every frame
	private boolean fragged = true;
	
	SelfInputLink selfIL;
	
	private ArrayList<String> shutdownCommands;	// soar commands to run before this agent is destroyed
	

	private int locationId = -1;
	
	/**
	 * @param agent a valid soar agent
	 * @param playerConfig the rest of the player config
	 */
	public SoarRobot(Agent agent, PlayerConfig playerConfig) {
		super(playerConfig);
		this.agent = agent;
		this.shutdownCommands = playerConfig.getShutdownCommands();
		
		agent.SetBlinkIfNoChange(false);

		previousLocation = new java.awt.Point(-1, -1);
		
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
	
	public void update(World world, java.awt.Point location) {
		
		// check to see if we've moved
		super.update(world, location);
		
		// if we've been fragged, set move to true
		if (fragged) {
			moved = true;
		}
		
		GridMap map = world.getMap();
		
		// if we moved
		if (moved) {
			// check if we're in a new location
			ArrayList<CellObject> locationObjects = map.getAllWithProperty(location, Names.kPropertyNumber);
			if (locationObjects.size() != 1) {
				logger.warning("Location objects greater or less than 1");
				locationId = -1;
				agent.Update(selfIL.area, locationId);

				// destroy old area information
				if (selfIL.areaDescription != null) {
					selfIL.destroyAreaDescription();
				}

			} else {
				int newLocationId = locationObjects.get(0).getIntProperty(Names.kPropertyNumber);

				if (newLocationId != locationId) {
					locationId = newLocationId;
					agent.Update(selfIL.area, locationId);

					// destroy old area information
					if (selfIL.areaDescription != null) {
						selfIL.destroyAreaDescription();
					}
					
					// create new area information
					ArrayList<Barrier> barrierList = world.getMap().getRoomBarrierList(locationId);
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
							ArrayList<Integer> gatewayDestList = world.getMap().getGatewayDestinationList(barrier.id);
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
					// barrier angle offs
					Iterator<BarrierInputLink> wallIter = selfIL.walls.iterator();
					while (wallIter.hasNext()) {
						BarrierInputLink barrier = wallIter.next();
						agent.Update(barrier.angleOff, world.angleOff(this, barrier.centerpoint));
					}
					
					Iterator<GatewayInputLink> gatewayIter = selfIL.gateways.iterator();
					while (gatewayIter.hasNext()) {
						BarrierInputLink barrier = gatewayIter.next();
						agent.Update(barrier.angleOff, world.angleOff(this, barrier.centerpoint));
					}
				}
			}

			// update the location
			agent.Update(selfIL.col, location.x);
			agent.Update(selfIL.row, location.y);
			
			Point2D.Double floatLocation = world.getFloatLocation(this);
			agent.Update(selfIL.x, floatLocation.x);
			agent.Update(selfIL.y, floatLocation.y);
			
			// heading
			agent.Update(selfIL.yaw, getHeadingRadians());
			
			// update the clock
			agent.Update(selfIL.cycle, world.getWorldCount());
		}
		
		// velocity
		agent.Update(selfIL.speed, Math.sqrt(Math.pow(this.getVelocity().x, 2) + Math.pow(this.getVelocity().y, 2)));
		agent.Update(selfIL.dx, getVelocity().x);
		agent.Update(selfIL.dy, getVelocity().y);
		
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
		
		// time
		agent.Update(selfIL.time, world.getTime());
		
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
				double maxAngleOff = Soar2D.config.getVisionCone() / 2;
				double angleOff = world.angleOff(this, bInfo.floatLocation);
				if (Math.abs(angleOff) <= maxAngleOff) {
					selfIL.addOrUpdateObject(bInfo, world, angleOff);
				} else {
					logger.finer(getName() + ": object " + bInfo.object.getProperty("object-id") + " out of cone");
				}
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
		// if there was no command issued, that is kind of strange
		if (agent.GetNumberCommands() == 0) {
			if (logger.isLoggable(Level.FINER)) logger.finer(getName() + " issued no command.");
			return new MoveInfo();
		}

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
				
				String amountString = commandId.GetParameterValue("amount");
				if (amountString == null) {
					logger.warning(getName() + " rotate-relative command missing amount parameter");
					commandId.AddStatusError();
					continue;
				}
				
				try {
					move.rotateRelativeAmount = Double.parseDouble(amountString);
				} catch (NumberFormatException e) {
					logger.warning(getName() + " rotate-relative amount parameter improperly formatted");
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
				if (oIL.range.GetValue() > Soar2D.config.getBookCellSize()) {
					logger.warning(getName() + " get command object out of range");
					commandId.AddStatusError();
					continue;
				}
				
				move.get = true;
				move.getLocation = new java.awt.Point(oIL.col.GetValue(), oIL.row.GetValue());
				getCommandId = commandId;
				
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
		locationId = -1;
		moveCommandId = null;
		moveCommandExecutingAdded = false;
		rotateCommandId = null;
		rotateCommandExecutingAdded = false;
		
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
