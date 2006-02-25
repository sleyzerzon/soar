package eaters;

import java.util.*;

import sml.*;
import utilities.*;

public class EatersSimulation  implements Runnable, Kernel.UpdateEventInterface, Kernel.SystemEventInterface {
	public static final int kDebuggerTimeoutSeconds = 15;	
	public static final int kMaxEaters = 8;	
	public static final String kTagEaters = "eaters";
	public static final String kTagSimulation = "simulation";
	public static final String kParamDebuggers = "debuggers";
	public static final String kParamDefaultMap = "default-map";
	public static final String kParamRuns = "runs";
	public static final String kTagAgents = "agents";
	public static final String kTagAgent = "agent";
	public static final String kParamName = "name";
	public static final String kParamProductions = "productions";
	public static final String kParamColor = "color";
		
	public static final String kGroupFolder = "Environments";
	public static final String kProjectFolder = "JavaEaters";
	public static final String kAgentFolder = "agents";
	public static final String kMapFolder = "maps";
	
    private Thread m_RunThread;
	boolean m_Debuggers;
	boolean m_DebuggerSpawned = false;
	int m_Runs = 0;
	String m_DefaultMap;
	Logger m_Logger = Logger.logger;
	String m_BasePath;
	World m_World;
	Kernel m_Kernel;
	boolean m_StopSoar = false;
	String m_CurrentMap;
	int m_WorldCount = 0;

	ArrayList m_SimulationListeners = new ArrayList();
	ArrayList m_AddSimulationListeners = new ArrayList();
	ArrayList m_RemoveSimulationListeners = new ArrayList();
	
	public EatersSimulation(String settingsFile, boolean quiet) {		
		
		// Log the settings file
		m_Logger.log("Settings file: " + settingsFile);

		// Initialize Soar
		initSoar();
		
		// Generate base path
		// TODO: chop instead of using ..
		m_BasePath = new String(m_Kernel.GetLibraryLocation());
		m_BasePath += System.getProperty("file.separator")
		+ ".." + System.getProperty("file.separator") 
		+ kGroupFolder + System.getProperty("file.separator") 
		+ kProjectFolder + System.getProperty("file.separator");

		m_Logger.log("Base path: " + m_BasePath);
		
		String [] initialNames = null;
		String [] initialProductions = null;
		String [] initialColors = null;
		
		// Load settings file
		try {
			JavaElementXML root = JavaElementXML.ReadFromFile(settingsFile);
			if (!root.getTagName().equalsIgnoreCase(kTagEaters)) {
				throw new Exception("Top level tag not " + kTagEaters);
			}
			// TODO: Version check
			
			for (int i = 0 ; i < root.getNumberChildren() ; ++i)
			{
				JavaElementXML child = root.getChild(i) ;

				String tagName = child.getTagName() ;
				
				if (tagName.equalsIgnoreCase(kTagSimulation)) {
					m_Debuggers = child.getAttributeBooleanThrows(kParamDebuggers);
					m_DefaultMap = child.getAttributeThrows(kParamDefaultMap);
					m_Runs = child.getAttributeIntThrows(kParamRuns);
					
					m_Logger.log("Default map: " + m_DefaultMap);
					
				} else if (tagName.equalsIgnoreCase(kTagAgents)) {
					initialNames = new String[child.getNumberChildren()];
					initialProductions = new String[child.getNumberChildren()];
					initialColors = new String[child.getNumberChildren()];
					for (int j = 0; j < initialNames.length; ++j) {
						JavaElementXML agent = child.getChild(j);
						
						initialNames[j] = agent.getAttributeThrows(kParamName);
						initialProductions[j] = agent.getAttributeThrows(kParamProductions);
						
						// Next two lines kind of a hack.  Convert / to \\ on windows, and vice versa
						if (System.getProperty("file.separator").equalsIgnoreCase("\\")) {
							initialProductions[j] = initialProductions[j].replaceAll("/", "\\\\");
						} else if (System.getProperty("file.separator").equalsIgnoreCase("/")) {
							initialProductions[j] = initialProductions[j].replaceAll("\\\\", "/");
						}
						
						initialColors[j] = agent.getAttributeThrows(kParamColor);
					}
				} else {
					// Throw during development, but really we should just ignore this
					// when reading XML (in case we add tags later and load this into an earlier version)
					throw new Exception("Unknown tag " + tagName) ;
				}
			}				
		} catch (Exception e) {
			System.out.println("Error loading XML settings: " + e.getMessage());
			shutdown();
			System.exit(1);
		}

		m_CurrentMap = getMapPath() + m_DefaultMap;

		// Load default world
		m_World = new World(this);	
		m_Logger.log("Attempting to load " + m_CurrentMap);
		if (!m_World.load(m_CurrentMap)) {
			shutdown();
			System.exit(1);
		}
		fireSimulationEvent(SimulationListener.kNewWorldEvent);
		
		// add initial eaters
		if (initialNames != null) {
			for (int i = 0; i < initialNames.length; ++i) {
				createEater(initialNames[i], getAgentPath() + initialProductions[i], initialColors[i]);
			}
		}
		
		// if in quiet mode, run!
		if (quiet) {
	    	m_Kernel.RunAllAgentsForever();
		}
	}
	
	public String getCurrentMap() {
		return m_CurrentMap;
	}
	
	public void setRuns(int runs) {
		if (runs < 0) {
			runs = -1;
		}
		m_Runs = runs;
	}
	
	public int getRuns() {
		return m_Runs;
	}
	
	public void setSpawnDebuggers(boolean mode) {
		m_Debuggers = mode;
	}
	
	public boolean getSpawnDebuggers() {
		return m_Debuggers;
	}
	
	void initSoar() {
		// Create kernel
		try {
			m_Kernel = Kernel.CreateKernelInNewThread("SoarKernelSML", 12121);
		} catch (Exception e) {
			m_Logger.log("Exception while creating kernel: " + e.getMessage());
			System.exit(1);
		}

		if (m_Kernel.HadError()) {
			m_Logger.log("Error creating kernel: " + m_Kernel.GetLastErrorDescription());
			System.exit(1);
		}

		// Register for events
		m_Kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_START, this, null);
		m_Kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_STOP, this, null);
		m_Kernel.RegisterForUpdateEvent(smlUpdateEventId.smlEVENT_AFTER_ALL_OUTPUT_PHASES, this, null);
	}

	public boolean waitForDebugger() {
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
	
  	public void updateEventHandler(int eventID, Object data, Kernel kernel, int runFlags) {
  		if (m_StopSoar) {
  			m_StopSoar = false;
  			m_Kernel.StopAllAgents();
  		}
		//m_Logger.log("Update event received from kernel.");
		m_World.update();
		++m_WorldCount;
		fireSimulationEvent(SimulationListener.kUpdateEvent);
  	}
  	
  	public int getWorldCount() {
  		return m_WorldCount;
  	}
  	
    public void systemEventHandler(int eventID, Object data, Kernel kernel) {
  		if (eventID == smlSystemEventId.smlEVENT_SYSTEM_START.swigValue()) {
  			// Start simulation
  			m_Logger.log("Start event received from kernel.");
  			fireSimulationEvent(SimulationListener.kStartEvent);
  		} else if (eventID == smlSystemEventId.smlEVENT_SYSTEM_STOP.swigValue()) {
  			// Stop simulation
  			m_Logger.log("Stop event received from kernel.");
  			fireSimulationEvent(SimulationListener.kStopEvent);	
 		} else {
 			m_Logger.log("Unknown system event received from kernel: " + eventID);
 		}
    }
    
    public void createEater(String name, String productions, String color) {
		Agent agent = createAgent(name, productions);
		if (agent == null) {
			return;
		}
		m_World.createEater(agent, productions, color);
		spawnDebugger(name);		
		fireSimulationEvent(SimulationListener.kAgentCreatedEvent);   	
    }
        
    private Agent createAgent(String name, String productions) {
    	Agent agent = m_Kernel.CreateAgent(name);
    	boolean load = agent.LoadProductions(productions);
    	if (!load || agent.HadError()) {
    		m_Logger.log("Error creating agent " + name + 
    				" (" + productions + 
    				"): " + agent.GetLastErrorDescription());
    		return null;
    	}
    	return agent;
    }
        
    public void run() {
    	do {
    		m_StopSoar = false;
    		m_Kernel.RunAllAgentsForever();
			resetSimulation();
    	} while (m_Runs != 0);
    }
    
	public String getAgentPath() {
		return m_BasePath + kAgentFolder + System.getProperty("file.separator");
	}
	
	public String getMapPath() {
		return m_BasePath + kMapFolder + System.getProperty("file.separator");
	}
	
	public World getWorld() {
		return m_World;
	}
	
	public void spawnDebugger(String agentName) {
		if (!m_Debuggers) return;
		if (m_DebuggerSpawned) return;
		
		Runtime r = java.lang.Runtime.getRuntime();
		try {
			// TODO: manage the returned process a bit better.
			// BUGBUG: javaw not portable!
			r.exec("javaw -jar " + m_Kernel.GetLibraryLocation() + System.getProperty("file.separator")
					+ "bin" + System.getProperty("file.separator") 
					+ "SoarJavaDebugger.jar -remote -agent " + agentName);
			
			if (!waitForDebugger()) {
				m_Logger.log("Debugger spawn failed for agent: " + agentName);
				return;
			}
			
		} catch (java.io.IOException e) {
			m_Logger.log("IOException spawning debugger: " + e.getMessage());
			shutdown();
			System.exit(1);
		}
		
		m_Logger.log("Spawned debugger for " + agentName);
		m_DebuggerSpawned = true;
	}
	
	public void shutdown() {
		m_Logger.log("Shutdown called.");
		fireSimulationEvent(SimulationListener.kShutdownEvent);
		if (m_World != null) {
			m_World.destroyAllEaters();
		}
		if (m_Kernel != null) {
			m_Kernel.Shutdown();
			m_Kernel.delete();
		}
	}
	
	public void startSimulation() {
        m_RunThread = new Thread(this);    
        m_StopSoar = false;
        m_RunThread.start();
	}
	
	public void stepSimulation() {
        m_Kernel.RunAllAgents(1);
	}
	
	public void stopSimulation() {
		if (m_Runs > 0) {
			--m_Runs;
		}
		
		if (m_Runs == 0) {
			m_RunThread = null;
		}
		m_StopSoar = true;
	}
	
	public boolean isRunning() {
		return (m_RunThread != null);
	}

	public void resetSimulation() {
		if (!m_World.load(m_CurrentMap)) {
			// TODO: this is not graceful error handling
			shutdown();
			System.exit(1);
		}
		m_WorldCount = 0;
		fireSimulationEvent(SimulationListener.kNewWorldEvent);
	}

	public void changeMap(String map) {
		m_CurrentMap = map;
		resetSimulation();
	}

	void destroyEater(Eater eater) {
		m_Kernel.DestroyAgent(eater.getAgent());
		eater.getAgent().delete();
		if (m_World.getEaters() == null) {
			m_DebuggerSpawned = false;
		}
		fireSimulationEvent(SimulationListener.kAgentDestroyedEvent);
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