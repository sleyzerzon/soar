package eaters;

public interface SimulationListener {
	// TODO: split update event out since it will slow things down
	public static final int 
		kNoEvent = 0,
		kStartEvent = 1,
		kStopEvent = 2,
		kShutdownEvent = 3,
		kUpdateEvent = 4,
		kNewWorldEvent = 5,
		kAgentCreatedEvent = 6,
		kAgentDestroyedEvent = 7;
	
	public void simulationEventHandler(int type);
}
