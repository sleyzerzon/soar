/********************************************************************************************
*
* TreeTraceView.java
* 
* Description:	Represents trace output using a tree, so we can capture trace information
* 				at one level of detail and display a lesser amount of detail.
* 
* Created on 	Mar 29, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package modules;

import java.io.File;

import general.ElementXML;
import helpers.CommandHistory;
import manager.Pane;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.*;
import org.eclipse.swt.graphics.Font;
import org.eclipse.swt.widgets.*;

import sml.*;
import debugger.MainFrame;
import doc.Document;

/************************************************************************
 * 
 * Represents trace output using a tree, so we can capture trace information
 * at one level of detail and display a lesser amount of detail.
 * 
 ************************************************************************/
public class TreeTraceView extends ComboCommandBase
{
	protected Tree m_Tree ;
	
	protected int m_xmlCallback = -1 ;
	
	/** The last root (top level item) added to the tree.  We add new sub items under this */
	protected TreeItem m_LastRoot ;

	protected static final String[] kPadSpaces = { "", " ", "  ", "   ", "    ", "     " } ;
	
	public TreeTraceView()
	{
		m_ClearEachCommand = false ;
		m_UpdateOnStop = false ;
		m_ClearComboEachCommand = true ;
		m_ComboAtTop = false ;
		m_ShowTraceOutput = false ;
		m_PromptForCommands = "<Type commands here>" ;		
	}

	/** This window can be the main display window */
	public boolean canBePrimeWindow() { return true ; }

	/********************************************************************************************
	* 
	* Create the window that will display the output
	* 
	********************************************************************************************/
	protected void createDisplayControl(Composite parent)
	{
		m_Tree = new Tree(parent, SWT.BORDER) ;
		m_LastRoot = null ;
		
		createContextMenu(m_Tree) ;

		/*
		 * Test code
		Tree tree = m_Tree ;
		File [] roots = File.listRoots ();
		for (int i=0; i<roots.length; i++) {
			TreeItem root = new TreeItem (tree, 0);
			root.setText (roots [i].toString ());
			root.setData (roots [i]);
			new TreeItem (root, 0);
		}

		tree.addListener (SWT.Expand, new Listener () {
			public void handleEvent (final Event event) {
				final TreeItem root = (TreeItem) event.item;
				TreeItem [] items = root.getItems ();
				for (int i= 0; i<items.length; i++) {
					if (items [i].getData () != null) return;
					items [i].dispose ();
				}
				File file = (File) root.getData ();
				File [] files = file.listFiles ();
				if (files == null) return;
				for (int i= 0; i<files.length; i++) {
					TreeItem item = new TreeItem (root, 0);
					item.setText (files [i].getName ());
					item.setData (files [i]);
					if (files [i].isDirectory()) {
						new TreeItem (item, 0);
					}
				}
			}
		});
		*/
	}

	/********************************************************************************************
	* 
	* This "base name" is used to generate a unique name for the window.
	* For example, returning a base name of "trace" would lead to windows named
	* "trace1", "trace2" etc.
	* 
	********************************************************************************************/
	public String getModuleBaseName()
	{
		return "treetrace" ;
	}

	/** The control we're using to display the output in this case **/
	protected Control getDisplayControl()
	{
		return m_Tree ;
	}

	/************************************************************************
	* 
	* Add the text to the view in a thread safe way (switches to UI thread)
	* 
	*************************************************************************/
	protected void appendSubTextSafely(final String text)
	{
		// If Soar is running in the UI thread we can make
		// the update directly.
		if (!Document.kDocInOwnThread)
		{
			appendSubText(text) ;
			return ;
		}

		// Have to make update in the UI thread.
		// Callback comes in the document thread.
        Display.getDefault().asyncExec(new Runnable() {
            public void run() {
            	appendSubText(text) ;
            }
         }) ;
	}
	
	/************************************************************************
	* 
	* Add the text to the view (this method assumes always called from UI thread)
	* 
	*************************************************************************/
	protected void appendSubText(String text)
	{
		String[] lines = text.split(getLineSeparator()) ;

		// Add the sub items under the last root in the tree
		if (m_LastRoot == null)
		{
			m_LastRoot = new TreeItem(m_Tree, 0) ;
		}

		TreeItem lastItem = m_LastRoot ;
		
		// Stop updating the tree while we add to it
		m_Tree.setRedraw(false) ;

		for (int i = 0 ; i < lines.length ; i++)
		{	
			if (lines[i].length() == 0)
				continue ;
			
			TreeItem node = new TreeItem (lastItem, 0);
			node.setText (lines[i]);
			node.setData (null);	// No additional watch data to attach here (for later expansion)
			//new TreeItem (root, 0);
		}

		// Scroll to the bottom -- not doing this for sub items to save speed
		// Requires that we issue a final top level item at the end of a run.
		//if (lastItem != null)
		//	m_Tree.showItem(lastItem) ;

		// Redraw the updated tree
		m_Tree.setRedraw(true) ;		
	}
	
	/************************************************************************
	* 
	* Add the text to the view (this method assumes always called from UI thread)
	* 
	*************************************************************************/
	protected void appendText(String text)
	{
		String[] lines = text.split(getLineSeparator()) ;

		TreeItem lastItem = null ;
		
		// Stop updating the tree while we add to it
		m_Tree.setRedraw(false) ;
		for (int i = 0 ; i < lines.length ; i++)
		{	
			if (lines[i].length() == 0)
				continue ;
			
			TreeItem root = new TreeItem (m_Tree, 0);
			root.setText (lines[i]);
			root.setData (null);	// No additional watch data to attach here (for later expansion)
			lastItem = root ;
			//new TreeItem (root, 0);
		}

		// Scroll to the bottom -- horribly slow
		if (lastItem != null)
		{
			m_Tree.showItem(lastItem) ;
			m_LastRoot = lastItem ;
		}
		
		// Redraw the updated tree
		m_Tree.setRedraw(true) ;
	}

	/************************************************************************
	* 
	* Clear the display control.
	* 
	*************************************************************************/
	protected void clearDisplay()
	{
		
	}
	
	/** Add spaces to the length until reaches minLength */
	protected String padLeft(String orig, int minLength)
	{
		if (orig.length() >= minLength)
			return orig ;
		
		// Add the appropriate number of spaces.  If this throws
		// we need to increase the size of the kPadSpaces array.
		return kPadSpaces[minLength - orig.length()] + orig ;
	}
	
	/********************************************************************************************
	 * 
	 * This handler should only be called from the UI thread as it does a lot of UI work.
	 * 
	 * @param agent
	 * @param xmlParent
	********************************************************************************************/
	protected void displayXmlTraceEvent(Agent agent, ClientTraceXML xmlParent)
	{
		int nChildren = xmlParent.GetNumberChildren() ;
		
		for (int childIndex = 0 ; childIndex < nChildren ; childIndex++)
		{
			// Analyze the children as ClientTraceXML objects
			ClientTraceXML xmlTrace = new ClientTraceXML() ;

			// Get each child in turn
			xmlParent.GetChild(xmlTrace, childIndex) ;
			
			StringBuffer text = new StringBuffer() ;
			final int decisionDigits = 3 ;
			
			// This is a state change (new decision)
			if (xmlTrace.IsTagState())
			{
				// 3:    ==>S: S2 (operator no-change)
				text.append(padLeft(xmlTrace.GetDecisionCycleCount(), decisionDigits)) ;
				text.append(": ==>S: ") ;
				text.append(xmlTrace.GetStateID()) ;
				
				if (xmlTrace.GetImpasseObject() != null)
				{
					text.append(" [") ;
					text.append(xmlTrace.GetImpasseObject()) ;
					text.append(" ") ;
					text.append(xmlTrace.GetImpasseType()) ;
					text.append("]") ;
				}
				
				if (text.length() != 0)
					this.appendText(text.toString()) ;
			} else if (xmlTrace.IsTagOperator())
			{
				 //2:    O: O8 (move-block)
				text.append(padLeft(xmlTrace.GetDecisionCycleCount(), decisionDigits)) ;
				text.append(":    O: ") ;
				text.append(xmlTrace.GetOperatorID()) ;
				
				if (xmlTrace.GetOperatorName() != null)
				{
					text.append(" [") ;
					text.append(xmlTrace.GetOperatorName()) ;
					text.append("]") ;
				}
	
				if (text.length() != 0)
					this.appendText(text.toString()) ;
				
			} else if (xmlTrace.IsTagPhase())
			{
				text.append("--- ") ;
				text.append(xmlTrace.GetPhaseName()) ;
				text.append(" ") ;
				text.append("phase ") ;
				if (xmlTrace.GetPhaseStatus() != null)
					text.append(xmlTrace.GetPhaseStatus()) ;
				text.append("---") ;
				
				if (text.length() != 0)
					this.appendSubText(text.toString()) ;
			}
			else if (xmlTrace.IsTagAddWme() || xmlTrace.IsTagRemoveWme())
			{
				boolean adding = xmlTrace.IsTagAddWme() ;
				for (int i = 0 ; i < xmlTrace.GetNumberChildren() ; i++)
				{
					ClientTraceXML child = new ClientTraceXML() ;
					xmlTrace.GetChild(child, i) ;
					
					if (child.IsTagWme())
					{
						text.append(adding ? "=>WM: (" : "<=WM: (") ;
						text.append(child.GetWmeTimeTag()) ;
						text.append(": ") ;
						text.append(child.GetWmeID()) ;
						text.append(" ^") ;
						text.append(child.GetWmeAttribute()) ;
						text.append(" ") ;
						text.append(child.GetWmeValue()) ;
						text.append(")") ;
					}
					
					child.delete() ;
				}
				
				if (text.length() != 0)
					this.appendSubText(text.toString()) ;	

			} else if (xmlTrace.IsTagPreference())
			{
				text.append("--> (") ;
				text.append(xmlTrace.GetPreferenceID()) ;
				text.append(" ^") ;
				text.append(xmlTrace.GetPreferenceAttribute()) ;
				text.append(" ") ;
				text.append(xmlTrace.GetPreferenceValue()) ;
				text.append(" + )") ;	// BUGBUG: + hard-coded for now -- PreferenceType present but empty

								if (text.length() != 0)
					this.appendSubText(text.toString()) ;
				
			} 
			else if (xmlTrace.IsTagFiringProduction() || xmlTrace.IsTagRetractingProduction())
			{
				boolean firing = xmlTrace.IsTagFiringProduction() ;

				for (int i = 0 ; i < xmlTrace.GetNumberChildren() ; i++)
				{
					ClientTraceXML child = new ClientTraceXML() ;
					xmlTrace.GetChild(child, i) ;
					if (child.IsTagProduction())
					{
						if (i > 0)
							text.append(getLineSeparator()) ;
						
						text.append(firing ? "Firing " : "Retracting ") ;
						text.append(child.GetProductionName()) ;
					}
					child.delete() ;
				}

				if (text.length() != 0)
					this.appendSubText(text.toString()) ;			
	
			}
			else
			{
				// These lines can be helpful if debugging this
				String xmlText = xmlParent.GenerateXMLString(true) ;
				System.out.println(xmlText) ;
			}

			// Manually clean up the child too
			System.out.print('x') ;
			xmlTrace.delete() ;		
		}
		
		// Technically this will happen when the object is garbage collected (and finalized)
		// but without it we'll get a million memory leak messages because (a) gc may not have been run for a while
		// (b) even if it runs not all objects will be reclaimed and (c) finalize isn't guaranteed before we exit
		// so all in all, let's just call it ourselves here :)
		xmlParent.delete() ;
	}
	
	public void xmlEventHandler(int eventID, Object data, final Agent agent, ClientXML xml)
	{
		if (eventID != smlXMLEventId.smlEVENT_XML_TRACE_OUTPUT.swigValue())
			return ;

		// The messages come collected into a parent <trace> tag so that one event
		// can send over many pieces of a trace in a single call.  Just more
		// efficient that way.
		final ClientTraceXML xmlParent = xml.ConvertToTraceXML() ;
		if (!xmlParent.IsTagTrace() || xmlParent.GetNumberChildren() == 0)
		{
			xml.delete() ;
			return ;
		}
		
		// If Soar is running in the UI thread we can make
		// the update directly.
		if (!Document.kDocInOwnThread)
		{
			displayXmlTraceEvent(agent, xmlParent) ;
			return ;
		}

		// Have to make update in the UI thread.
		// Callback comes in the document thread.
        Display.getDefault().syncExec(new Runnable() {
            public void run() {
    			displayXmlTraceEvent(agent, xmlParent) ;
            }
         }) ;		
	}

	/********************************************************************************************
	 * 
	 * Register for events of particular interest to this view
	 * 
	 ********************************************************************************************/
	protected void registerForViewAgentEvents(Agent agent)
	{
		m_xmlCallback = agent.RegisterForXMLEvent(smlXMLEventId.smlEVENT_XML_TRACE_OUTPUT, this, "xmlEventHandler", null) ;
	}

	protected boolean unregisterForViewAgentEvents(Agent agent)
	{
		boolean ok = true ;
		
		if (m_xmlCallback != -1)
			ok = agent.UnregisterForXMLEvent(m_xmlCallback) ;

		m_xmlCallback = -1 ;
		
		return ok ;
	}

	
	protected void storeContent(ElementXML element)
	{
		
	}

	protected void restoreContent(ElementXML element)
	{
		
	}

	/********************************************************************************************
	 * 
	 * Display a dialog that allows the user to adjust properties for this window
	 * e.g. choosing whether to clear the window everytime a new command executes or not.
	 * 
	********************************************************************************************/
	public void showProperties()
	{
	}
}
