/********************************************************************************************
*
* DebuggerWindow.java
* 
* Created on 	Nov 12, 2003
*
* @author 		Doug
* @version
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package modules;

import manager.MainWindow;
import manager.Pane;

import org.eclipse.swt.*;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.graphics.* ;
import org.eclipse.swt.events.*;

import doc.* ;
import doc.events.*;
import debugger.* ;

import sml.Agent;

/********************************************************************************************
* 
* The functionality that a debugger window needs to support.
* 
********************************************************************************************/
public abstract class AbstractView implements AgentFocusListener
{	
	/** The document is used to represent the Soar process.  There is one shared document
	 *  for the entire debugger. **/
	protected Document 		m_Document ;
	
	/** The main frame that owns this window.  **/
	protected MainFrame		m_MainFrame ;
	
	/** The window that owns this view */
	protected Pane			m_Pane ;

	/** The name of this view -- this name is unique within the frame (not the entire debugger) so we can cross-reference based on this name */
	protected String		m_Name ;
	
	/********************************************************
	 * All AbstractView's need to have a default constructor
	 * as that's how we build them (using reflection) when
	 * loading/saving or picking up module plug-ins.
	 *********************************************************/
	public AbstractView()
	{
	}
		
	/** The main frame that owns this window.  **/
	public MainFrame getMainFrame() { return m_MainFrame; }

	/** The document is used to represent the Soar process.  There is one shared document for the entire debugger. **/
	public Document getDocument() 	{ return m_Document ; }

	/** The name of this view -- this name is unique within the frame (not the entire debugger) so we can cross-reference based on this name */
	public String	getName() 		{ return m_Name ; }

	/** The window that owns this view */
	public Pane getPane() 			{ return m_Pane ; }
	
	/** This method is called when we initialize the module. */
	protected void setValues(MainFrame frame, Document doc, Pane parentPane)
	{
		m_MainFrame = frame ;
		m_Document  = doc ;
		m_Pane 	    = parentPane ;
	}
	
	/** Generates a unique name for this window */
	public void generateName()
	{
		m_Name = m_MainFrame.generateName(getModuleBaseName(), this) ;
	}

	/********************************************************************************************
	 * 
	 * Returns the agent that is associated with the main frame.
	 * A given window can choose to override this and work with a different agent.
	 * 
	 * This can return null if no agent is currently selected in the main frame.
	 * 
	********************************************************************************************/
	public Agent getAgentFocus()
	{
		return m_MainFrame.getAgentFocus() ;
	}

	/********************************************************************************************
	* 
	* This "base name" is used to generate a unique name for the window.
	* For example, returning a base name of "trace" would lead to windows named
	* "trace1", "trace2" etc.
	* 
	********************************************************************************************/
	public abstract String getModuleBaseName() ;

	/********************************************************************************************
	* 
	* Change the font we use to display text items in this window.
	* 
	********************************************************************************************/
	public abstract void setTextFont(Font f) ;
		
	/********************************************************************************************
	* 
	* Initialize this window and its children.
	* Should call setValues() and generateName() at the start to complete initialization of the abstract view.
	* 
	********************************************************************************************/
	public abstract void init(MainFrame frame, Document doc, Pane parentPane) ;
	
	/************************************************************************
	* 
	* Converts this object into an XML representation.
	* 
	*************************************************************************/
	public abstract general.ElementXML convertToXML(String tagName) ;

	/************************************************************************
	* 
	* Execute a command (send it to Soar) and display the output in a manner
	* appropriate to this view.
	* 
	* @param Command		The command line to execute
	* @param echoCommand	If true, display the command in the output window as well.
	* 
	* The result (if any) is also returned to the caller.
	* 
	*************************************************************************/
	public abstract String executeAgentCommand(String command, boolean echoCommand) ;

	/************************************************************************
	* 
	* Display the given text in this view (if possible).
	* 
	* This method is used to programmatically insert text that Soar doesn't generate
	* into the output window.
	* 
	*************************************************************************/
	public abstract void displayText(String text) ;

	/************************************************************************
	* 
	* Return true from a subclass if the window is a trace window.  We'll send
	* menu commands (like "source file") to this window and display the results of
	* the command here.  Multiple windows can return true in which case the first is
	* selected (currently).
	* 
	*************************************************************************/
	public boolean canBePrimeWindow() { return false ; }
	
	/************************************************************************
	* 
	* Set the focus to this window so the user can type commands easily.
	* Return true if this window wants the focus.
	* 
	*************************************************************************/
	public abstract boolean setFocus() ;
	public abstract boolean hasFocus() ;

	/************************************************************************
	* 
	* Rebuild the object from an XML representation.
	* 
	* @param frame			The top level window that owns this window
	* @param doc			The document we're rebuilding
	* @param parent			The pane window that owns this view
	* @param element		The XML representation of this command
	* 
	*************************************************************************/
	public abstract void loadFromXML(MainFrame frame, doc.Document doc, Pane parent, general.ElementXML element) throws Exception ;
	
	protected abstract void registerForAgentEvents(Agent agent) ;
	
	protected abstract void unregisterForAgentEvents(Agent agent) ;
	
	public void agentGettingFocus(AgentFocusEvent e)
	{
		if (e.getAgent() != null)
			registerForAgentEvents(e.getAgent()) ;
	}
	
	public void agentLosingFocus(AgentFocusEvent e)
	{
		// We may be passed "null" for the agent losing focus if the
		// agent is no longer valid to access (e.g. we shutdown the kernel).
		// We still get a notification in case we need to do any other clean-up.
		unregisterForAgentEvents(e.getAgent()) ;
	}
}


