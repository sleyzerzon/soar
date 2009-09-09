package edu.umich.soar.gridmap2d.core;

import edu.umich.soar.gridmap2d.map.Eater;
import edu.umich.soar.gridmap2d.map.EaterCommander;
import edu.umich.soar.gridmap2d.map.Robot;
import edu.umich.soar.gridmap2d.map.RobotCommander;
import edu.umich.soar.gridmap2d.map.RoomWorld;
import edu.umich.soar.gridmap2d.map.Tank;
import edu.umich.soar.gridmap2d.map.TankCommander;
import edu.umich.soar.gridmap2d.map.Taxi;
import edu.umich.soar.gridmap2d.map.TaxiCommander;


public interface CognitiveArchitecture {

	void seed(int seed);

	EaterCommander createEaterCommander(Eater eater, String productions, int vision, String[] shutdownCommands);
	TankCommander createTankCommander(Tank tank, String productions, String[] shutdown_commands);
	TaxiCommander createTaxiCommander(Taxi taxi, String productions, String[] shutdown_commands);
	RobotCommander createRoomCommander(Robot player, RoomWorld world, String productions, String[] shutdown_commands);

	void doBeforeClients();

	void doAfterClients();

	boolean debug();
	
	void destroyPlayer(String name);

	void reload(String name);

	void shutdown();

	boolean isClientConnected(String debuggerClient);
}
