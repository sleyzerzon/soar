package doc;

import sml.Agent;
import sml.ConnectionInfo;
import sml.Kernel;
import sml.smlSystemEventId;
import sml.smlUpdateEventId;
import sml.sml_Names;
import utilities.Logger;
import eaters.EatersSimulation;

public class Document extends Thread implements Kernel.UpdateEventInterface, Kernel.SystemEventInterface {
	
	public static final int kDebuggerTimeoutSeconds = 15;
	
	protected Logger m_Logger = Logger.logger;
	protected Kernel m_Kernel;
	protected boolean m_AskedToShutdown = false;
	protected boolean m_StopSoar = false;
	protected boolean m_Run = false;
	protected boolean m_Step = false;	
	protected EatersSimulation m_Simulation;
	
	public Document (EatersSimulation simulation) {
		m_Simulation = simulation;
	}
	
	public void init() {
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
		
		this.start();
	}

	public boolean waitForDebugger() {
		boolean ready = false;
		for (int tries = 0; tries < Document.kDebuggerTimeoutSeconds; ++tries) {
			m_Kernel.GetAllConnectionInfo();
			if (m_Kernel.HasConnectionInfoChanged()) {
				for (int i = 0; i < m_Kernel.GetNumberConnections(); ++i) {
					ConnectionInfo info =  m_Kernel.GetConnectionInfo(i);
					if (info.GetName().equalsIgnoreCase("java-debugger")) {
						if (info.GetAgentStatus().equalsIgnoreCase(sml.sml_Names.getKStatusReady())) {
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
	
	public String getLibraryLocation() {
		return m_Kernel.GetLibraryLocation();
	}
	
  	public void updateEventHandler(int eventID, Object data, Kernel kernel, int runFlags) {
  		if (m_StopSoar) {
  			m_StopSoar = false;
  			m_Kernel.StopAllAgents();
  		}
  		m_Simulation.updateWorld();
  	}
  	
    public void systemEventHandler(int eventID, Object data, Kernel kernel) {
  		if (eventID == smlSystemEventId.smlEVENT_SYSTEM_START.swigValue()) {
  			// Start simulation
  			m_Logger.log("Start event received from kernel.");
  			m_Simulation.startSimulation();
  		} else if (eventID == smlSystemEventId.smlEVENT_SYSTEM_STOP.swigValue()) {
  			// Stop simulation
  			m_Logger.log("Stop event received from kernel.");
  			m_Simulation.stopSimulation();
 		}
		m_Logger.log("Unknown system event received from kernel: " + new Integer(eventID).toString());
    }
    
    public void stopSoar() {
    	m_StopSoar = true;
    }
    
    public Agent createAgent(String name, String productions) {
    	Agent agent = m_Kernel.CreateAgent(name);
    	boolean load = agent.LoadProductions(m_Simulation.getAgentPath() + productions);
    	if (!load || agent.HadError()) {
    		m_Logger.log("Error creating agent " + name + 
    				" (" + m_Simulation.getAgentPath() + productions + 
    				"): " + agent.GetLastErrorDescription());
    		return null;
    	}
    	return agent;
    }
    
	/** Ask this thread to halt */
	public synchronized void askToShutdown() {
		m_AskedToShutdown = true ;
	}

	public synchronized void runSoar() {
		m_Run = true ;
		interrupt();
	}

	public synchronized void stepSoar() {
		m_Step = true ;
		interrupt();
	}

    public void run() {
		while (!m_AskedToShutdown) {
			if (m_Run) {
				m_Run = false;
				m_Kernel.RunAllAgentsForever();
			} else if (m_Step) {
				m_Step = false;
				m_Kernel.RunAllAgents(1);
			}
			try { Thread.sleep(10) ; } catch (InterruptedException e) { } 
		}
		m_Logger.log("Document exiting, deleting kernel.");
		m_Kernel.Shutdown();
		m_Kernel.delete();
		m_Kernel = null;
    }
}
