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
	protected MainFrame		m_Frame ;
	
	/** The window that owns this view */
	protected Pane			m_Pane ;

	/** The name of this view -- this name is unique within the frame (not the entire debugger) so we can cross-reference based on this name */
	protected String		m_Name ;
	
	/** The window which will contain all others within this view */
	protected Composite		m_Container ;
	
	/********************************************************
	 * All AbstractView's need to have a default constructor
	 * as that's how we build them (using reflection) when
	 * loading/saving or picking up module plug-ins.
	 *********************************************************/
	public AbstractView()
	{
	}
		
	/** The main frame that owns this window.  **/
	public MainFrame getMainFrame() { return m_Frame; }

	/** The document is used to represent the Soar process.  There is one shared document for the entire debugger. **/
	public Document getDocument() 	{ return m_Document ; }

	/** The name of this view -- this name is unique within the frame (not the entire debugger) so we can cross-reference based on this name */
	public String	getName() 		{ return m_Name ; }

	/** The window that owns this view */
	public Pane getPane() 			{ return m_Pane ; }
	
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
		return m_Frame.getAgentFocus() ;
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
	* Return true if this view shouldn't be user resizable.  E.g. A text window would return false
	* but a bar for buttons would return true.
	* 
	********************************************************************************************/
	public abstract boolean isFixedSizeView() ;

	/********************************************************************************************
	* 
	* Change the font we use to display text items in this window.
	* 
	********************************************************************************************/
	public abstract void setTextFont(Font f) ;
		
	/********************************************************************************************
	* 
	* Initialize this window and its children.
	* Should call setValues() at the start to complete initialization of the abstract view.
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
	* Return true if this window wants the focus (some don't have a sensible
	* place to focus on).
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
	
	/************************************************************************
	* 
	* Given a context menu and a control, fill in the items you want to 
	* see in the menu.  The simplest is to just call "fillWindowMenu".
	* 
	* This call is made after the user has clicked to bring up the menu
	* so we can create a dymanic menu based on the current context.
	* 
	* You also have to call createContextMenu() to request a context menu
	* be attached to a specific control.
	* 
	*************************************************************************/
	protected abstract void fillInContextMenu(Menu contextMenu, Control control) ;
	
	/************************************************************************
	* 
	* Register and unregister for Soar events for this agent.
	* (E.g. a trace window might register for the print event)
	* 
	*************************************************************************/
	protected abstract void registerForAgentEvents(Agent agent) ;	
	protected abstract void unregisterForAgentEvents(Agent agent) ;
	
	/** Close down this window, doing any necessary clean up */
	public void close(boolean dispose)
	{
		unregisterName() ;
		unregisterForAgentEvents(getAgentFocus()) ;

		if (dispose)
			m_Container.dispose() ;
	}
	
	/** This method is called when we initialize the module. */
	protected void setValues(MainFrame frame, Document doc, Pane parentPane)
	{
		m_Frame = frame ;
		m_Document  = doc ;
		m_Pane 	    = parentPane ;
	}
	
	/** Generates a unique name for this window */
	public void generateName(MainFrame frame)
	{
		if (m_Name != null)
			throw new IllegalStateException("Should only call this once.  If we really want to allow this then if m_Name != null unregister this name before generating a new name.") ;
		
		// Use the frame that's passed in, in case our frame pointer is yet to be initialized
		m_Name = frame.generateName(getModuleBaseName(), this) ;
	}
	
	protected void unregisterName()
	{
		m_Frame.getNameRegister().unregisterName(getName()) ;		
	}
	
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
	
	protected void addItem(Menu menu, String text, final String command)
	{
		MenuItem item = new MenuItem (menu, SWT.PUSH);
		item.setText (text) ;
		
		item.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) { m_Frame.executeDebuggerCommand(command, false) ; } } ) ;
	}	
	
	public void fillWindowMenu(Menu menu)
	{
		addItem(menu, "Add window to right ...", "addview " + m_Frame.getName() + " " + this.getName() + " " + MainWindow.kAttachRightValue) ;
		addItem(menu, "Add window to left ...", "addview " + m_Frame.getName() + " " + this.getName() + " " + MainWindow.kAttachLeftValue) ;
		addItem(menu, "Add window to top ...", "addview " + m_Frame.getName() + " " + this.getName() + " " + MainWindow.kAttachTopValue) ;
		addItem(menu, "Add window to bottom ...", "addview " + m_Frame.getName() + " " + this.getName() + " " + MainWindow.kAttachBottomValue) ;
		new MenuItem(menu, SWT.SEPARATOR) ;
		addItem(menu, "Remove window", "removeview " + m_Frame.getName() + " " + this.getName()) ;
	}
	
	protected Menu createContextMenu(final Control control)
	{
		// Create a custom context menu for the text area
		final Menu menu = new Menu (control.getShell(), SWT.POP_UP);
		menu.addMenuListener(new MenuListener() {
			public void menuShown(MenuEvent e)
			{
				// We'll build the menu dynamically based on the text the user selects etc.
				fillInContextMenu(menu, control) ;
			}
			public void menuHidden(MenuEvent e)
			{
			}
		
		}) ;
		control.setMenu (menu);
		return menu ;
	}
	
}


