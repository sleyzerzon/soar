package soar2d.players.soar;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;

import org.apache.log4j.Logger;

import sml.Agent;
import sml.FloatElement;
import sml.Identifier;
import sml.IntElement;
import sml.StringElement;
import soar2d.Direction;
import soar2d.Names;
import soar2d.Simulation;
import soar2d.Soar2D;
import soar2d.map.CellObject;
import soar2d.map.KitchenMap;
import soar2d.players.Cook;
import soar2d.players.MoveInfo;
import soar2d.world.World;

public class SoarCook extends Cook {
	private static Logger logger = Logger.getLogger(SoarCook.class);

	private Agent agent;
	private float random;
	
	IntElement x, y, reward;
	StringElement northObject, southObject, eastObject, westObject;
	StringElement northType, southType, eastType, westType, type;
	FloatElement randomWME;
	Identifier self, view, cell;
	
	InputLinkMetadata metadata;

	class ObjectInputLink {
		private Identifier objectIdentifier;
		private StringElement texture, color, smell;
		
		ObjectInputLink(String id, String texture, String color, String smell) {
			assert this.objectIdentifier == null;
			
			objectIdentifier = agent.CreateIdWME(cell, "object");
			
			if (id.startsWith("mixture")) {
				agent.CreateStringWME(objectIdentifier, "id", "mixture");
			} else {
				agent.CreateStringWME(objectIdentifier, "id", id);
			}
			this.texture = agent.CreateStringWME(objectIdentifier, "texture", texture);
			this.color = agent.CreateStringWME(objectIdentifier, "color", color);
			this.smell = agent.CreateStringWME(objectIdentifier, "smell", smell);
		}
		
		void Update(String texture, String color, String smell) {
			agent.Update(this.texture, texture);
			agent.Update(this.color, color);
			agent.Update(this.smell, smell);
		}

		public void destroy() {
			agent.DestroyWME(objectIdentifier);
			objectIdentifier = null;
		}
	}
	HashMap<String, ObjectInputLink> objects = new HashMap<String, ObjectInputLink>();
	
	public SoarCook(Agent agent, String playerId) {
		super(playerId);

		this.agent = agent;
		agent.SetBlinkIfNoChange(false);
		
		random = 0;
		generateNewRandom();

		initInputLink();
		
		if (!agent.Commit()) {
			error(Names.Errors.commitFail + this.getName());
			Soar2D.control.stopSimulation();
		}
	}
	
	private void error(String message) {
		logger.error(message);
		Soar2D.control.errorPopUp(message);
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
	
	private void initInputLink() {
		self = agent.CreateIdWME(agent.GetInputLink(), "self");
		Identifier position = agent.CreateIdWME(self, "position");
		x = agent.CreateIntWME(position, "x", 0);
		y = agent.CreateIntWME(position, "y", 0);
		reward = agent.CreateIntWME(self, "reward", 0);
		view = agent.CreateIdWME(agent.GetInputLink(), "view");
		Identifier north = agent.CreateIdWME(view, "north");
		Identifier south = agent.CreateIdWME(view, "south");
		Identifier east = agent.CreateIdWME(view, "east");
		Identifier west = agent.CreateIdWME(view, "west");
		northObject = agent.CreateStringWME(north, "object", "false");
		southObject = agent.CreateStringWME(south, "object", "false");
		eastObject = agent.CreateStringWME(east, "object", "false");
		westObject = agent.CreateStringWME(west, "object", "false");
		northType = agent.CreateStringWME(north, "type", "normal");
		southType = agent.CreateStringWME(south, "type", "normal");
		eastType = agent.CreateStringWME(east, "type", "normal");
		westType = agent.CreateStringWME(west, "type", "normal");
		cell = agent.CreateIdWME(agent.GetInputLink(), "cell");
		type = agent.CreateStringWME(cell, "type", "normal");
		randomWME = agent.CreateFloatWME(agent.GetInputLink(), "random", random);
		
		metadata = InputLinkMetadata.load(agent);
	}

	@Override
	public void moveWithObjectFailed() {
		moveWithObjectCommand.AddStatusError();
		moveWithObjectCommand = null;
	}
	
	@Override
	public void moveFailed() {
		moveCommand.AddStatusError();
		moveCommand = null;
	}
	
	@Override
	public void mixFailed() {
		mixCommand.AddStatusError();
		mixCommand = null;
	}
	
	@Override
	public void cookFailed() {
		cookCommand.AddStatusError();
		cookCommand = null;
	}
	
	@Override
	public void eatFailed() {
		eatCommand.AddStatusError();
		eatCommand = null;
	}
	
	@Override
	public void update(int [] location) {
		World world = Soar2D.simulation.world;
		KitchenMap map = (KitchenMap)world.getMap();
		
		// TODO: this algorithm assumes nothing will change without us acting
		assert world.getPlayers().size() == 1;

		// check to see if we've moved
		super.update(location);
		
		if (moved) {
			// clear out the current cell data
			agent.DestroyWME(cell);
			objects.clear();

			// update position
			agent.Update(x, location[0]);
			agent.Update(y, location[1]);

			// update view
			int [] tempLocation;
			tempLocation = Arrays.copyOf(location, location.length);
			Direction.translate(tempLocation, Direction.kNorthInt);
			updateCell(map, tempLocation, northObject, northType);

			tempLocation = Arrays.copyOf(location, location.length);
			Direction.translate(tempLocation, Direction.kSouthInt);
			updateCell(map, tempLocation, southObject, southType);
			
			tempLocation = Arrays.copyOf(location, location.length);
			Direction.translate(tempLocation, Direction.kEastInt);
			updateCell(map, tempLocation, eastObject, eastType);
			
			tempLocation = Arrays.copyOf(location, location.length);
			Direction.translate(tempLocation, Direction.kWestInt);
			updateCell(map, tempLocation, westObject, westType);

			// update cell type
			updateCell(map, location, null, type);
		}
		
		// update object information
		ArrayList<CellObject> stuff = map.getAllWithProperty(location, "smell");
		HashMap<String, ObjectInputLink> newObjects = new HashMap<String, ObjectInputLink>();
		if (stuff != null) {
			for (CellObject item : stuff) {
				ObjectInputLink itemIL = objects.get(item.getName());
				if (itemIL == null) {
					itemIL = new ObjectInputLink(
							item.getName(), 
							item.getProperty("texture"), 
							item.getProperty("color"), 
							item.getProperty("smell"));
				} else {
					itemIL.Update(
							item.getProperty("texture"), 
							item.getProperty("color"), 
							item.getProperty("smell"));
				}
				newObjects.put(item.getName(), itemIL);
			}
			for (String objKey : objects.keySet()) {
				if (newObjects.containsKey(objKey) == false) {
					objects.get(objKey).destroy();
				}
			}
		}
		objects.clear();
		objects = newObjects;
		
		// update the reward no matter what
		agent.Update(reward, this.getPointsDelta());
		
		// update the random no matter what
		float oldrandom = random;
		do {
			random = Simulation.random.nextFloat();
		} while (random == oldrandom);
		agent.Update(randomWME, random);
		
		// update statuses
		if (moveWithObjectCommand != null) {
			moveWithObjectCommand.AddStatusComplete();
		}
		if (moveCommand != null) {
			moveCommand.AddStatusComplete();
		}
		if (mixCommand != null) {
			mixCommand.AddStatusComplete();
		}
		if (cookCommand != null) {
			cookCommand.AddStatusComplete();
		}
		if (eatCommand != null) {
			eatCommand.AddStatusComplete();
		}
		
		// commit everything
		if (!agent.Commit()) {
			error(Names.Errors.commitFail + this.getName());
			Soar2D.control.stopSimulation();
		}
	}
	
	private void updateCell(KitchenMap map, int [] tempLocation, StringElement objectWME, StringElement typeWME) {
		if (objectWME != null) {
			if (map.hasAnyWithProperty(tempLocation, "smell")) {
				agent.Update(objectWME, "true");
			} else {
				agent.Update(objectWME, "false");
			}
		}
		
		if (map.hasObject(tempLocation, "wall")) {
			agent.Update(typeWME, "wall");
		} else if (map.hasObject(tempLocation, "oven")) {
			agent.Update(typeWME, "oven");
		} else if (map.hasObject(tempLocation, "countertop")) {
			agent.Update(typeWME, "countertop");
		} else {
			agent.Update(typeWME, "normal");
		}
	}
	
	Identifier moveWithObjectCommand;
	Identifier moveCommand;
	Identifier eatCommand;
	Identifier mixCommand;
	Identifier cookCommand;
	
	/* (non-Javadoc)
	 * @see soar2d.player.Eater#getMove()
	 */
	public MoveInfo getMove() {
		// if there was no command issued, that is kind of strange
		if (agent.GetNumberCommands() == 0) {
			if (logger.isDebugEnabled()) {
				logger.debug(getName() + " issued no command.");
			}
			return new MoveInfo();
		}

		// go through the commands
		// see move info for details
		MoveInfo move = new MoveInfo();
		for (int i = 0; i < agent.GetNumberCommands(); ++i) {
			Identifier commandId = agent.GetCommand(i);
			String commandName = commandId.GetAttribute();
			
			if (commandName.equalsIgnoreCase(Names.kMoveID)) {
				if (move.move) {
					logger.warn(getName() + ": multiple move commands detected");
					continue;
				}
				move.move = true;
				
				String direction = commandId.GetParameterValue(Names.kDirectionID);
				if (direction != null) {
					move.moveDirection = Direction.getInt(direction); 
					this.setFacingInt(move.moveDirection);
					moveCommand = commandId;
					continue;
				}
				
			} else if (commandName.equalsIgnoreCase("move-with-object")) {
				if (move.moveWithObject) {
					logger.warn(getName() + ": multiple move-with-object commands detected");
					continue;
				}
				move.moveWithObject = true;
				moveWithObjectCommand = commandId;
				continue;

			} else if (commandName.equalsIgnoreCase("mix")) {
				if (move.mix) {
					logger.warn(getName() + ": multiple mix commands detected");
					continue;
				}
				move.mix = true;
				mixCommand = commandId;
				continue;

			} else if (commandName.equalsIgnoreCase("cook")) {
				if (move.cook) {
					logger.warn(getName() + ": multiple cook commands detected");
					continue;
				}
				move.cook = true;
				cookCommand = commandId;
				continue;

			} else if (commandName.equalsIgnoreCase("eat")) {
				if (move.eat) {
					logger.warn(getName() + ": multiple eat commands detected");
					continue;
				}
				move.eat = true;
				eatCommand = commandId;
				continue;

			} else {
				logger.warn("Unknown command: " + commandName);
				continue;
			}
			
			logger.warn("Improperly formatted command: " + commandName);
		}
		agent.ClearOutputLinkChanges();
		if (!agent.Commit()) {
			error(Names.Errors.commitFail + this.getName());
			Soar2D.control.stopSimulation();
		}
		return move;
	}
	
	/* (non-Javadoc)
	 * @see soar2d.player.Player#reset()
	 */
	public void reset() {
		super.reset();
		
		if (agent == null) {
			return;
		}

		agent.DestroyWME(self);
		agent.DestroyWME(view);
		agent.DestroyWME(cell);
		metadata.destroy();
		metadata = null;
		
		initInputLink();


		if (!agent.Commit()) {
			error(Names.Errors.commitFail + this.getName());
			Soar2D.control.stopSimulation();
		}

		agent.InitSoar();
	}
}
