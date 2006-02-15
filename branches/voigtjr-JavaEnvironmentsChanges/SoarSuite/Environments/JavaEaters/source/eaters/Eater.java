package eaters;

import utilities.Logger;

public class Eater {
	protected String m_Name;
	protected String m_Productions;
	protected Logger m_Logger = Logger.logger;
	
	public Eater(String name, String productions) {
		m_Logger.log("Created eater: " + name + ", " + productions);
		m_Name = name;
		m_Productions = productions;
	}
}
