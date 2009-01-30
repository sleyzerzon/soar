package soar2d;

import sml.Agent;

public interface CognitiveArchitecture {

	void seed(int seed);

	Agent createSoarAgent(String name, String productions) throws Exception;

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
