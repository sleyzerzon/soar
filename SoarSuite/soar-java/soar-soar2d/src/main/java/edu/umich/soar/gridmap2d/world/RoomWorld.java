package edu.umich.soar.gridmap2d.world;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;
import java.util.Queue;
import java.util.Set;

import jmat.LinAlg;

import lcmtypes.pose_t;

import org.apache.log4j.Logger;

import edu.umich.soar.gridmap2d.CognitiveArchitecture;
import edu.umich.soar.gridmap2d.Direction;
import edu.umich.soar.gridmap2d.Gridmap2D;
import edu.umich.soar.gridmap2d.Names;
import edu.umich.soar.gridmap2d.config.PlayerConfig;
import edu.umich.soar.gridmap2d.map.Cell;
import edu.umich.soar.gridmap2d.map.CellObject;
import edu.umich.soar.gridmap2d.map.GridMap;
import edu.umich.soar.gridmap2d.map.RoomMap;
import edu.umich.soar.gridmap2d.players.CommandInfo;
import edu.umich.soar.gridmap2d.players.Player;
import edu.umich.soar.gridmap2d.players.RoomCommander;
import edu.umich.soar.gridmap2d.players.RoomPlayer;
import edu.umich.soar.gridmap2d.players.RoomPlayerState;
import edu.umich.soar.robot.DifferentialDriveCommand;
import edu.umich.soar.robot.SendMessagesInterface;

public class RoomWorld implements World, SendMessagesInterface {
	private static Logger logger = Logger.getLogger(RoomWorld.class);

	private RoomMap roomMap;
	private PlayersManager<RoomPlayer> players = new PlayersManager<RoomPlayer>();
	private boolean forceHuman = false;
	private List<String> stopMessages = new ArrayList<String>();
	private final double LIN_SPEED = 16;
	public static final int CELL_SIZE = 16;
	private double ANG_SPEED = Math.PI / 4.0;
	private CognitiveArchitecture cogArch;
	private String blockManipulationReason;
	private Queue<Message> messages = new LinkedList<Message>();
	
	public RoomWorld(CognitiveArchitecture cogArch) {
		this.cogArch = cogArch;
	}

	@Override
	public void addPlayer(String playerId, PlayerConfig playerConfig, boolean debug) throws Exception {
		
		RoomPlayer player = new RoomPlayer(playerId);

		players.add(player, roomMap, playerConfig.pos);
		
		if (playerConfig.productions != null) {
			RoomCommander eaterCommander = cogArch.createRoomCommander(player, this, playerConfig.productions, playerConfig.shutdown_commands, roomMap.getMetadataFile(), debug);
			player.setCommander(eaterCommander);
		} else if (playerConfig.script != null) {
			// TODO: implement
			assert false;
		}

		int [] location = WorldUtil.getStartingLocation(player, roomMap, players.getInitialLocation(player));
		players.setLocation(player, location);
		
		// put the player in it
		roomMap.getCell(location).setPlayer(player);
		
		player.getState().setLocationId(roomMap.getLocationId(location));
		double [] floatLocation = defaultFloatLocation(location);
		player.getState().setPos(floatLocation);

		logger.info(player.getName() + ": Spawning at (" + location[0] + "," + location[1] + "), (" + floatLocation[0] + "," + floatLocation[1] + ")");
		
		updatePlayers();
	}

	@Override
	public GridMap getMap() {
		return roomMap;
	}

	@Override
	public Player[] getPlayers() {
		return players.getAllAsPlayers();
	}

	@Override
	public void interrupted(String agentName) throws Exception {
		players.interrupted(agentName);
		stopMessages.add("interrupted");
	}

	@Override
	public boolean isTerminal() {
		return stopMessages.size() > 0;
	}

	@Override
	public int numberOfPlayers() {
		return players.numberOfPlayers();
	}

	@Override
	public void removePlayer(String name) throws Exception {
		RoomPlayer player = players.get(name);
		roomMap.getCell(players.getLocation(player)).setPlayer(null);
		players.remove(player);
		player.shutdownCommander();
		updatePlayers();
	}

	@Override
	public void reset() throws Exception {
		blockManipulationReason = null;
		messages.clear();
		roomMap.reset();
		resetState();
	}

	@Override
	public void setForceHumanInput(boolean setting) {
		forceHuman = setting;
	}

	@Override
	public void setMap(String mapPath) throws Exception {
		roomMap = new RoomMap(mapPath);
		resetState();
	}

	private void resetState() throws Exception {
		stopMessages.clear();
		resetPlayers();
	}

	private void resetPlayers() throws Exception {
		if (players.numberOfPlayers() == 0) {
			return;
		}
		
		for (RoomPlayer player : players.getAll()) {
			player.reset();
			
			// find a suitable starting location
			int [] startingLocation = WorldUtil.getStartingLocation(player, roomMap, players.getInitialLocation(player));
			players.setLocation(player, startingLocation);

			// put the player in it
			roomMap.getCell(startingLocation).setPlayer(player);

			player.getState().setLocationId(roomMap.getLocationId(startingLocation));

			double [] floatLocation = defaultFloatLocation(startingLocation);
			player.getState().setPos(floatLocation);

			logger.info(player.getName() + ": Spawning at (" + startingLocation[0] + "," + startingLocation[1] + "), (" + floatLocation[0] + "," + floatLocation[1] + ")");
		}
		
		updatePlayers();
	}
	
	private double [] defaultFloatLocation(int [] location) {
		double [] floatLocation = new double [2];
		final int cellSize = CELL_SIZE;
		
		// default to center of square
		floatLocation[0] = (location[0] * cellSize) + (cellSize / 2); 
		floatLocation[1] = (location[1] * cellSize) + (cellSize / 2); 

		return floatLocation;
	}
	
	@Override
	public void update(int worldCount) throws Exception {
		WorldUtil.checkNumPlayers(players.numberOfPlayers());

		// Collect input
		for (RoomPlayer player : players.getAll()) {
			player.resetPointsChanged();

			CommandInfo command = forceHuman ? Gridmap2D.control.getHumanCommand(player) : player.getCommand();
			if (command == null) {
				Gridmap2D.control.stopSimulation();
				return;
			}
			players.setCommand(player, command);
			WorldUtil.checkStopSim(stopMessages, command, player);
		}
		
		moveRoomPlayers(Gridmap2D.control.getTimeSlice());
		
		for (Message message : messages) {
			message.recipient.getReceiveMessagesInterface().newMessage(message.from, message.tokens);
		}
		messages.clear();
		
		updatePlayers();
	}

	private void moveRoomPlayers(double time) throws Exception {
		for (RoomPlayer player : players.getAll()) {
			CommandInfo command = players.getCommand(player);	
			RoomPlayerState state = player.getState();
			
			DifferentialDriveCommand ddc = command.ddc;
			if (ddc != null) {
				switch(ddc.getType()) {
				case ANGVEL:
					state.setAngularVelocity(ddc.getAngularVelocity());
					state.resetDestYaw();
					break;
				case ESTOP:
					state.stop();
					break;
				case HEADING:
					player.getState().setDestYaw(ddc.getHeading(), ANG_SPEED);
					break;
				case HEADING_LINVEL:
					player.getState().setDestYaw(ddc.getHeading(), ANG_SPEED);
					state.setLinearVelocity(ddc.getLinearVelocity() * LIN_SPEED);
					break;
				case LINVEL:
					state.setLinearVelocity(ddc.getLinearVelocity() * LIN_SPEED);
					break;
				case MOTOR:
					// TODO: other than stop
					state.stop();
					break;
				case MOVE_TO:
					// TODO: implement
					assert false;
					break;
				case VEL:
					state.setAngularVelocity(ddc.getAngularVelocity());
					state.setLinearVelocity(ddc.getLinearVelocity() * LIN_SPEED);
					state.resetDestYaw();
					break;
				}
			}
						
			// reset collision sensor
			state.setCollisionX(false);
			state.setCollisionY(false);

			roomMovePlayer(player, time);
		}
	}

	private void updatePlayers() throws Exception {
		for (RoomPlayer player : players.getAll()) {
			player.update(players.getLocation(player), roomMap);
		}
	}

	private boolean checkBlocked(int [] location) {
		if (roomMap.getCell(location).hasAnyWithProperty(Names.kPropertyBlock)) {
			return true;
		}
		return false;
	}
	
	private void roomMovePlayer(RoomPlayer player, double time) {
		int [] oldLocation = players.getLocation(player);
		int [] newLocation = Arrays.copyOf(oldLocation, oldLocation.length);

		RoomPlayerState state = player.getState();
		state.update(time);
		pose_t pose = state.getPose();
		
		newLocation[0] = (int)pose.pos[0] / CELL_SIZE;
		newLocation[1] = (int)pose.pos[1] / CELL_SIZE;

		while (checkBlocked(newLocation)) {
			// 1) determine what edge we're intersecting
			if ((newLocation[0] != oldLocation[0]) && (newLocation[1] != oldLocation[1])) {
				// corner case
				int [] oldx = new int [] { oldLocation[0], newLocation[1] };
				
				// if oldx is blocked
				if (checkBlocked(oldx)) {
					state.setCollisionY(true);
					// calculate y first
					if (newLocation[1] > oldLocation[1]) {
						// south
						pose.pos[1] = oldLocation[1] * CELL_SIZE;
						pose.pos[1] += CELL_SIZE - 0.1;
						newLocation[1] = oldLocation[1];
					} 
					else if (newLocation[1] < oldLocation[1]) {
						// north
						pose.pos[1] = oldLocation[1] * CELL_SIZE;
						newLocation[1] = oldLocation[1];
					} else {
						assert false;
					}
				} 
				else {
					state.setCollisionX(true);
					// calculate x first
					if (newLocation[0] > oldLocation[0]) {
						// east
						pose.pos[0] = oldLocation[0] * CELL_SIZE;
						pose.pos[0] += CELL_SIZE - 0.1;
						newLocation[0] = oldLocation[0];
					} 
					else if (newLocation[0] < oldLocation[0]) {
						// west
						pose.pos[0] = oldLocation[0] * CELL_SIZE;
						newLocation[0] = oldLocation[0];
					} else {
						assert false;
					} 
				}
				continue;
			}
			
			if (newLocation[0] > oldLocation[0]) {
				state.setCollisionX(true);
				// east
				pose.pos[0] = oldLocation[0] * CELL_SIZE;
				pose.pos[0] += CELL_SIZE - 0.1;
				newLocation[0] = oldLocation[0];
			} 
			else if (newLocation[0] < oldLocation[0]) {
				state.setCollisionX(true);
				// west
				pose.pos[0] = oldLocation[0] * CELL_SIZE;
				newLocation[0] = oldLocation[0];
			} 
			else if (newLocation[1] > oldLocation[1]) {
				state.setCollisionY(true);
				// south
				pose.pos[1] = oldLocation[1] * CELL_SIZE;
				pose.pos[1] += CELL_SIZE - 0.1;
				newLocation[1] = oldLocation[1];
			} 
			else if (newLocation[1] < oldLocation[1]) {
				state.setCollisionY(true);
				// north
				pose.pos[1] = oldLocation[1] * CELL_SIZE;
				newLocation[1] = oldLocation[1];
			}
		}
		
		state.setPos(pose.pos);
		
		//state.setVelocity(new double [] { (newFloatLocation[0] - oldFloatLocation[0])/time, (newFloatLocation[1] - oldFloatLocation[1])/time });
		roomMap.getCell(oldLocation).setPlayer(null);
		players.setLocation(player, newLocation);
		state.setLocationId(roomMap.getLocationId(newLocation));
		roomMap.getCell(newLocation).setPlayer(player);
		
		// redraw the 8 cells around it
		int[] redrawLoc = Arrays.copyOf(newLocation, newLocation.length); 
		Direction.translate(redrawLoc, Direction.EAST);
		roomMap.getCell(redrawLoc).forceRedraw();
		Direction.translate(redrawLoc, Direction.SOUTH);
		roomMap.getCell(redrawLoc).forceRedraw();
		Direction.translate(redrawLoc, Direction.WEST);
		roomMap.getCell(redrawLoc).forceRedraw();
		Direction.translate(redrawLoc, Direction.WEST);
		roomMap.getCell(redrawLoc).forceRedraw();
		Direction.translate(redrawLoc, Direction.NORTH);
		roomMap.getCell(redrawLoc).forceRedraw();
		Direction.translate(redrawLoc, Direction.NORTH);
		roomMap.getCell(redrawLoc).forceRedraw();
		Direction.translate(redrawLoc, Direction.EAST);
		roomMap.getCell(redrawLoc).forceRedraw();
		Direction.translate(redrawLoc, Direction.EAST);
		roomMap.getCell(redrawLoc).forceRedraw();
	}

	public boolean dropObject(RoomPlayer player, int id) {
		blockManipulationReason = null;
		RoomPlayerState state = player.getState();

		if (!state.hasObject()) {
			blockManipulationReason = "Not carrying an object";
			return false;
		}
		
		CellObject object = state.getObject();
		
//		RoomMap.RoomObjectInfo info = roomMap.getRoomObjectInfo(object);
//		info.pose = state.getPose().copy();
//		
//		// move just a bit in front of player
//		double yaw = LinAlg.quatToRollPitchYaw(info.pose.orientation)[2];
//		info.pose.pos[0] += Math.cos(yaw);
//		info.pose.pos[1] += Math.sin(yaw);
//		info.location = new int [] { (int)info.pose.pos[0] / CELL_SIZE, (int)info.pose.pos[1] / CELL_SIZE };
//
//		Cell destCell = roomMap.getCell(info.location);
		
		Cell destCell = roomMap.getCell(player.getLocation());
		CellObject temp = destCell.getObject(object.getName());
		if (temp != null) {
			blockManipulationReason = "Destination cell for drop is occupied";
//			info.pose = null;
//			info.location = null;
			return false;
		}
		
		state.drop();
		destCell.addObject(object);
		return true;
	}

	public boolean getObject(RoomPlayer player, int id) {
		blockManipulationReason = null;
		
		if (player.getState().hasObject()) {
			blockManipulationReason = "Already carrying an object";
			return false;
		}
		
		// TODO: This is a stupid way to do this.
		Set<CellObject> objects = roomMap.getRoomObjects();
		for (CellObject object : objects) {
			if (object.getIntProperty("object-id", -1) == id) {
				RoomMap.RoomObjectInfo info = roomMap.getRoomObjectInfo(object);
				if (info.pose == null) {
					continue;
				}
				double distance = LinAlg.distance(player.getState().getPose().pos, info.pose.pos);
				if (Double.compare(distance, CELL_SIZE) <= 0) {
					if (roomMap.getCell(info.location).removeObject(object.getName()) == null) {
						throw new IllegalStateException("Remove object failed for object that should be there.");
					}
					info.location = null;
					info.pose = null;
					player.getState().pickUp(object);
					return true;
				}
				blockManipulationReason = "Object is too far";
				return false;
			}
		}
		blockManipulationReason = "No such object ID";
		return false;
	}

	public String reason() {
		return blockManipulationReason;
	}

	public List<double[]> getWaypointList(RoomPlayer player) {
		return player.getWaypointList();
	}

	private static class Message {
		String from;
		RoomPlayer recipient;
		List<String> tokens;
	}
	
	@Override
	public void sendMessage(String from, String to, List<String> tokens) {
		if (to != null) {
			RoomPlayer recipient = players.get(to);
			if (recipient == null) {
				RoomPlayer sender = players.get(from);
				if (sender == null) {
					StringBuilder sb = new StringBuilder();
					sb.append("Unknown sender ").append(from).append(" for message: ");
					for (String token : tokens) {
						sb.append(token);
						sb.append(" ");
					}
					logger.error(sb);
					return;
				}
				StringBuilder sb = new StringBuilder();
				sb.append("Unknown recipient ").append(to).append(" for message: ");
				for (String token : tokens) {
					sb.append(token);
					sb.append(" ");
				}
				logger.error(sb);
				return;
			}
			
			Message message = new Message();
			message.from = from;
			message.recipient = recipient;
			message.tokens = new ArrayList<String>(tokens.size());
			for (String token : tokens) {
				message.tokens.add(token);
			}
			messages.add(message);
		} else {
			for (Player p : getPlayers()) {
				RoomPlayer recipient = (RoomPlayer)p;
				Message message = new Message();
				message.from = from;
				message.recipient = recipient;
				message.tokens = new ArrayList<String>();
				message.tokens.addAll(tokens);
				messages.add(message);
			}
		}
	}
}
