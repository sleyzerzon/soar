package eaters;

public interface SimulationListener {
	public static final int 
		kStartEvent = 0,
		kStopEvent = 1,
		kShutdownEvent = 2,
		kUpdateEvent = 3,
		kNewWorldEvent = 4;
	
	public void simulationEventHandler(int type, Object object);
}
