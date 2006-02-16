package eaters;

public interface SimulationListener {
	public static final int kStartEvent = 0;
	public static final int kStopEvent = 1;
	public static final int kShutdownEvent = 2;
	public static final int kNewEaterEvent = 3;
	public static final int kUpdateEvent = 4;
	
	public void simulationEventHandler(int type, Object object);
}
