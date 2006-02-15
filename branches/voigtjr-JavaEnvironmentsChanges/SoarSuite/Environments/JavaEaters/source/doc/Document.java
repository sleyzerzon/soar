package doc;

import sml.Kernel;
import sml.smlSystemEventId;
import sml.smlUpdateEventId;
import utilities.Logger;
import eaters.EatersSimulation;

public class Document extends Thread implements Kernel.UpdateEventInterface, Kernel.SystemEventInterface {
	
	protected Logger m_Logger = Logger.logger;
	protected Kernel m_Kernel;
	protected boolean m_AskedToStop = false;
	protected boolean m_Run = false;
	protected boolean m_Step = false;	
	
	public Document () {
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
	
	public String getLibraryLocation() {
		return m_Kernel.GetLibraryLocation();
	}
	
  	public void updateEventHandler(int eventID, Object data, Kernel kernel, int runFlags) {
  	}
  	
    public void systemEventHandler(int eventID, Object data, Kernel kernel) {
  		if (eventID == smlSystemEventId.smlEVENT_SYSTEM_START.swigValue()) {
  			// Start simulation
  			m_Logger.log("Start event received from kernel.");
  		} else if (eventID == smlSystemEventId.smlEVENT_SYSTEM_STOP.swigValue()) {
  			// Stop simulation
  			m_Logger.log("Stop event received from kernel.");
  		}
		m_Logger.log("Unknown system event received from kernel: " + new Integer(eventID).toString());
    }
    
	/** Ask this thread to halt */
	public synchronized void askToStop() {
		m_AskedToStop = true ;
	}

	/** Ask this thread to halt */
	public synchronized void runSoar() {
		m_Run = true ;
		interrupt();
	}

	/** Ask this thread to halt */
	public synchronized void stepSoar() {
		m_Step = true ;
		interrupt();
	}

    public void run() {
		while (!m_AskedToStop) {
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
		m_Kernel.delete();
		m_Kernel = null;
    }
}
