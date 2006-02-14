package eaters;

import java.util.ArrayList;

import eaters.JavaElementXML;

public class EatersSettings {
	
	// Constants
	public static final String kGroupFolder = "Environments";
	public static final String kProjectFolder = "JavaEaters";
	public static final String kAgentFolder = "agents";
	public static final String kMapFolder = "maps";
	public static final String kSoarExt = ".soar";
	public static final String kSeaterExt = ".seater";
	public static final String kDefaultMap = "default.emap";
	public static final String kDefaultXMLSettingsFile = "eaters-default-settings.xml";
	
	// Manipulatable constants
	public static int kMapWidth;
	public static int kMapHeight;

	protected boolean m_Quiet = false;
	protected boolean m_NoDebuggers = false;

	protected String m_SettingsFile;
	protected String[] m_SoarFiles;
	protected String[] m_SeaterFiles;

	// XML Tags
	public static final String kTagEaters = "eaters";
	public static final String kTagGeneral = "general";
	
	// XML Params
	public static final String kParamNoDebuggers = "nodebuggers";
	
	// Command line settings
	public boolean parseCommandLine(String[] args) {
		if (hasOption(args, "-?") || hasOption(args, "-help") || hasOption(args, "-h")) {
			printCommandLineHelp();
			return false;
		}

		m_Quiet = hasOption(args, "-quiet");
		m_NoDebuggers = hasOption(args, "-" + kParamNoDebuggers);
		m_SettingsFile = getOptionValue(args, "-settings");
		
		gatherFiles(args, kSoarExt, m_SoarFiles);	
		gatherFiles(args, kSeaterExt, m_SeaterFiles);	

		if (bootstrapXMLFile()) return true;
		
		if (m_SettingsFile == null) {
			m_SettingsFile = kDefaultXMLSettingsFile;
		}
		try {
			JavaElementXML root = JavaElementXML.ReadFromFile(m_SettingsFile);
			if (!root.getTagName().equalsIgnoreCase(kTagEaters)) {
				throw new Exception("Top level tag not " + kTagEaters);
			}
			loadSettingsFromXML(root);
		} catch (Exception e) {
			System.out.println("Error loading XML settings: " + e.getMessage());
			System.exit(1);
		}
		return true;
	}

	protected void loadSettingsFromXML(JavaElementXML element) throws Exception {
		for (int i = 0 ; i < element.getNumberChildren() ; ++i)
		{
			JavaElementXML child = element.getChild(i) ;

			String tagName = child.getTagName() ;
			
			if (tagName.equalsIgnoreCase(kTagGeneral)) {
				m_NoDebuggers = child.getAttributeBooleanThrows(kParamNoDebuggers);
			} else if (tagName.equalsIgnoreCase(Map.kTagMap)) {
				Map.loadSettingsFromXML(child);
			} else if (tagName.equalsIgnoreCase(Food.kTagFood)) {
				Food.loadSettingsFromXML(child);
			} else {
				// Throw during development, but really we should just ignore this
				// when reading XML (in case we add tags later and load this into an earlier version)
				throw new Exception("Unknown tag " + tagName) ;
			}
		}				
	}

	protected boolean bootstrapXMLFile() {
		return false;
		//saveSettingsToXML();
		//return true;
	}
	
	protected void saveSettingsToXML() {
		JavaElementXML root = new JavaElementXML(kTagEaters);
		JavaElementXML general = new JavaElementXML(kTagGeneral);
		general.addAttribute(kParamNoDebuggers, m_NoDebuggers ? "true" : "false");
		root.addChildElement(general);

		root.addChildElement(Map.saveSettingsToXML());
		
		root.addChildElement(Food.saveSettingsToXML());
		
		try {
			root.WriteToFile(kDefaultXMLSettingsFile);
		} catch (Exception ignored) {
			
		}
	}

	protected void gatherFiles(String[] args, String ext, String[] array) {
		ArrayList list = new ArrayList();
		for (int i = 0; i < args.length; ++i) {
			if (args[i].toLowerCase().endsWith(ext)) {
				list.add(args[i]);
			}
		}
		if (list.size() > 0) {
			array = (String[]) list.toArray(new String[0]);
		}		
	}
	
	protected void printCommandLineHelp() {
		System.out.println("Java Eaters help");
		System.out.println("\t*.seater: Names of .seater files for eater agents.");
		System.out.println("\t*.soar: Names of .soar files for soar agents.");
		System.out.println("\t-quiet: Disables all windows, runs simulation quietly.");
		System.out.println("\t-" + kParamNoDebuggers + ": Disables debugger spawn on agent creation.");
		System.out.println("\t-settings" + ": XML file in data dir with run settings.");
	}
	
	public String getCommandLine() {
		String ret = new String();
		if (m_Quiet) ret += "-quiet ";
		if (m_NoDebuggers) ret += "-" + kParamNoDebuggers + " ";
		
		if (m_SoarFiles != null) {
			for (int i = 0; i < m_SoarFiles.length; ++i) ret += m_SoarFiles[i] + " ";
		}
			
		if (m_SeaterFiles != null) {
			for (int i = 0; i < m_SeaterFiles.length; ++i) ret += m_SeaterFiles[i] + " ";
		}
		
		if (m_SettingsFile != null) {
			ret += "-settings " + m_SettingsFile;
		}			
		return ret;
	}

	// Returns true if a given option appears in the list
	// (Use this for simple flags like -remote)
	protected boolean hasOption(String[] args, String option) {
		for (int i = 0 ; i < args.length ; i++){
			if (args[i].equalsIgnoreCase(option))
				return true ;
		}
		return false ;
	}	
	
	// Returns the next argument after the matching option.
	// (Use this for parameters like -port ppp)
	protected String getOptionValue(String[] args, String option) {
		for (int i = 0 ; i < args.length-1 ; i++) {
			if (args[i].equalsIgnoreCase(option))
				return args[i+1] ;
		}		
		return null ;
	}

	public boolean getNoDebuggers() {
		return m_NoDebuggers;
	}

	public void setNoDebuggers(boolean noDebuggers) {
		m_NoDebuggers = noDebuggers;
	}

	public boolean isQuiet() {
		return m_Quiet;
	}
	
	public String getMap() {
		return kDefaultMap;
	}
}
