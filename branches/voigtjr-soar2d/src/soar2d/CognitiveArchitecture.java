package soar2d;

import java.io.File;

import soar2d.players.Eater;
import soar2d.players.EaterCommander;
import soar2d.players.Tank;
import soar2d.players.TankCommander;

public interface CognitiveArchitecture {

	void seed(int seed);

	EaterCommander createEaterCommander(Eater eater, String productions, int vision, String[] shutdownCommands, File metadataFile) throws Exception;
	TankCommander createTankCommander(Tank tank, String productions, String[] shutdown_commands, File metadataFile) throws Exception;

	void doBeforeClients() throws Exception;

	void doAfterClients() throws Exception;

	void destroyPlayer(String name);

	void reload(String name);

	void shutdown();

	String getAgentPath();

	boolean isClientConnected(String debuggerClient);

	boolean haveAgents();

	void runStep();

	void runForever();

}
