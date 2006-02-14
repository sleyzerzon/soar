package base;

import sml.Kernel;
import sml.smlSystemEventId;
import sml.smlUpdateEventId;
import base.Logger;
import java.io.IOException;
import java.util.Random;

public abstract class SimulationControl implements Kernel.UpdateEventInterface,
		Kernel.SystemEventInterface {
	
	public Logger logger = null;
	
	protected Kernel kernel = null;
	protected String basePath = null;
	protected Object[][] map = null;
	protected Random random = new Random();
	
	private Settings m_Settings;

	protected SimulationControl(Settings settings) {
		m_Settings = settings;
		try {
			logger = new Logger(m_Settings.logfile, m_Settings.clearlog, m_Settings.loglevel);
		} catch(IOException e) {
			// TODO: Warn
		}
		
		map = new Object[m_Settings.MAP_WIDTH][m_Settings.MAP_HEIGHT];
	}
	
	protected void initSoar() {
		
		// Create kernel
		try {
			kernel = Kernel.CreateKernelInNewThread("SoarKernelSML", 12121);
		} catch (Exception e) {
			logger.log("Exception while creating kernel: " + e.getMessage());
			System.exit(1);
		}

		if (kernel.HadError()) {
			logger.log("Error creating kernel: " + kernel.GetLastErrorDescription());
			System.exit(1);
		}

		// Register for events
		kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_START, this, null);
		kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_STOP, this, null);
		kernel.RegisterForUpdateEvent(smlUpdateEventId.smlEVENT_AFTER_ALL_OUTPUT_PHASES, this, null);
	}
	
	protected void generateBasePath(String folderName) {
		// Generate base path
		basePath = new String(kernel.GetLibraryLocation());
		basePath += System.getProperty("file.separator")
		+ ".." + System.getProperty("file.separator") 
		+ m_Settings.PROJECT_FOLDER + System.getProperty("file.separator") 
		+ folderName + System.getProperty("file.separator");
	}
	
	protected String getAgentPath() {
		return basePath + System.getProperty("file.separator") + m_Settings.AGENT_FOLDER;
	}
	
	protected String getMapPath() {
		return basePath + System.getProperty("file.separator") + m_Settings.MAP_FOLDER;
	}	
	
	public void systemEventHandler(int eventID, Object data, Kernel kernel) {
		
	}

	public void updateEventHandler(int eventID, Object data, Kernel kernel, int runFlags) {
		
	}
}
