package tanksoar;

import org.eclipse.swt.graphics.Point;

import simulation.*;
import sml.*;

public class World implements WorldManager {

	public World(TankSoarSimulation simulation) {
		
	}
	
	public boolean load(String map) {
		return false;
	}
	
	void createTank(Agent agent, String productions, String color) {
		createTank(agent, productions, color, null);
	}

	void createTank(Agent agent, String productions, String color, Point location) {

	}
	
	void destroyTank(Tank tank) {
		
	}
	
	public void update() {
		
	}
	
	public boolean noAgents() {
		return true;
	}
	
	public void shutdown() {
		
	}
}
