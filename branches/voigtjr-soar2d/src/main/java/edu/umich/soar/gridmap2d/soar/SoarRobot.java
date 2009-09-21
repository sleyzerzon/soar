package edu.umich.soar.gridmap2d.soar;

import java.util.List;

import jmat.LinAlg;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.gridmap2d.core.Simulation;
import edu.umich.soar.gridmap2d.map.Robot;
import edu.umich.soar.gridmap2d.map.RobotCommand;
import edu.umich.soar.gridmap2d.map.RobotCommander;
import edu.umich.soar.gridmap2d.map.RoomMap;
import edu.umich.soar.gridmap2d.map.RoomWorld;
import edu.umich.soar.robot.ObjectManipulationInterface;
import edu.umich.soar.robot.OutputLinkManager;
import edu.umich.soar.robot.ConfigureInterface;
import edu.umich.soar.robot.OffsetPose;
import edu.umich.soar.robot.DifferentialDriveCommand;
import edu.umich.soar.robot.ReceiveMessagesInterface;
import lcmtypes.pose_t;

import sml.Agent;
import sml.Kernel;

public class SoarRobot implements RobotCommander, ConfigureInterface, OffsetPose, ObjectManipulationInterface, SoarAgent  {
	private static Log logger = LogFactory.getLog(SoarRobot.class);

	private Robot player;
	private Agent agent;
	private String [] shutdownCommands;
	RoomWorld world;
	
	final SoarRobotInputLinkManager input;
	final OutputLinkManager output;
	
	DifferentialDriveCommand ddc;
	pose_t offset = new pose_t();
	boolean floatYawWmes = true;
	
	public static final double PIXELS_2_METERS = 1.0 / 16.0;
	public static final double METERS_2_PIXELS = 16.0;
	
	private final Simulation sim;
	
	private RobotCommand command;
	
	public SoarRobot(Simulation sim, Robot player, Agent agent, Kernel kernel,
			RoomWorld world, String[] shutdownCommands) {
		this.player = player;
		this.sim = sim;
		this.agent = agent;
		this.world = world;
		this.shutdownCommands = shutdownCommands;
		
		agent.SetBlinkIfNoChange(false);
		
		input = new SoarRobotInputLinkManager(sim, agent, kernel, this, player.getState());
		input.create();
		output = new OutputLinkManager(agent);
		output.create(input.getWaypointInterface(), world, input.getReceiveMessagesInterface(), this, this, this);
	}

	@Override
	public void reset() {
		if (agent == null) {
			return;
		}
		command = null;
		input.destroy();
		output.destroy();

		agent.InitSoar();

		input.create();
		output.create(input.getWaypointInterface(), world, input.getReceiveMessagesInterface(), this, this, this);
	}

	@Override
	public RobotCommand nextCommand() {
		RobotCommand temp = command;
		command = null;
		return temp;
	}
	
	private void error(String message) {
		sim.error("Soar Robot", message);
	}
	
	@Override
	public void shutdown() {
		assert agent != null;
		if (shutdownCommands != null) { 
			// execute the pre-shutdown commands
			for (String command : shutdownCommands) {
				String result = player.getName() + ": result: " + agent.ExecuteCommandLine(command, true);
				logger.info(player.getName() + ": shutdown command: " + command);
				if (agent.HadError()) {
					error(result);
				} else {
					logger.info(player.getName() + ": result: " + result);
				}
			}
		}
	}

	@Override
	public boolean isFloatYawWmes() {
		return floatYawWmes;
	}

	@Override
	public void setFloatYawWmes(boolean setting) {
		floatYawWmes = setting;
	}

	@Override
	public pose_t getOffset() {
		pose_t copy = offset.copy();
		LinAlg.scaleEquals(copy.pos, PIXELS_2_METERS);
		LinAlg.scaleEquals(copy.vel, PIXELS_2_METERS);
		return copy;
	}

	@Override
	public pose_t getPose() {
		pose_t copy = player.getState().getPose().copy();
		LinAlg.scaleEquals(copy.pos, PIXELS_2_METERS);
		LinAlg.scaleEquals(copy.vel, PIXELS_2_METERS);
		return copy;
	}

	@Override
	public void setOffset(pose_t offset) {
		pose_t copy = offset.copy();
		LinAlg.scaleEquals(copy.pos, METERS_2_PIXELS);
		LinAlg.scaleEquals(copy.vel, METERS_2_PIXELS);
		this.offset = copy;
	}

	@Override
	public boolean drop(int id) {
		return world.dropObject(player, id);
	}

	@Override
	public boolean get(int id) {
		return world.getObject(player, id);
	}

	@Override
	public String reason() {
		return world.reason();
	}

	@Override
	public List<double[]> getWaypointList() {
		List<double[]> waypoints = input.getWaypointInterface().getWaypointList();
		for (double[] wp : waypoints) {
			LinAlg.scaleEquals(wp, METERS_2_PIXELS);
		}
		return waypoints;
	}

	@Override
	public ReceiveMessagesInterface getReceiveMessagesInterface() {
		return input.getReceiveMessagesInterface();
	}

	@Override
	public void processSoarOuput() {
		command = new RobotCommand.Builder(ddc).build();
	}

	@Override
	public void updateSoarInput() {
		ddc = output.update();

		input.update(player, world, (RoomMap)sim.getMap(), this.isFloatYawWmes());	
	}
}
