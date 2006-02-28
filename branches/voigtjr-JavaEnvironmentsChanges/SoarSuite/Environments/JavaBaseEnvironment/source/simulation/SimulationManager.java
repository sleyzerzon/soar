package simulation;

public interface SimulationManager {
	public WorldManager getWorldManager();
	public String getAgentPath();
    public void createEntity(String name, String productions, String color);
}
