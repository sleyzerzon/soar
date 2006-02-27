package eaters;

import simulation.*;
import sml.*;
import utilities.*;

public class EatersSimulation extends Simulation {
	public static final int kMaxEaters = 8;	
	
	private static final String kTagEaters = "eaters";
	private static final String kTagSimulation = "simulation";
	private static final String kParamDebuggers = "debuggers";
	private static final String kParamDefaultMap = "default-map";
	private static final String kParamRuns = "runs";
	private static final String kTagAgents = "agents";
	private static final String kParamName = "name";
	private static final String kParamProductions = "productions";
	private static final String kParamColor = "color";
	private static final String kDefaultMap = "default.emap";
		
	private World m_World;
	private String m_CurrentMap;
	private String m_DefaultMap;

	public EatersSimulation(String settingsFile, boolean quiet) {		
		
		// Log the settings file
		m_Logger.log("Settings file: " + settingsFile);

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
					setSpawnDebuggers(child.getAttributeBooleanDefault(kParamDebuggers, true));
					m_DefaultMap = child.getAttribute(kParamDefaultMap);
					if (m_DefaultMap == null) {
						m_DefaultMap = kDefaultMap;
					}
					setRuns(child.getAttributeIntDefault(kParamRuns, 0));
					
					m_Logger.log("Default map: " + m_DefaultMap);
					
				} else if (tagName.equalsIgnoreCase(kTagAgents)) {
					initialNames = new String[child.getNumberChildren()];
					initialProductions = new String[child.getNumberChildren()];
					initialColors = new String[child.getNumberChildren()];
					for (int j = 0; j < initialNames.length; ++j) {
						JavaElementXML agent = child.getChild(j);
						
						initialNames[j] = agent.getAttribute(kParamName);
						initialProductions[j] = agent.getAttribute(kParamProductions);
						
						// Next two lines kind of a hack.  Convert / to \\ on windows, and vice versa
						if (System.getProperty("file.separator").equalsIgnoreCase("\\")) {
							initialProductions[j] = initialProductions[j].replaceAll("/", "\\\\");
						} else if (System.getProperty("file.separator").equalsIgnoreCase("/")) {
							initialProductions[j] = initialProductions[j].replaceAll("\\\\", "/");
						}
						
						initialColors[j] = agent.getAttribute(kParamColor);
					}
				} else {
					// Throw during development, but really we should just ignore this
					// when reading XML (in case we add tags later and load this into an earlier version)
					throw new Exception("Unknown tag " + tagName) ;
				}
			}				
		} catch (Exception e) {
			fireErrorMessage("Error loading XML settings: " + e.getMessage());
			shutdown();
			System.exit(1);

		}

		m_CurrentMap = getMapPath() + m_DefaultMap;

		// Load default world
		m_World = new World(this);
		setWorldManager(m_World);
		resetSimulation(false);
		
		// add initial eaters
		if (initialNames != null) {
			for (int i = 0; i < initialNames.length; ++i) {
				createEater(initialNames[i], getAgentPath() + initialProductions[i], initialColors[i]);
			}
		}
		
		// if in quiet mode, run!
		if (quiet) {
	    	startSimulation(false);
		}
	}
	
	public String getCurrentMap() {
		return m_CurrentMap;
	}
	
	public void resetSimulation(boolean fallBackToDefault) {
		String fatalErrorMessage = null;
		if (!m_World.load(m_CurrentMap)) {
			if (fallBackToDefault) {
				fireErrorMessage("Error loading map, check log for more information. Loading default map.");
				// Fall back to default map
				m_CurrentMap = getMapPath() + m_DefaultMap;
				if (!m_World.load(m_CurrentMap)) {
					fatalErrorMessage = "Error loading default map, closing Eaters.";
				}
			} else {
				fatalErrorMessage = "Error loading map, check log for more information. Loading default map.";
			}
		}
		if (fatalErrorMessage != null) {
			fireErrorMessage(fatalErrorMessage);
			shutdown();
			System.exit(1);
		}
		super.resetSimulation();
	}

    public void createEater(String name, String productions, String color) {
    	if (name == null || productions == null) {
    		fireErrorMessage("Failed to create agent, name, productions or color null.");
    		return;
    	}
    	
		Agent agent = createAgent(name, productions);
		if (agent == null) {
			return;
		}
		m_World.createEater(agent, productions, color);
		spawnDebugger(name);		
		fireSimulationEvent(SimulationListener.kAgentCreatedEvent);   	
    }
        
	public World getWorld() {
		return m_World;
	}
	
	public void changeMap(String map) {
		m_CurrentMap = map;
		resetSimulation(true);
	}

	public void destroyEater(Eater eater) {
		if (eater == null) {
    		m_Logger.log("Asked to destroy null agent, ignoring.");
    		return;
		}	
		m_World.destroyEater(eater);
		fireSimulationEvent(SimulationListener.kAgentDestroyedEvent);
	}
	
}