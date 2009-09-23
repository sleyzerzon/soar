package edu.umich.soar.gridmap2d.map;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;
import java.util.Queue;

import jmat.LinAlg;

import lcmtypes.pose_t;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.gridmap2d.config.PlayerConfig;
import edu.umich.soar.gridmap2d.core.Names;
import edu.umich.soar.gridmap2d.core.Simulation;
import edu.umich.soar.robot.DifferentialDriveCommand;
import edu.umich.soar.robot.SendMessagesInterface;

public class RoomWorld implements World, SendMessagesInterface {
	private static Log logger = LogFactory.getLog(RoomWorld.class);

	private RoomMap map;
	private PlayersManager<Robot, RobotCommand> players = new PlayersManager<Robot, RobotCommand>();
	private List<String> stopMessages = new ArrayList<String>();
	private final double LIN_SPEED = 16;
	public static final int CELL_SIZE = 16;
	private double ANG_SPEED = Math.PI / 4.0;
	private String blockManipulationReason;
	private Queue<Message> messages = new LinkedList<Message>();
	private final Simulation sim;
	
	public RoomWorld(Simulation sim) {
		this.sim = sim;
	}

	@Override
	public boolean hasPlayer(String name) {
		return players.get(name) != null;
	}
	
	@Override
	public Player addPlayer(PlayerConfig cfg) {
		int [] location = WorldUtil.getStartingLocation(map, cfg.pos);
		if (location == null) {
			sim.error("Room Environment", "There are no suitable starting locations.");
			return null;
		}

		Robot player = new Robot(sim, cfg.name, cfg.color);
		players.add(player, cfg.pos);
		
		if (cfg.productions != null) {
			RobotCommander cmdr = sim.getCogArch().createRoomCommander(player, this, cfg.productions, cfg.shutdown_commands);
			if (cmdr == null) {
				players.remove(player);
				return null;
			}
			player.setCommander(cmdr);
		} else if (cfg.script != null) {
			sim.info("Room Environment", "Scripted robots not implemented.");
		}

		// put the player in it
		map.getCell(location).addPlayer(player);
		
		player.getState().setLocationId(map.getLocationId(location));
		double [] floatLocation = defaultFloatLocation(location);
		player.getState().setPos(floatLocation);

		logger.info(player.getName() + ": Spawning at (" + location[0] + "," + location[1] + "), (" + floatLocation[0] + "," + floatLocation[1] + ")");
		return player;
	}

	@Override
	public GridMap getMap() {
		return map;
	}

	@Override
	public List<? extends Player> getPlayers() {
		return players.getAllAsPlayers();
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
	public void removePlayer(String name) {
		Robot player = players.get(name);
		map.getCell(player.getState().getLocation()).removePlayer(player);
		players.remove(player);
		player.shutdownCommander();
	}

	@Override
	public void reset() {
		blockManipulationReason = null;
		messages.clear();
		map.reset();
		resetState();
	}

	@Override
	public void setAndResetMap(String mapPath) {
		map = new RoomMap(mapPath);
		resetState();
	}

	private void resetState() {
		stopMessages.clear();
		resetPlayers();
	}

	/**
	 * @throws IllegalStateException If there are no available locations to spawn the players
	 */
	private void resetPlayers() {
		if (players.numberOfPlayers() == 0) {
			return;
		}
		
		for (Robot player : players.getAll()) {
			player.reset();
			
			// find a suitable starting location
			int [] location = WorldUtil.getStartingLocation(map, players.getInitialLocation(player));
			if (location == null) {
				throw new IllegalStateException("no empty locations available for spawn");
			}

			// put the player in it
			map.getCell(location).addPlayer(player);

			player.getState().setLocationId(map.getLocationId(location));

			double [] floatLocation = defaultFloatLocation(location);
			player.getState().setPos(floatLocation);

			logger.info(player.getName() + ": Spawning at (" + location[0] + "," + location[1] + "), (" + floatLocation[0] + "," + floatLocation[1] + ")");
		}
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
	public void update(int worldCount) {
		WorldUtil.checkNumPlayers(sim, players.numberOfPlayers());

		// Collect input
		for (Robot player : players.getAll()) {
			RobotCommand command = player.getCommand();
			if (command == null) {
				sim.stop();
				return;
			}
			players.setCommand(player, command);
			WorldUtil.checkStopSim(sim, stopMessages, command.isStopSim(), player);
		}
		
		moveRoomPlayers(0.005);
		
		for (Message message : messages) {
			message.recipient.getReceiveMessagesInterface().newMessage(message.from, message.tokens);
		}
		messages.clear();
	}

	private void moveRoomPlayers(double time) {
		for (Robot player : players.getAll()) {
			RobotCommand command = players.getCommand(player);	
			RobotState state = player.getState();
			
			DifferentialDriveCommand ddc = command.getDdc();
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

	private boolean checkBlocked(int [] location) {
		if (map.getCell(location).hasObjectWithProperty(Names.kPropertyBlock)) {
			return true;
		}
		return false;
	}
	
	private void roomMovePlayer(Robot player, double time) {
		int [] oldLocation = player.getState().getLocation();
		int [] newLocation = Arrays.copyOf(oldLocation, oldLocation.length);

		RobotState state = player.getState();
		state.update(time);
		pose_t pose = state.getPose();
		
		newLocation[0] = (int)pose.pos[0] / CELL_SIZE;
		newLocation[1] = (int)pose.pos[1] / CELL_SIZE;

		if (Arrays.equals(oldLocation, newLocation)) {
			map.getCell(oldLocation).setModified(true);
		} else {
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
			
			map.getCell(oldLocation).removePlayer(player);
			state.setLocationId(map.getLocationId(newLocation));
			map.getCell(newLocation).addPlayer(player);
		}
	}

	public boolean dropObject(Robot player, int id) {
		blockManipulationReason = null;
		RobotState state = player.getState();

		if (!state.hasObject()) {
			blockManipulationReason = "Not carrying an object";
			return false;
		}
		
		RoomObject object = state.getRoomObject();
		state.drop();
		object.setPose(state.getPose());
		map.getCell(player.getState().getLocation()).addObject(object.getCellObject());
		return true;
	}

	/**
	 * @param player
	 * @param id
	 * @return
	 * @throws IllegalStateException If tried to remove the object from the cell it was supposed to be in but it wasn't there 
	 */
	public boolean getObject(Robot player, int id) {
		blockManipulationReason = null;
		
		if (player.getState().hasObject()) {
			blockManipulationReason = "Already carrying an object";
			return false;
		}
		
		// note: This is a stupid way to do this.
		for (RoomObject rObj : map.getRoomObjects()) {
			if (rObj.getPose() == null) {
				// already carried by someone
				continue;
			}
			CellObject cObj = rObj.getCellObject();
			
			if (rObj.getId() == id) {
				double distance = LinAlg.distance(player.getState().getPose().pos, rObj.getPose().pos);
				if (Double.compare(distance, CELL_SIZE) <= 0) {
					if (!cObj.getCell().removeObject(cObj)) {
						throw new IllegalStateException("Remove object failed for object that should be there.");
					}
					rObj.setPose(null);
					player.getState().pickUp(rObj);
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

	public List<double[]> getWaypointList(Robot player) {
		return player.getWaypointList();
	}

	private static class Message {
		String from;
		Robot recipient;
		List<String> tokens;
	}
	
	@Override
	public void sendMessage(String from, String to, List<String> tokens) {
		if (to != null) {
			Robot recipient = players.get(to);
			if (recipient == null) {
				Robot sender = players.get(from);
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
				Robot recipient = (Robot)p;
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
