package eaters;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;

import doc.Document;

public class EatersSimulation {
	
	protected EatersSettings m_Settings;	
	protected Document m_Document;
	protected Logger m_Logger = new Logger();
	protected String m_BasePath;
	
	public EatersSimulation(EatersSettings settings) {		
		m_Settings = settings;
		m_Document = new Document(m_Settings, m_Logger);
		
		// Log the command line
		m_Logger.log(m_Settings.getCommandLine());

		// Initialize Soar
		//m_Document.init();
		
		// Generate base path
		m_BasePath = new String(m_Document.getLibraryLocation());
		m_BasePath += System.getProperty("file.separator")
		+ ".." + System.getProperty("file.separator") 
		+ EatersSettings.kGroupFolder + System.getProperty("file.separator") 
		+ EatersSettings.kProjectFolder + System.getProperty("file.separator");

		// Load default map
		String mapFile = getMapPath() + m_Settings.getMap();
		m_Logger.log("Attempting to load " + mapFile);
		loadMap(new File(mapFile));
	}
	
	public String getAgentPath() {
		return m_BasePath + EatersSettings.kAgentFolder + System.getProperty("file.separator");
	}
	
	public String getMapPath() {
		return m_BasePath + EatersSettings.kMapFolder + System.getProperty("file.separator");
	}	
	
	public EatersSettings getSettings() {
		return m_Settings;
	}
	
	public void loadMap(File file) {
		
		BufferedReader bIn = null;
		
		try {
			bIn = new BufferedReader(new FileReader(file));
		} catch (FileNotFoundException f) {
			m_Logger.log("File not found exception: " + f.getMessage());
			System.exit(1);
		}
		
		// TODO: create map
		m_Logger.log("Map loaded.");
		
		try {
			bIn.close();
		} catch (IOException ignored) {
		}
		
		// TODO: events
		//fireNewMapNotification(null);
	}

}