package eaters;

public interface SimulationListener {
	// TODO: split update event out since it will slow things down
	public static final int 
		kStartEvent = 0,
		kStopEvent = 1,
		kShutdownEvent = 2,
		kUpdateEvent = 3,
		kNewWorldEvent = 4,
		kAgentCreatedEvent = 5,
		kAgentDestroyedEvent = 6;
	
	public void simulationEventHandler(int type, Object object);
}
