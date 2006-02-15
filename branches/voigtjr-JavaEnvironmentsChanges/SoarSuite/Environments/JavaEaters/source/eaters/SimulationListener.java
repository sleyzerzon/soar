package eaters;

public interface SimulationListener {
	public static final int kStartEvent = 0;
	public static final int kStopEvent = 1;
	public static final int kQuitEvent = 2;
	
	public void simulationEventHandler(int type);
}
