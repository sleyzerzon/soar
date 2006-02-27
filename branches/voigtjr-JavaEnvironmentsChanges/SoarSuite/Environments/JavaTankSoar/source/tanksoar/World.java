package tanksoar;

import org.eclipse.swt.graphics.*;

import simulation.*;
import sml.*;

public class World implements WorldManager {
	static final String kTagTankSoarWorld = "tanksoar-world";

	static final String kTagCells = "cells";
	static final String kParamWorldWidth = "world-width";
	static final String kParamWorldHeight = "world-height";
	static final String kTagRow = "row";
	static final String kTagCell = "cell";
	static final String kParamType = "type";
	
	static final String kTypeWall = "wall";
	static final String kTypeEmpty = "empty";

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
