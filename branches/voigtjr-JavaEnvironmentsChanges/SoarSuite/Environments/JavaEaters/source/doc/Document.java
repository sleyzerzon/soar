package doc;

import sml.Kernel;
import sml.smlSystemEventId;
import sml.smlUpdateEventId;
import eaters.Logger;

public class Document implements Kernel.UpdateEventInterface, Kernel.SystemEventInterface {
	
	Kernel m_Kernel;
	Logger m_Logger;
	
	public Document (Logger logger) {
		m_Logger = logger;
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
	}
	
	public String getLibraryLocation() {
		return m_Kernel.GetLibraryLocation();
	}
	
  	public void updateEventHandler(int eventID, Object data, Kernel kernel, int runFlags) {
  		
  	}
  	
    public void systemEventHandler(int eventID, Object data, Kernel kernel) {
    	
    }
}
