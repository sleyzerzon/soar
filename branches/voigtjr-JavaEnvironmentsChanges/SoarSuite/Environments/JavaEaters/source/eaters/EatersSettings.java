//package eaters;
//
//import java.util.ArrayList;
//
//import eaters.JavaElementXML;
//
//public class EatersSettings {
//	
//	// Constants
//	public static final String kSoarExt = ".soar";
//	public static final String kDefaultMap = "default.xml";
//	
//
//	protected String m_SettingsFile;
//	protected String[] m_SoarFiles;
//	protected String[] m_SeaterFiles;
//
//	// XML Tags
//	
//	// XML Params
//	
//	// Command line settings
//	public boolean parseCommandLine(String[] args) {
//		if (hasOption(args, "-?") || hasOption(args, "-help") || hasOption(args, "-h")) {
//			printCommandLineHelp();
//			return false;
//		}
//
//		m_Quiet = hasOption(args, "-quiet");
//		m_NoDebuggers = hasOption(args, "-" + kParamNoDebuggers);
//		m_SettingsFile = getOptionValue(args, "-settings");
//		
//		gatherFiles(args, kSoarExt, m_SoarFiles);	
//		gatherFiles(args, kSeaterExt, m_SeaterFiles);	
//
//		if (bootstrapXMLFile()) return true;
//		
//		if (m_SettingsFile == null) {
//			m_SettingsFile = kDefaultXMLSettingsFile;
//		}
//		try {
//			JavaElementXML root = JavaElementXML.ReadFromFile(m_SettingsFile);
//			if (!root.getTagName().equalsIgnoreCase(kTagEaters)) {
//				throw new Exception("Top level tag not " + kTagEaters);
//			}
//			loadSettingsFromXML(root);
//		} catch (Exception e) {
//			System.out.println("Error loading XML settings: " + e.getMessage());
//			System.exit(1);
//		}
//		return true;
//	}
//
//	protected void loadSettingsFromXML(JavaElementXML element) throws Exception {
//	}
//
//	protected boolean bootstrapXMLFile() {
//		return false;
//		//saveSettingsToXML();
//		//return true;
//	}
//	
//	protected void saveSettingsToXML() {
//		JavaElementXML root = new JavaElementXML(kTagEaters);
//		JavaElementXML general = new JavaElementXML(kTagGeneral);
//		general.addAttribute(kParamNoDebuggers, m_NoDebuggers ? "true" : "false");
//		root.addChildElement(general);
//
//		root.addChildElement(World.saveSettingsToXML());
//		
//		root.addChildElement(FoodInfo.saveSettingsToXML());
//		
//		try {
//			root.WriteToFile(kDefaultXMLSettingsFile);
//		} catch (Exception ignored) {
//			
//		}
//	}
//
//	protected void gatherFiles(String[] args, String ext, String[] array) {
//		ArrayList list = new ArrayList();
//		for (int i = 0; i < args.length; ++i) {
//			if (args[i].toLowerCase().endsWith(ext)) {
//				list.add(args[i]);
//			}
//		}
//		if (list.size() > 0) {
//			array = (String[]) list.toArray(new String[0]);
//		}		
//	}
//	
//	public String getCommandLine() {
//		String ret = new String();
//		if (m_Quiet) ret += "-quiet ";
//		if (m_NoDebuggers) ret += "-" + kParamNoDebuggers + " ";
//		
//		if (m_SoarFiles != null) {
//			for (int i = 0; i < m_SoarFiles.length; ++i) ret += m_SoarFiles[i] + " ";
//		}
//			
//		if (m_SeaterFiles != null) {
//			for (int i = 0; i < m_SeaterFiles.length; ++i) ret += m_SeaterFiles[i] + " ";
//		}
//		
//		if (m_SettingsFile != null) {
//			ret += "-settings " + m_SettingsFile;
//		}			
//		return ret;
//	}
//
//	public boolean getNoDebuggers() {
//		return m_NoDebuggers;
//	}
//
//	public void setNoDebuggers(boolean noDebuggers) {
//		m_NoDebuggers = noDebuggers;
//	}
//
//	public boolean isQuiet() {
//		return m_Quiet;
//	}
//	
//	public String getMap() {
//		return kDefaultMap;
//	}
//}
