package simulation;

import java.util.*;

import sml.*;
import utilities.*;

public abstract class Simulation implements Runnable, Kernel.UpdateEventInterface, Kernel.SystemEventInterface {
	public static final String kGroupFolder = "Environments";
	public static final String kProjectFolder = "JavaEaters";
	public static final String kAgentFolder = "agents";
	public static final String kMapFolder = "maps";
	public static final int kDebuggerTimeoutSeconds = 15;	
	
	protected Logger m_Logger = Logger.logger;
	Kernel m_Kernel;
	boolean m_StopSoar = false;
	int m_WorldCount = 0;
	protected int m_Runs = 0;
    private Thread m_RunThread;
	String m_LastErrorMessage = "No error.";
	String m_BasePath;
	WorldManager m_WorldManager;
	protected String m_CurrentMap;
	protected boolean m_Debuggers;
	boolean m_DebuggerSpawned = false;
	ArrayList m_SimulationListeners = new ArrayList();
	ArrayList m_AddSimulationListeners = new ArrayList();
	ArrayList m_RemoveSimulationListeners = new ArrayList();
	
	protected Simulation() {
		// Initialize Soar
		// Create kernel
		try {
			m_Kernel = Kernel.CreateKernelInNewThread("SoarKernelSML", 12121);
		} catch (Exception e) {
			fireErrorMessage("Exception while creating kernel: " + e.getMessage());
			System.exit(1);
		}

		if (m_Kernel.HadError()) {
			fireErrorMessage("Error creating kernel: " + m_Kernel.GetLastErrorDescription());
			System.exit(1);
		}

		// Register for events
		m_Kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_START, this, null);
		m_Kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_STOP, this, null);
		m_Kernel.RegisterForUpdateEvent(smlUpdateEventId.smlEVENT_AFTER_ALL_OUTPUT_PHASES, this, null);
		
		// Generate base path
		// TODO: chop instead of using ..
		m_BasePath = new String(m_Kernel.GetLibraryLocation());
		m_BasePath += System.getProperty("file.separator")
		+ ".." + System.getProperty("file.separator") 
		+ kGroupFolder + System.getProperty("file.separator") 
		+ kProjectFolder + System.getProperty("file.separator");

		m_Logger.log("Base path: " + m_BasePath);
		
		
	}
	
	protected void setWorldManager(WorldManager worldManager) {
		m_WorldManager = worldManager;
	}
	
	public String getCurrentMap() {
		return m_CurrentMap;
	}
	
	public String getLastErrorMessage() {
		return m_LastErrorMessage;
	}
	
	public int getRuns() {
		return m_Runs;
	}
	
  	public int getWorldCount() {
  		return m_WorldCount;
  	}
  	
	public String getAgentPath() {
		return m_BasePath + kAgentFolder + System.getProperty("file.separator");
	}
	
	public String getMapPath() {
		return m_BasePath + kMapFolder + System.getProperty("file.separator");
	}
	
    protected Agent createAgent(String name, String productions) {
    	Agent agent = m_Kernel.CreateAgent(name);
    	boolean load = agent.LoadProductions(productions);
    	if (!load || agent.HadError()) {
			fireErrorMessage("Failed to create agent " + name + " (" + productions + "): " + agent.GetLastErrorDescription());
			m_Kernel.DestroyAgent(agent);
			agent.delete();
    		return null;
    	}
    	return agent;
    }
        
    protected void destroyAgent(Agent agent) {
		m_Kernel.DestroyAgent(agent);
		agent.delete();
		if (m_WorldManager.noAgents()) {
			m_DebuggerSpawned = false;
		}
    }
        
	public void spawnDebugger(String agentName) {
		if (!m_Debuggers) return;
		if (m_DebuggerSpawned) return;
		
		// Figure out whether to use java or javaw
		String os = System.getProperty("os.name");
		String javabin = "java";
		if (os.matches(".+indows.*") || os.matches("INDOWS")) {
			javabin = "javaw";
		}
		
		Runtime r = java.lang.Runtime.getRuntime();
		try {
			// TODO: manage the returned process a bit better.
			r.exec(javabin + " -jar \"" + m_Kernel.GetLibraryLocation() + System.getProperty("file.separator")
					+ "bin" + System.getProperty("file.separator") 
					+ "SoarJavaDebugger.jar\" -remote -agent " + agentName);
			
			if (!waitForDebugger()) {
				fireErrorMessage("Debugger spawn failed for agent: " + agentName);
				return;
			}
			
		} catch (java.io.IOException e) {
			fireErrorMessage("IOException spawning debugger: " + e.getMessage());
			shutdown();
			System.exit(1);
		}
		
		m_Logger.log("Spawned debugger for " + agentName);
		m_DebuggerSpawned = true;
	}
	
	boolean waitForDebugger() {
		boolean ready = false;
		for (int tries = 0; tries < kDebuggerTimeoutSeconds; ++tries) {
			m_Kernel.GetAllConnectionInfo();
			if (m_Kernel.HasConnectionInfoChanged()) {
				for (int i = 0; i < m_Kernel.GetNumberConnections(); ++i) {
					ConnectionInfo info =  m_Kernel.GetConnectionInfo(i);
					if (info.GetName().equalsIgnoreCase("java-debugger")) {
						if (info.GetAgentStatus().equalsIgnoreCase(sml_Names.getKStatusReady())) {
							ready = true;
							break;
						}
					}
				}
				if (ready) break;
			}
			try { Thread.sleep(1000); } catch (InterruptedException ignored) {}
		}
		return ready;
	}
	
	public void shutdown() {
		m_Logger.log("Shutdown called.");
		if (m_WorldManager != null) {
			m_WorldManager.shutdown();
		}
		if (m_Kernel != null) {
			m_Kernel.Shutdown();
			m_Kernel.delete();
		}
	}
	
	public void startSimulation(boolean inNewThread) {
        m_StopSoar = false;
		if (inNewThread) {
			m_RunThread = new Thread(this);
	        m_RunThread.start();
		} else {
			run();
		}
	}
	
	public void stepSimulation() {
        m_Kernel.RunAllAgents(1);
	}
	
	public void stopSimulation() {
		if (m_Runs == 0) {
			m_RunThread = null;
		}
		m_StopSoar = true;
	}
	
	public boolean isRunning() {
		return (m_RunThread != null);
	}

	protected void resetSimulation() {
		m_WorldCount = 0;
		fireSimulationEvent(SimulationListener.kResetEvent);
	}

    public void run() {
    	do {
    		if (m_Runs > 0) {
    			--m_Runs;
    		}
    		
    		m_StopSoar = false;
    		m_Kernel.RunAllAgentsForever();
    		
    		if (m_Runs != 0) {
    			resetSimulation();
    		}
    	} while (m_Runs != 0);
    }
    
  	public void updateEventHandler(int eventID, Object data, Kernel kernel, int runFlags) {
  		if (m_StopSoar) {
  			m_StopSoar = false;
  			m_Kernel.StopAllAgents();
  		}
  		m_WorldManager.update();
		++m_WorldCount;
		fireSimulationEvent(SimulationListener.kUpdateEvent);
  	}
  	
    public void systemEventHandler(int eventID, Object data, Kernel kernel) {
  		if (eventID == smlSystemEventId.smlEVENT_SYSTEM_START.swigValue()) {
  			// Start simulation
  			m_Logger.log("Start event received from kernel.");
  			fireSimulationEvent(SimulationListener.kStartEvent);
  		} else if (eventID == smlSystemEventId.smlEVENT_SYSTEM_STOP.swigValue()) {
  			// Stop simulation
  			m_Logger.log("Stop event received from kernel.");
  			if (m_Runs == 0) {
  				m_RunThread = null;
  			}
  			fireSimulationEvent(SimulationListener.kStopEvent);	
 		} else {
 			m_Logger.log("Unknown system event received from kernel: " + eventID);
 		}
    }
    
	protected void fireErrorMessage(String errorMessage) {
		m_LastErrorMessage = errorMessage;
		fireSimulationEvent(SimulationListener.kErrorMessageEvent);
		m_Logger.log(errorMessage);
	}
	
	public void addSimulationListener(SimulationListener listener) {
		m_AddSimulationListeners.add(listener);
	}
	
	public void removeSimulationListener(SimulationListener listener) {
		m_RemoveSimulationListeners.add(listener);
	}
	
	protected void fireSimulationEvent(int type) {
		updateSimulationListenerList();
		Iterator iter = m_SimulationListeners.iterator();
		while(iter.hasNext()){
			((SimulationListener)iter.next()).simulationEventHandler(type);
		}		
	}
		
	protected void updateSimulationListenerList() {
		Iterator iter = m_RemoveSimulationListeners.iterator();
		while(iter.hasNext()){
			m_SimulationListeners.remove(iter.next());
		}
		m_RemoveSimulationListeners.clear();
		
		iter = m_AddSimulationListeners.iterator();
		while(iter.hasNext()){
			m_SimulationListeners.add(iter.next());
		}
		m_AddSimulationListeners.clear();		
	}
}