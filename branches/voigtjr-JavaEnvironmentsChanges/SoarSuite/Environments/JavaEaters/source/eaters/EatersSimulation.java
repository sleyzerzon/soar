package eaters;

import doc.Document;

public class EatersSimulation {
	public static final String kTagEaters = "eaters";
	public static final String kTagSimulation = "simulation";
	public static final String kParamNoDebuggers = "nodebuggers";
	public static final String kParamDefaultMap = "default-map";
	public static final String kTagAgents = "agents";
	public static final String kTagAgent = "agent";
	public static final String kParamName = "name";
	public static final String kParamProductions = "productions";
		
	public static final String kGroupFolder = "Environments";
	public static final String kProjectFolder = "JavaEaters";
	public static final String kAgentFolder = "agents";
	public static final String kMapFolder = "maps";
	
	public static Logger logger = new Logger();

	protected boolean m_NoDebuggers = false;
	protected String m_DefaultMap;
	
	protected Document m_Document;
	protected String m_BasePath;
	protected World m_Map;
	protected Eater[] m_Eaters;
	
	public EatersSimulation(String settingsFile) {		
		// Log the settings file
		logger.log("Settings file: " + settingsFile);

		// Initialize Soar
		m_Document = new Document();
		m_Document.init();
		
		// Generate base path
		// TODO: chop instead of using ..
		m_BasePath = new String(m_Document.getLibraryLocation());
		m_BasePath += System.getProperty("file.separator")
		+ ".." + System.getProperty("file.separator") 
		+ kGroupFolder + System.getProperty("file.separator") 
		+ kProjectFolder + System.getProperty("file.separator");

		logger.log("Base path: " + m_BasePath);
		
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
					m_NoDebuggers = child.getAttributeBooleanThrows(kParamNoDebuggers);
					m_DefaultMap = child.getAttributeThrows(kParamDefaultMap);
					
					logger.log("Default map: " + m_DefaultMap);
					
				} else if (tagName.equalsIgnoreCase(kTagAgents)) {
					m_Eaters = new Eater[child.getNumberChildren()];
					for (int j = 0; j < m_Eaters.length; ++j) {
						JavaElementXML agent = child.getChild(j);
						m_Eaters[j] = new Eater(
								agent.getAttributeThrows(kParamName),
								agent.getAttributeThrows(kParamProductions));
					}
				} else {
					// Throw during development, but really we should just ignore this
					// when reading XML (in case we add tags later and load this into an earlier version)
					throw new Exception("Unknown tag " + tagName) ;
				}
			}				
		} catch (Exception e) {
			System.out.println("Error loading XML settings: " + e.getMessage());
			System.exit(1);
		}

		// Load default map
		String mapFile = getMapPath() + m_DefaultMap;
		logger.log("Attempting to load " + mapFile);
		m_Map = new World(mapFile);
	}
	
	public String getAgentPath() {
		return m_BasePath + kAgentFolder + System.getProperty("file.separator");
	}
	
	public String getMapPath() {
		return m_BasePath + kMapFolder + System.getProperty("file.separator");
	}	
}