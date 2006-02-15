package eaters;

import sml.Agent;
import utilities.Logger;

public class Eater {
	protected Agent m_Agent;
	protected String m_Name;
	protected Logger m_Logger = Logger.logger;
	
	public Eater(Agent agent) {
		m_Agent = agent;
		m_Name = m_Agent.GetAgentName();
		m_Logger.log("Created eater: " + m_Name);
	}
}
