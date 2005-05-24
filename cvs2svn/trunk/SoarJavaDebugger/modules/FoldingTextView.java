/********************************************************************************************
*
* FoldingTextView.java
* 
* Description:	
* 
* Created on 	May 6, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package modules;

import general.ElementXML;
import helpers.FormDataHelper;

import java.util.ArrayList;
import java.util.Iterator;

import manager.Pane;
import menu.ParseSelectedText;

import org.eclipse.swt.SWT;
import org.eclipse.swt.custom.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.graphics.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;

import sml.Agent;
import sml.ClientTraceXML;
import sml.ClientXML;
import sml.smlXMLEventId;
import debugger.MainFrame;
import dialogs.PropertiesDialog;
import doc.Document;
import helpers.* ;

/************************************************************************
 * 
 * A text view for showing trace information from Soar.
 * 
 * This text view allows for "folding" which is expanding and collapsing
 * part of the text, as if it were a tree.  Very similar to an editing
 * window in Eclipse.
 *  
 ************************************************************************/
public class FoldingTextView extends AbstractComboView
{
	protected FoldingText m_FoldingText ;
	
	protected int m_xmlCallback = -1 ;
	
	/** How many spaces we indent for a subgoal */
	protected int m_IndentSize = 3 ;
	
	/** When true, expand the tree automatically as it's created */
	protected boolean m_ExpandTracePersistent = false ;
	
	/** When true, we expand the tree as it's created -- but this one is not persistent between debugger sessions */
	protected boolean m_ExpandTrace = false ;
	
	/** The last root (top level item) added to the tree.  We add new sub items under this */
	protected TreeItem m_LastRoot ;
	
	protected Composite	m_Buttons ;

	protected Button m_ExpandButton ;
	protected Button m_ExpandPageArrow ;
	protected Menu   m_ExpandPageMenu ;
	
	protected Label  m_FilterLabel ;
	protected Button m_FilterArrow ;
	protected Menu	 m_FilterMenu ;
	
	/** Controls whether we cache strings that are due to be subtree nodes and only add the nodes when the user clicks--or not */
	protected final static boolean kCacheSubText = true ;
	
	/** We use this structure if we're caching sub nodes in the tree for expansion only when the user clicks */
	protected static class TreeData
	{
		protected ArrayList m_Lines = new ArrayList() ;
		
		public void addLine(String text) 	{ m_Lines.add(text) ; }
		public Iterator getLinesIterator() 	{ return m_Lines.iterator() ; }
	}
	
	public FoldingTextView()
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

	public Color getBackgroundColor() { return m_Frame.m_White ; }
	
	protected void updateButtonState()
	{
		m_ExpandButton.setText(m_ExpandTrace ? "Collapse" : " Expand ") ;
		m_ExpandButton.setData("expand", m_ExpandTrace ? Boolean.TRUE : Boolean.FALSE) ;
		
		// Set the checkboxes to match current filter state
		for (int i = 0 ; i < m_FilterMenu.getItemCount() ; i++)
		{
			MenuItem item = m_FilterMenu.getItem(i) ;
			
			Long typeObj = (Long)item.getData("type") ;
			if (typeObj == null)
				continue ;

			// Update the checkbox to match whether this type is visible or not
			long type = typeObj.longValue() ;
			item.setSelection((m_FoldingText.isTypeVisible(type))) ;
			
			// Enable/disable the item to match whether filtering is enabled at all
			item.setEnabled(m_FoldingText.isFilteringEnabled()) ;
		}

		// Change the color of the label if any filtering is enabled, so it's clear that this is happening.
		// Don't want to be doing this by accident and not realize it.
		if (m_FoldingText.isFilteringEnabled() && m_FoldingText.getExclusionFilter() != 0)
		{
			m_FilterLabel.setForeground(getMainFrame().getDisplay().getSystemColor(SWT.COLOR_BLUE)) ;			
		}
		else
		{
			m_FilterLabel.setForeground(getMainFrame().getDisplay().getSystemColor(SWT.COLOR_GRAY)) ;
		}
	}
	
	/********************************************************************************************
	* 
	* Create the window that will display the output
	* 
	********************************************************************************************/
	protected void createDisplayControl(Composite parent)
	{
		m_FoldingText = new FoldingText(parent) ;
		m_LastRoot = null ;
		
		createContextMenu(m_FoldingText.getTextWindow()) ;
		
		m_Buttons = new Composite(m_ComboContainer, 0) ;
		m_Buttons.setLayout(new RowLayout()) ;
		final Composite owner = m_Buttons ;
		
		// Add a button that offers an expand/collapse option instantly (for just one page)
		m_ExpandTrace = m_ExpandTracePersistent ;		
		m_ExpandButton = new Button(owner, SWT.PUSH);
		
		m_ExpandButton.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e)
		{
			m_ExpandTrace = (e.widget.getData("expand") == Boolean.FALSE) ;
			updateButtonState() ;
			
			// We expand the current page if the user asks for "expand" but we're not making this symmetric
			// because "collapse" probably means "I'm done with detailed debugging" but not necessarily "I don't want to see what I was just working on".
			// If you don't agree with that logic just comment out the "if".
			if (m_ExpandTrace)
				expandPage(m_ExpandTrace) ;
		} } ) ;
		
		m_ExpandPageArrow = new Button(owner, SWT.ARROW | SWT.DOWN) ;
		m_ExpandPageMenu  = new Menu(owner) ;

		m_ExpandPageArrow.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent event)
		{ 	Point pt = m_ExpandPageArrow.toDisplay(new Point(event.x, event.y)) ;
			m_ExpandPageMenu.setLocation(pt.x, pt.y) ;
			m_ExpandPageMenu.setVisible(true) ;
		} }) ;

		MenuItem menuItem = new MenuItem (m_ExpandPageMenu, SWT.PUSH);
		menuItem.setText ("Expand page");
		menuItem.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { expandPage(true) ; } } ) ;

		// Note: Doing expand page then collapse page doesn't get you back to where you started--the page will be showing far fewer
		// blocks when you hit "collapse page" so it does less work.  Collapse page may have little value.
		menuItem = new MenuItem (m_ExpandPageMenu, SWT.PUSH);
		menuItem.setText ("Collapse page");		
		menuItem.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { expandPage(false) ; } } ) ;

		menuItem = new MenuItem (m_ExpandPageMenu, SWT.PUSH);
		menuItem.setText ("Expand all");		
		menuItem.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { expandAll(true) ; } } ) ;

		menuItem = new MenuItem (m_ExpandPageMenu, SWT.PUSH);
		menuItem.setText ("Collapse all");		
		menuItem.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { expandAll(false) ; } } ) ;
		
		// Add a button that controls whether we are filtering or not
		Composite labelHolder = new Composite(owner, SWT.NULL) ;
		labelHolder.setLayout(new GridLayout(1, true)) ;
		
		m_FilterLabel = new Label(labelHolder, 0);
		m_FilterLabel.setText("Filters") ;

		// Place the label in the center of a tiny grid layout so we can
		// align the text to match the expand button along side
		GridData data = new GridData() ;
		data.horizontalAlignment = SWT.CENTER ;
		data.verticalAlignment = SWT.CENTER ;
		m_FilterLabel.setLayoutData(data) ;
		
		m_FilterArrow = new Button(owner, SWT.ARROW | SWT.DOWN) ;
		m_FilterMenu  = new Menu(owner) ;

		m_FilterArrow.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent event)
		{ 	Point pt = m_FilterArrow.toDisplay(new Point(event.x, event.y)) ;
			m_FilterMenu.setLocation(pt.x, pt.y) ;
			m_FilterMenu.setVisible(true) ;
		} }) ;

		menuItem = new MenuItem (m_FilterMenu, SWT.CHECK);
		menuItem.setText ("Phases") ;
		menuItem.setData("type", new Long(TraceType.kPhase)) ;
		menuItem.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { changeFilter(e.widget, TraceType.kPhase) ; } } ) ;

		menuItem = new MenuItem (m_FilterMenu, SWT.CHECK);
		menuItem.setText ("Preferences") ;
		menuItem.setData("type", new Long(TraceType.kPreference)) ;
		menuItem.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { changeFilter(e.widget, TraceType.kPreference) ; } } ) ;

		menuItem = new MenuItem (m_FilterMenu, SWT.CHECK);
		menuItem.setText ("Wme Changes") ;
		menuItem.setData("type", new Long(TraceType.kWmeChange)) ;
		menuItem.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { changeFilter(e.widget, TraceType.kWmeChange) ; } } ) ;

		menuItem = new MenuItem (m_FilterMenu, SWT.CHECK);
		menuItem.setText ("Production Firings") ;
		menuItem.setData("type", new Long(TraceType.kFiring)) ;
		menuItem.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { changeFilter(e.widget, TraceType.kFiring) ; } } ) ;

		menuItem = new MenuItem (m_FilterMenu, SWT.CHECK);
		menuItem.setText ("Production Retractions") ;
		menuItem.setData("type", new Long(TraceType.kRetraction)) ;
		menuItem.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { changeFilter(e.widget, TraceType.kRetraction) ; } } ) ;

		menuItem = new MenuItem (m_FilterMenu, SWT.CHECK);
		menuItem.setText ("Stack Trace") ;
		menuItem.setData("type", new Long(TraceType.kStack)) ;
		menuItem.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { changeFilter(e.widget, TraceType.kStack) ; } } ) ;

		menuItem = new MenuItem (m_FilterMenu, SWT.CHECK);
		menuItem.setText ("Rhs Writes") ;
		menuItem.setData("type", new Long(TraceType.kRhsWrite)) ;
		menuItem.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { changeFilter(e.widget, TraceType.kRhsWrite) ; } } ) ;

		menuItem = new MenuItem (m_FilterMenu, SWT.PUSH);
		menuItem.setText ("Show all") ;
		menuItem.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { m_FoldingText.setExclusionFilter(0, true) ; updateButtonState() ; } } ) ;

		menuItem = new MenuItem (m_FilterMenu, SWT.PUSH);
		menuItem.setText ("Hide all") ;
		menuItem.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { m_FoldingText.setExclusionFilter(TraceType.kAllExceptTopLevel, true) ; updateButtonState() ; } } ) ;

		updateButtonState() ;
	}
	
	protected void changeFilter(Widget widget, long type)
	{
		MenuItem item = (MenuItem)widget ;
		boolean selected = item.getSelection() ;
		
		m_FoldingText.changeExclusionFilter(type, !selected, true) ;
		
		// A change to one button can affect others
		updateButtonState() ;
	}
	
	/************************************************************************
	* 
	* Search for the next occurance of 'text' in this view and place the selection
	* at that point.
	* 
	* @param text			The string to search for
	* @param searchDown		If true search from top to bottom
	* @param matchCase		If true treat the text as case-sensitive
	* @param wrap			If true after reaching the bottom, continue search from the top
	* @param searchHidden	If true and this view has hidden text (e.g. unexpanded tree nodes) search that text
	* 
	*************************************************************************/
	public boolean find(String text, boolean searchDown, boolean matchCase, boolean wrap, boolean searchHiddenText)
	{
		boolean found = false ;
		
		// Show the wait cursor as the hidden search could take a while
		Cursor wait = new Cursor(getWindow().getDisplay(), SWT.CURSOR_WAIT) ;
		getWindow().getShell().setCursor(wait) ;
				
		String windowText = m_FoldingText.getAllText(searchHiddenText) ;
		
		// If we're case insensitive shift all to lower case
		if (!matchCase)
		{
			windowText = windowText.toLowerCase() ;
			text = text.toLowerCase() ;
		}
		
		// Find out where we're starting from
		Point selectionPoint = m_FoldingText.getTextWindow().getSelection() ;
		int selectionStart = selectionPoint.x ;
		
		// If we're searching the entire set of text need to switch to the position within the entire set of text
		if (searchHiddenText)
			selectionStart = m_FoldingText.convertVisibleToAllCharPos(selectionStart) ;
		
		int origStart = selectionStart ;
		
		int start = -1 ;
		boolean wrapped = false ;
		boolean done ;
		do
		{
			if (searchDown)
			{
				start = windowText.indexOf(text, selectionStart + 1) ;
			}
			else
			{
				start = windowText.lastIndexOf(text, selectionStart - 1) ;
			}
			
			if (start != -1)
			{
				// We found some text, so set the selection and we're done.
				found = true ;
				done = true ;				

				// Unless we've done a wrapped search and passed our start point
				// in which case we're actually done
				if (wrapped && ((searchDown && (start >= origStart)) || (!searchDown && (start <= origStart))))
					found = false ;
			}
			else
			{
				if (wrap && !wrapped)
				{
					// If fail to find text with the basic search repeat it here
					// which produces a wrap effect.
					done = false ;
					wrapped = true ;	// Only do it once
					selectionStart = searchDown ? -1 : windowText.length() ;
				}
				else
				{
					// If we're not wrapping (or already did the wrap) return false
					// to signal we failed to find anything.
					found = false ;
					done = true ;
				}
			}
		} while (!done) ;
		
		if (found)
		{
			int end = start + text.length() ;

			// If we're searching in hidden text need to convert back to the visible space and
			// possibly expand the block
			if (searchHiddenText)
			{
				// Force block to expand if needed
				m_FoldingText.makeCharPosVisible(start) ;
				
				start = m_FoldingText.convertAllToVisibleCharPos(start) ;
				end   = m_FoldingText.convertAllToVisibleCharPos(end) ;
			}
			
			// Set the newly found text to be selected
			m_FoldingText.setSelection(start, end) ;
		}
		
		getWindow().getShell().setCursor(null) ;
		wait.dispose() ;
		return found ;
	}
	
	/************************************************************************
	* 
	* Go from current selection (where right click occured) to the object
	* selected by the user (e.g. a production name).
	* 
	*************************************************************************/
	protected ParseSelectedText.SelectedObject getCurrentSelection(int mouseX, int mouseY)
	{
		// Switchfrom screen coords to coords based on the text window
		Point pt = m_FoldingText.getTextWindow().toControl(mouseX, mouseY) ;
		mouseX = pt.x ;
		mouseY = pt.y ;

		int line = m_FoldingText.getLine(mouseY) ;
		if (line == -1)
			return null ;
		
		String text = m_FoldingText.getTextForLine(line) ;		
		if (text == null)
			return null ;

		int pos = m_FoldingText.getCharacterPosition(text, mouseX) ;
		if (pos == -1)
			return null ;

		// Sometimes we need to search back up the trace or down the trace to determine the context
		// so we'll add these lines above and below the to current line, adjusting the position as we do.
		// This is just a heuristic but it should cover 99% of cases.
		int bufferLinesAbove = Math.min(20, line) ;
		int bufferLinesBelow = 1 ;
		String combinedText = text ;
		int combinedPos = pos ;
		for (int i = 0 ; i < bufferLinesAbove ; i++)
		{
			String lineText = m_FoldingText.getTextForLine(line - i - 1) ;
			
			if (lineText != null)
			{
				combinedText = lineText + combinedText ;
				combinedPos += lineText.length() ;
			}
		}
		for (int i = 0 ; i < bufferLinesBelow ; i++)
		{
			String lineText = m_FoldingText.getTextForLine(line + i + 1) ;
			if (lineText != null)
				combinedText = combinedText + lineText ;
		}

		// Go from the text to a Soar selection object (e.g. an id or an attribute -- that sort of thing)
		ParseSelectedText selection = new ParseSelectedText(combinedText, combinedPos) ;
		
		return selection.getParsedObject() ;
		
	}
	
	protected void expandPage(boolean state)
	{
		m_FoldingText.expandPage(state) ;
	}
	
	protected void expandAll(boolean state)
	{
		m_FoldingText.expandAll(state) ;
	}
	
	protected void layoutComboBar(boolean top)
	{
		m_ComboContainer.setLayout(new FormLayout()) ;

		FormData containerData = top ? FormDataHelper.anchorTop(0) : FormDataHelper.anchorBottom(0) ;
		m_ComboContainer.setLayoutData(containerData) ;

		FormData comboData = FormDataHelper.anchorTopLeft(0) ;
		comboData.right = new FormAttachment(m_Buttons) ;
		m_CommandCombo.setLayoutData(comboData) ;

		FormData buttonData = FormDataHelper.anchorTop(0) ;
		buttonData.left = null ;
		m_Buttons.setLayoutData(buttonData) ;
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
		return "trace" ;
	}

	/** The control we're using to display the output in this case **/
	protected Control getDisplayControl()
	{
		return m_FoldingText.getTextWindow() ;
	}

	/** 
	 * Returns the entire window, within which the display control lies.
	 * 
	 * Usually the display control is all there is, but this method allows us to define
	 * a container that surrounds the display control and includes other supporting controls.
	 * In which case this method should be overriden.
	 */
	protected Control getDisplayWindow()
	{
		return m_FoldingText.getWindow() ;
	}

	public void setTextFont(Font f)
	{
		super.setTextFont(f) ;
		
		// Changing the font means we need to redraw the icon bar, just as if we scrolled
		m_FoldingText.scrolled() ;
	}

	/************************************************************************
	* 
	* Add the text to the view in a thread safe way (switches to UI thread)
	* 
	*************************************************************************/
	protected void appendSubTextSafely(final String text, final boolean redrawTree, final long type)
	{
		// If Soar is running in the UI thread we can make
		// the update directly.
		if (!Document.kDocInOwnThread)
		{
			appendSubText(text, redrawTree, type) ;
			return ;
		}

		// Have to make update in the UI thread.
		// Callback comes in the document thread.
        Display.getDefault().asyncExec(new Runnable() {
            public void run() {
            	appendSubText(text, redrawTree, type) ;
            }
         }) ;
	}
		
	/************************************************************************
	* 
	* Add the text to the view (this method assumes always called from UI thread)
	* 
	*************************************************************************/
	protected void appendSubText(String text, boolean redrawTree, long type)
	{
		String[] lines = text.split(kLineSeparator) ;

		for (int i = 0 ; i < lines.length ; i++)
		{	
			if (lines[i].length() == 0)
				continue ;

			m_FoldingText.appendSubText(lines[i] + kLineSeparator, m_ExpandTracePersistent || m_ExpandTrace, type) ;
		}
	}
	
	/************************************************************************
	* 
	* Add the text to the view (this method assumes always called from UI thread)
	* 
	*************************************************************************/
	protected void appendText(String text, long type)
	{
		String[] lines = text.split(kLineSeparator) ;

		for (int i = 0 ; i < lines.length ; i++)
		{	
			if (lines[i].length() == 0)
				continue ;
			
			m_FoldingText.appendText(lines[i] + kLineSeparator, type) ;
		}
	}
	
	protected void appendText(String text)
	{
		appendText(text, TraceType.kTopLevel) ;
	}
	
	/************************************************************************
	* 
	* Clear the display control.
	* 
	*************************************************************************/
	public void clearDisplay()
	{
		m_FoldingText.clear() ;
	}
	
	/********************************************************************************************
	 * 
	 * This handler should only be called from the UI thread as it does a lot of UI work.
	 * 
	 * @param agent
	 * @param xmlParent
	********************************************************************************************/
	protected void displayXmlTraceEvent(Agent agent, ClientXML xmlParent)
	{
		// Stop updating the tree control while we work on it
		//m_Tree.setRedraw(false) ;
		
		// For debugging
		//String message = xmlParent.GenerateXMLString(true) ;
		//System.out.println(message) ;
		
		int nChildren = xmlParent.GetNumberChildren() ;
		
		for (int childIndex = 0 ; childIndex < nChildren ; childIndex++)
		{
			// Analyze the children as ClientTraceXML objects
			ClientTraceXML xmlTrace = new ClientTraceXML() ;

			// Get each child in turn
			xmlParent.GetChild(xmlTrace, childIndex) ;
			
			final int decisionDigits = 6 ;	// So colons match with print --stack in trace window
			
			// This is a state change (new decision)
			if (xmlTrace.IsTagState())
			{
				// 3:    ==>S: S2 (operator no-change)
				StringBuffer text = new StringBuffer() ;				
				text.append(XmlOutput.padLeft(xmlTrace.GetDecisionCycleCount(), decisionDigits)) ;
				text.append(": ") ;
				text.append(XmlOutput.indent(xmlTrace.GetStackLevel(), -1, m_IndentSize)) ;

				// Add an appropriate subgoal marker to match the indent size
				if (m_IndentSize == 3)
					text.append("==>") ;
				else if (m_IndentSize == 2)
					text.append("=>") ;
				else if (m_IndentSize == 1)
					text.append(">") ;
				else if (m_IndentSize > 3)
				{
					text.append(XmlOutput.getSpaces(m_IndentSize - 3)) ;
					text.append("==>") ;
				}
				
				text.append("S: ") ;
				text.append(xmlTrace.GetStateID()) ;
				
				if (xmlTrace.GetImpasseObject() != null)
				{
					text.append(" (") ;
					text.append(xmlTrace.GetImpasseObject()) ;
					text.append(" ") ;
					text.append(xmlTrace.GetImpasseType()) ;
					text.append(")") ;
				}
				
				if (text.length() != 0)
					this.appendText(text.toString(), TraceType.kStack) ;
			} else if (xmlTrace.IsTagOperator())
			{
				 //2:    O: O8 (move-block)
				StringBuffer text = new StringBuffer() ;
				text.append(XmlOutput.padLeft(xmlTrace.GetDecisionCycleCount(), decisionDigits)) ;
				text.append(": ") ;
				text.append(XmlOutput.indent(xmlTrace.GetStackLevel(), 0, m_IndentSize)) ;
				text.append("O: ") ;
				text.append(xmlTrace.GetOperatorID()) ;
				
				if (xmlTrace.GetOperatorName() != null)
				{
					text.append(" (") ;
					text.append(xmlTrace.GetOperatorName()) ;
					text.append(")") ;
				}
	
				if (text.length() != 0)
					this.appendText(text.toString(), TraceType.kStack) ;
			} else if (xmlTrace.IsTagRhsWrite())
			{
				String output = xmlTrace.GetString() ;
				
				if (output.length() != 0)
					this.appendText(output, TraceType.kRhsWrite) ;				
				
			} else if (xmlTrace.IsTagPhase())
			{
				String status = xmlTrace.GetPhaseStatus() ;
				
				String output = XmlOutput.getPhaseText(agent, xmlTrace, status) ;
								
				// Don't show end of phase messages
				boolean endOfPhase = (status != null && status.equalsIgnoreCase("end")) ;
				
				if (output.length() != 0 && !endOfPhase)
					this.appendSubText(output, false, TraceType.kPhase) ;
			}
			else if (xmlTrace.IsTagAddWme() || xmlTrace.IsTagRemoveWme())
			{
				boolean adding = xmlTrace.IsTagAddWme() ;
				String output = XmlOutput.getWmeChange(agent, xmlTrace, adding) ;
				
				if (output.length() != 0)
					this.appendSubText(output, false, TraceType.kWmeChange) ;	

			} else if (xmlTrace.IsTagPreference())
			{
				String output = XmlOutput.getPreferenceText(agent, xmlTrace) ;
				
				if (output.length() != 0)
					this.appendSubText(output.toString(), false, TraceType.kPreference) ;
				
			} 
			else if (xmlTrace.IsTagFiringProduction() || xmlTrace.IsTagRetractingProduction())
			{
				boolean firing = xmlTrace.IsTagFiringProduction() ;

				String output = XmlOutput.getProductionFiring(agent, xmlTrace, firing) ;
				
				long type = (firing ? TraceType.kFiring : TraceType.kRetraction) ;

				if (output.length() != 0)
					this.appendSubText(output, false, type) ;			
	
			}
			else if (xmlTrace.IsTagLearning())
			{
				// Building chunk*name
				// Optionally followed by the production
				for (int i = 0 ; i < xmlTrace.GetNumberChildren() ; i++)
				{
					ClientTraceXML prod = new ClientTraceXML() ;
					xmlTrace.GetChild(prod, i) ;
					if (prod.IsTagProduction())
					{
						if (prod.GetNumberChildren() == 0)
						{
							// If this production has no children it's just a "we're building x" message.
							StringBuffer text = new StringBuffer() ;
							
							if (i > 0)
								text.append(kLineSeparator) ;
							
							text.append("Building ") ;
							text.append(prod.GetProductionName()) ;							
							appendText(text.toString(), TraceType.kLearning) ;			
						}
						else
						{	
							// If has children we're looking at a full production print
							// (Note -- we get the "we're building" from above as well as this so we don't
							// add the "build x" message when this comes in.
							String prodText = XmlOutput.getProductionText(agent, prod) ;
							
							if (prodText != null && prodText.length() != 0)
								this.appendSubText(prodText, false, TraceType.kFullLearning) ;			
						}
					}
					prod.delete() ;
				}
			}
			else
			{
				// These lines can be helpful if debugging this -- we
				// print out any XML we completely fail to understand.
				String xmlText = xmlParent.GenerateXMLString(true) ;
				System.out.println(xmlText) ;
			}

			// Manually clean up the child too
			xmlTrace.delete() ;	
		}
		
		// Technically this will happen when the object is garbage collected (and finalized)
		// but without it we'll get a million memory leak messages on shutdown because (a) gc may not have been run for a while
		// (b) even if it runs not all objects will be reclaimed and (c) finalize isn't guaranteed before we exit
		// so all in all, let's just call it ourselves here :)
		xmlParent.delete() ;
		
		// Redraw the tree to show our changes
		//m_Tree.setRedraw(true) ;
	}
	
	public static class RunWrapper implements Runnable
	{
		FoldingTextView m_View ;
		Agent 		  	m_Agent ;
		ClientXML 	  	m_XML ;
		
		public RunWrapper(FoldingTextView view, Agent agent, ClientXML xml)
		{
			m_Agent = agent ;
			m_XML   = xml ;
			m_View  = view ;
		}
		
		public void run()
		{
			m_View.displayXmlTraceEvent(m_Agent, m_XML) ;
		}
	}
	
	
	public void xmlEventHandler(int eventID, Object data, Agent agent, ClientXML xml)
	{
		if (eventID != smlXMLEventId.smlEVENT_XML_TRACE_OUTPUT.swigValue())
			return ;

		// The messages come collected into a parent <trace> tag so that one event
		// can send over many pieces of a trace in a single call.  Just more
		// efficient that way.
		
		ClientTraceXML xmlParent = xml.ConvertToTraceXML() ;
		if (!xmlParent.IsTagTrace() || xmlParent.GetNumberChildren() == 0)
		{
			xml.delete() ;
			return ;
		}
		// The conversion creates a new SWIG object which we have to delete.
		xmlParent.delete() ;
		
		// If Soar is running in the UI thread we can make
		// the update directly.
		if (!Document.kDocInOwnThread)
		{
			displayXmlTraceEvent(agent, xml) ;
			return ;
		}

		// Have to make update in the UI thread.
		// Callback comes in the document thread.
		// NOTE: I had to write a real class here to do this rather than just using my
		// "normal" trick of making agent and xml "final" and creating a class on the fly.
		// Doing that lead to an intermittement memory leak from the xml object--really hard to track down
		// and I'm still not fully clear on why that happened, but I suspect if this event was called again
		// before the wrapper had run, we had a problem.
		
		// We need to create a new copy of the XML we were passed because we're
		// going to use an asynch call, which won't execute until after this function has
		// completed and xml goes out of scope.  Why use the asynch method rather than syncExec()?
		// For a watch 5 trace using asynch here lets things run up to 5 times faster(!)
		ClientXML pKeep = new ClientXML(xml) ;
        Display.getDefault().asyncExec(new RunWrapper(this, agent, pKeep)) ;
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

	protected void clearViewAgentEvents()
	{
		m_xmlCallback = -1 ;
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
		PropertiesDialog.Property properties[] = new PropertiesDialog.Property[3] ;

		// Providing a range for indent so we can be sure we don't get back a negative value
		properties[0] = new PropertiesDialog.IntProperty("Indent per subgoal", m_IndentSize, 0, 10) ;
		properties[1] = new PropertiesDialog.BooleanProperty("Expand trace as it is created", m_ExpandTracePersistent) ;
		properties[2] = new PropertiesDialog.BooleanProperty("Capture data to support filtering", m_FoldingText.isFilteringEnabled()) ;

		boolean ok = PropertiesDialog.showDialog(m_Frame, "Properties", properties) ;

		if (ok)
		{
			m_IndentSize = ((PropertiesDialog.IntProperty)properties[0]).getValue() ;
			m_ExpandTracePersistent = ((PropertiesDialog.BooleanProperty)properties[1]).getValue() ;
			m_FoldingText.setFilteringEnabled(((PropertiesDialog.BooleanProperty)properties[2]).getValue()) ;
			
			// Make the button match the persistent property
			m_ExpandTrace = m_ExpandTracePersistent ;
			updateButtonState() ;
		}		
	}
	
	/************************************************************************
	* 
	* Converts this object into an XML representation.
	* 
	* @param tagName		The tag name to use for the top XML element created by this view
	* @param storeContent	If true, record the content from the display (e.g. the text from a trace window)
	* 
	*************************************************************************/
	public general.ElementXML convertToXML(String tagName, boolean storeContent)
	{
		ElementXML element = super.convertToXML(tagName, storeContent) ;
		element.addAttribute("indent", Integer.toString(m_IndentSize)) ;
		element.addAttribute("auto-expand", Boolean.toString(m_ExpandTracePersistent)) ;

		if (m_FoldingText != null)
		{
			element.addAttribute("filtering", Boolean.toString(m_FoldingText.isFilteringEnabled())) ;
			element.addAttribute("filter", Long.toString(m_FoldingText.getExclusionFilter())) ;
		}
		
		return element ;
	}

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
	public void loadFromXML(MainFrame frame, doc.Document doc, Pane parent, general.ElementXML element) throws Exception
	{
		m_IndentSize = element.getAttributeIntThrows("indent") ;
		m_ExpandTracePersistent = element.getAttributeBooleanDefault("auto-expand", false) ;

		boolean filtering = element.getAttributeBooleanDefault("filtering", true) ;
		long filter = element.getAttributeLongDefault("filter", 0) ;
		
		super.loadFromXML(frame, doc, parent, element) ;
		
		// Have to wait until base class has been called and window has been created before setting these values
		if (m_FoldingText != null)
		{
			m_FoldingText.setFilteringEnabled(filtering) ;
			m_FoldingText.setExclusionFilter(filter, false) ;
			updateButtonState() ;
		}
	}
}
