/********************************************************************************************
*
* PhaseView.java
* 
* Description:	
* 
* Created on 	Sep 25, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package modules;

import general.ElementXML;
import manager.Pane;
import modules.ButtonView.ButtonInfo;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.ControlAdapter;
import org.eclipse.swt.events.ControlEvent;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.graphics.Font;
import org.eclipse.swt.layout.RowLayout;
import org.eclipse.swt.widgets.*;

import sml.Agent;
import debugger.MainFrame;
import doc.Document;

/************************************************************************
 * 
 * Displays the phases of Soar's kernel as it is executing.
 * Also allows the user to select where Soar will stop when running by decision.
 * 
 ************************************************************************/
public class PhaseView extends AbstractFixedView
{
	protected Button	m_UpdateButton ;
	protected Canvas	m_PhaseDiagram ;
	
	/********************************************************************************************
	* 
	* This "base name" is used to generate a unique name for the window.
	* For example, returning a base name of "trace" would lead to windows named
	* "trace1", "trace2" etc.
	* 
	********************************************************************************************/
	public String getModuleBaseName()
	{
		return "phaseview" ;
	}

	/********************************************************************************************
	* 
	* Initialize this window and its children.
	* Should call setValues() at the start to complete initialization of the abstract view.
	* 
	********************************************************************************************/
	public void init(MainFrame frame, Document doc, Pane parentPane)
	{
		setValues(frame, doc, parentPane) ;
		
		createPanel(m_Pane.getWindow()) ;
	}

	protected void createPanel(final Composite parent)
	{
		// Allow us to recreate the panel by calling this multiple times
		if (m_Container != null)
		{
			m_Container.dispose() ;
			m_Container = null ;
		}
		
		// The container lets us control the layout of the controls
		// within this window
		m_Container	   = new Composite(parent, SWT.NULL) ;

		RowLayout layout = new RowLayout(SWT.HORIZONTAL) ;
		m_Container.setLayout(layout) ;
		
		/*
		m_Container.addControlListener(new ControlAdapter() {
			public void controlResized(ControlEvent e)
			{
				setPrefSize() ;
		} }) ;
		*/
		
		Button check = new Button(m_Container, SWT.CHECK) ;
		check.setText("Update") ;
		m_UpdateButton = check ;
		
		Canvas canvas = new Canvas(m_Container, SWT.NULL) ;
		canvas.setSize(100, 50) ;
		m_PhaseDiagram = canvas ;
				
		// Create a context menu for the container and the diagram
		// It will be filled in via a call to fillInContextMenu when the menu is popped up
		// (this allows for dynamic content)
		createContextMenu(m_Container) ;
		createContextMenu(m_PhaseDiagram) ;
		
		// Layout the parent again, because this set of windows has changed
		// This only matters if we're recreating the windows
		parent.layout(true) ;
	}
		
	/********************************************************************************************
	 * @param contextMenu
	 * @param control
	 * @param mouseX
	 * @param mouseY
	 * 
	 * @see modules.AbstractView#fillInContextMenu(org.eclipse.swt.widgets.Menu, org.eclipse.swt.widgets.Control, int, int)
	 ********************************************************************************************/

	protected void fillInContextMenu(Menu contextMenu, Control control, int mouseX, int mouseY)
	{
		// TODO Auto-generated method stub

	}

	/********************************************************************************************
	 * @param agent
	 * 
	 * @see modules.AbstractView#registerForAgentEvents(sml.Agent)
	 ********************************************************************************************/

	protected void registerForAgentEvents(Agent agent)
	{
		// TODO Auto-generated method stub

	}

	/********************************************************************************************
	 * @param agent
	 * 
	 * @see modules.AbstractView#unregisterForAgentEvents(sml.Agent)
	 ********************************************************************************************/

	protected void unregisterForAgentEvents(Agent agent)
	{
		// TODO Auto-generated method stub

	}

	/********************************************************************************************
	 * 
	 * 
	 * @see modules.AbstractView#clearAgentEvents()
	 ********************************************************************************************/

	protected void clearAgentEvents()
	{
		// TODO Auto-generated method stub

	}

	/********************************************************************************************
	 * 
	 * 
	 * @see modules.AbstractView#showProperties()
	 ********************************************************************************************/

	public void showProperties()
	{
		// TODO Auto-generated method stub

	}

}
