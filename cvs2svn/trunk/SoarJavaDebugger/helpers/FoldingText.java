/********************************************************************************************
*
* FoldingText.java
* 
* Description:	
* 
* Created on 	May 1, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package helpers;

import java.util.ArrayList;

import modules.AbstractView;

import org.eclipse.swt.widgets.*;
import org.eclipse.swt.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.graphics.*;
import org.eclipse.swt.layout.*;

/************************************************************************
 * 
 * A widget that consists of a scrolling text window and a small bar with
 * icons for 'folding' the text (i.e. expanding and contracting) sections
 * of the text.
 * 
 * This is very similar to a view offered by Eclipse, but only depends on SWT
 * and may be higher performance if we get it right.
 * 
 ************************************************************************/
public class FoldingText
{
	protected Text		m_Text ;
	protected Canvas 	m_IconBar ;
	protected Composite m_Container ;
	protected FoldingTextDoc m_FoldingDoc = new FoldingTextDoc(this) ;
	protected FilterDoc	m_FilterDoc = new FilterDoc(this) ;
	protected int		m_LastTopIndex ;
	protected boolean	m_DrawingDisabled = false ;
	
	// If false, we don't collect information to support filtering (increases performance and saves memory although
	// the differences seem to be small from my testing).
	protected boolean	m_FilteringEnabled = true ;

	public static class FilterRecord
	{
		protected String 	m_Text ;			// The text itself
		protected boolean 	m_SubText ;			// If true this text goes at a sub level
		protected long		m_Type ;		// The type of information stored here, against which we filter.  (This is treated as a bit field).
		
		public FilterRecord(String text, boolean subText, long type)
		{
			m_Text = text ;
			m_SubText = subText ;
			m_Type = type ;
		}
		
		public boolean isSubText()
		{
			return m_SubText;
		}
		public String getText()
		{
			return m_Text;
		}
		public long getType()
		{
			return m_Type;
		}
	}
	
	public static class FilterDoc
	{
		protected ArrayList		m_AllRecords = new ArrayList() ;
		protected FoldingText	m_FoldingText ;
		
		// If type & excludeFilter != 0 we won't display it
		protected long			m_ExcludeFilter = 0 ;
		
		public FilterDoc(FoldingText foldingText)
		{
			m_FoldingText = foldingText ;
		}

		public boolean show(boolean subText, long type)
		{
			return ((m_ExcludeFilter & type) == 0) ; 
		}
		
		// Returns true if we should show this line
		public boolean addRecord(String text, boolean subText, long type)
		{
			m_AllRecords.add(new FilterRecord(text, subText, type)) ;
			
			return show(subText, type) ;
		}
		
		public void clear()
		{
			m_AllRecords.clear() ;
		}
		
		public void setExcludeFilter(long filter)
		{
			m_ExcludeFilter = filter ;
		}
		
		public long getExcludeFilter()
		{
			return m_ExcludeFilter ;
		}
		
		public void regenerateDisplay()
		{
			// Turn off tree updates
			m_FoldingText.setRedraw(false) ;
			
			// Clear the existing text window
			m_FoldingText.getTextWindow().setText("") ;

			// BUGBUG: Need to go through existing blocks and decide if a top level line
			// is part of an expanded block or not, so we can recreate the expanded block structure correctly.
			
			// Clear all existing blocks
			m_FoldingText.m_FoldingDoc.clear() ;
			
			int size = m_AllRecords.size() ;
			for (int i = 0 ; i < size ; i++)
			{
				FilterRecord record = (FilterRecord)m_AllRecords.get(i) ;
				
				if (show(record.isSubText(), record.getType()))
				{
					if (record.isSubText())
						m_FoldingText.appendSubTextInternal(record.getText(), false) ;
					else
						m_FoldingText.appendTextInternal(record.getText()) ;
				}
			}
			
			// Draw the new tree
			m_FoldingText.setRedraw(true) ;
		}
	}
	
	public static class FoldingTextDoc
	{
		protected	ArrayList	m_TextBlocks = new ArrayList() ;
		protected	int			m_ShowFilter ;
		protected	int			m_HideFilter ;
		
		protected 	FoldingText	m_FoldingText ;
		protected	Block		m_LastBlock ;
		
		public FoldingTextDoc(FoldingText text)
		{
			m_FoldingText = text ;
		}
		
		public void clear()
		{
			m_TextBlocks.clear() ;
			m_LastBlock = null ;
		}
		
		public String toString()
		{
			StringBuffer buffer = new StringBuffer() ;
			
			buffer.append(m_TextBlocks.toString()) ;
			
			return buffer.toString() ;
		}
		
		public void addBlock(Block block)
		{
			block.setIndex(m_TextBlocks.size()) ;
			int start = m_LastBlock != null ? m_LastBlock.getStart() + m_LastBlock.getVisibleSize() : 0 ;
			block.setStart(start) ;
			
			m_TextBlocks.add(block) ;
			m_LastBlock = block ;
		}
		
		/** Returns the block (if any) which starts at the given line number. */
		public Block getBlockStartsAtLineNumber(int lineNumber)
		{
			// Get the block which contains this line
			Block block = getBlockByLineNumber(lineNumber) ;
			
			if (block == null)
				return null ;

			// Check if the block starts with the target line in this case
			if (block.getStart() == lineNumber)
				return block ;
			
			return null ;
		}
		
		/** Returns the index of the first block which either starts at lineNumber or includes lineNumer */
		public Block getBlockByLineNumber(int lineNumber)
		{
			// BUGBUG: We should really hash this lookup.  Without it our paint code and other logic is always O(n)
			// for n blocks, which means performance of the view will slow down over time.
			// If we hash the values we can make that O(1) so we don't slow down.
			// So why haven't I hashed?  I want to make sure everything is correct first before adding a cache (the hash table) which
			// could get out of date and cause problems.  When we have the rest tested and are confident it's correct we can add the hash table
			// with confidence.
			// (BTW, my plan would be to hash on specific line numbers -- perhaps every 100 lines -- so given a line number we look up the general region
			//  and then go find the specific block.  Otherwise we'll get a pretty big hash table with an entry for each line of output which will be slow to update.
			//  This would still be O(1).  The time to update the table is when the start line for any of the blocks is changed (e.g. when expanding/contracting).
			int size = m_TextBlocks.size() ;
			for (int b = 0 ; b < size ; b++)
			{
				Block block = (Block)m_TextBlocks.get(b) ;
				if (block.containsLine(lineNumber))
					return block ;
			}
			
			return null ;
		}
		
		public Block getBlock(int index)
		{
			return (Block)m_TextBlocks.get(index) ;
		}
		
		public int getNumberBlocks()
		{
			return m_TextBlocks.size() ;
		}
		
		public String getAllText(boolean includeHidden)
		{
			StringBuffer all = new StringBuffer() ;

			int size = m_TextBlocks.size() ;
			for (int b = 0 ; b < size ; b++)
			{
				Block block = (Block)m_TextBlocks.get(b) ;
				String text = (includeHidden ? block.getAll() : block.getVisibleText()) ; 
				all.append(text) ;
			}
			
			return all.toString() ;
		}
		
		/** Returns the character positions for the start and end of a block -- so we can use these to set the selection to the block */
		public Point getBlockSelectionRange(Block block)
		{
			int start = 0 ;

			for (int b = 0 ; b < block.getIndex() ; b++)
			{
				int chars = ((Block)m_TextBlocks.get(b)).getVisibleCharCount() ;
				start += chars ;
			}
			
			int end = start + block.getVisibleCharCount() ;
			
			return new Point(start, end) ;
		}
		
		public int convertVisibleToAllCharPos(int charPos)
		{
			int allCharPos = 0 ;
			int visCharPos = 0 ;
			
			int size = m_TextBlocks.size() ;
			for (int b = 0 ; b < size ; b++)
			{
				Block block = (Block)m_TextBlocks.get(b) ;
				int allChars = block.getAllCharCount() ;
				int visChars = block.getVisibleCharCount() ;
				
				if (charPos >= visCharPos && charPos < visCharPos + visChars)
				{
					// The visible selection lies within this block so return
					// the sum of all characters to this block, plus the number of chars into this block
					return allCharPos + (charPos - visCharPos) ;
				}
				
				allCharPos += allChars ;
				visCharPos += visChars ;
			}
			
			return -1 ;
		}

		public int convertAllToVisibleCharPos(int charPos)
		{
			int allCharPos = 0 ;
			int visCharPos = 0 ;
			
			int size = m_TextBlocks.size() ;
			for (int b = 0 ; b < size ; b++)
			{
				Block block = (Block)m_TextBlocks.get(b) ;
				int allChars = block.getAllCharCount() ;
				int visChars = block.getVisibleCharCount() ;
				
				if (charPos >= allCharPos && charPos < allCharPos + allChars)
				{
					// The visible selection lies within this block so return
					// the sum of all characters to this block, plus the number of chars into this block
					return visCharPos + (charPos - allCharPos) ;
				}
				
				allCharPos += allChars ;
				visCharPos += visChars ;
			}
			
			return -1 ;
		}
		
		public Block findBlockByAllCharPos(int charPos)
		{
			int allCharPos = 0 ;
			int visCharPos = 0 ;
			
			int size = m_TextBlocks.size() ;
			for (int b = 0 ; b < size ; b++)
			{
				Block block = (Block)m_TextBlocks.get(b) ;
				int allChars = block.getAllCharCount() ;
				int visChars = block.getVisibleCharCount() ;
				
				if (charPos >= allCharPos && charPos < allCharPos + allChars)
				{
					// The visible selection lies within this block so return
					// the sum of all characters to this block, plus the number of chars into this block
					return block ;
				}
				
				allCharPos += allChars ;
				visCharPos += visChars ;
			}
			
			return null ;			
		}		

		/** Given a character position in the entire text stream (e.g. the selection) returns the line and offset.
		 *  NOTE: Not fully tested -- stopped using it before then but could be useful later so keeping the code. **/
		public Point getLineAndOffset(int charPos)
		{
			int globalCharPos = 0 ;

			int nBlocks = m_TextBlocks.size() ;
			for (int b = 0 ; b < nBlocks ; b++)
			{
				Block block = (Block)m_TextBlocks.get(b) ;
				int chars = block.getVisibleCharCount() ;
				
				if (charPos >= globalCharPos && charPos < globalCharPos + chars)
				{
					int charOffset = charPos - globalCharPos ;
					
					// Get the line and offset for this character position
					Point line = block.findLineFromCharCount(charOffset) ;
					
					// Convert the line number into the global line number (not block relative)
					Point result = new Point(line.x + block.getStart(), line.y) ;
					return result ;
				}
				
				// Move the search on
				globalCharPos += chars ;
			}
			
			return null ;
		}
		
		public void expandBlock(Block block, boolean state)
		{
			if (block.isExpanded() == state || !block.canExpand() || block.getSize() == 1)
				return ;

			Point range = getBlockSelectionRange(block) ;
			int delta = 0 ;
			
			//boolean selected = (m_FoldingText.m_Text.getSelectionCount() > 1) ;
			
			m_FoldingText.m_Text.setSelection(range) ;
			
			// For debugging show selection and then update it
			//if (!selected)
			//	return ;
			
			if (state)
			{
				// Expanding
				m_FoldingText.m_Text.insert(block.getAll()) ;
				delta = block.getSize() - 1 ;
			}
			else
			{
				m_FoldingText.m_Text.insert(block.getFirst()) ;
				delta = 1 - block.getSize() ;
			}
			
			// Update the remaining block position info
			int size = getNumberBlocks() ;
			for (int b = block.getIndex()+1 ; b < size ; b++)
			{
				Block update = getBlock(b) ;
				update.setStart(update.getStart() + delta) ;
			}
			
			block.setExpand(state) ;
		}
	}
	
	/** Represents a section of text that is a single unit for expanding/collapsing. */
	public static class Block
	{
		protected int		m_Index ;
		protected boolean	m_CanExpand ;
		protected boolean	m_IsExpanded ;
		protected int		m_Start ;				// The first line where this block appears in the text widget

		protected ArrayList m_Lines = new ArrayList() ;
		protected StringBuffer m_All = new StringBuffer() ;
		
		public Block(boolean canExpand) { m_CanExpand = canExpand ; m_IsExpanded = false ; }
		
		public void setIndex(int index)	{ m_Index = index ; }
		public int getIndex()			{ return m_Index ; }
		
		public void setStart(int start)			{ m_Start = start ; }
		public int  getStart() 					{ return m_Start ; }
		public void setCanExpand(boolean state)	{ m_CanExpand = state ; }
		public boolean canExpand()				{ return m_CanExpand && m_Lines.size() > 1 ; }
		public void setExpand(boolean state)	{ m_IsExpanded = state ; }
		
		// Blocks which can't expand/collapse are always in the expanded state
		public boolean isExpanded()				{ return m_IsExpanded || !m_CanExpand ; }
		
		public int  getSize()  				{ return m_Lines.size() ; }
		public int  getVisibleSize()		{ return isExpanded() ? m_Lines.size() : 1 ; }
		public void appendLine(String text) { m_Lines.add(text) ; m_All.append(text) ; }
		
		public void removeLastLine()		{ m_Lines.remove(m_Lines.size() - 1) ; recalcAll() ; if (m_Lines.size() == 0) throw new IllegalStateException("Shouldn't empty a block") ; }
		
		public boolean containsLine(int line) { return line >= m_Start && line <= m_Start + getVisibleSize() - 1 ; }
		
		public String getFirst() 			{ return (String)m_Lines.get(0) ; }
		public String getAll()				{ return m_All.toString() ; }
		public String getLastLine()			{ if (m_Lines.size() == 0) throw new IllegalStateException("Block shouldn't be empty") ;
											  return (String)m_Lines.get(m_Lines.size()-1) ; }
		public String getVisibleText()		{ return (isExpanded() ? getAll() : getFirst()) ; }
		
		public String getTextForLine(int line) { return (String)m_Lines.get(line) ; }
		
		private void recalcAll()
		{
			m_All = new StringBuffer() ;
			
			for (int i = 0 ; i < m_Lines.size() ; i++)
				m_All.append((String)m_Lines.get(i)) ;
		}
		
		public int getFirstLineCharCount()	{ return getFirst().length() ; }
		public int getAllCharCount()		{ return m_All.length() ; }	
		public int getVisibleCharCount()	{ return isExpanded() ? getAllCharCount() : getFirstLineCharCount() ; }
		
		/** Returns (line, offset) */
		public Point findLineFromCharCount(int charCount)
		{
			int pos = 0 ;
			int lines = m_Lines.size() ;
			for (int i = 0 ; i < lines ; i++)
			{
				String line = (String)m_Lines.get(i) ;
				int len = line.length() ;

				if (charCount >= pos && charCount < pos + len)
				{
					return new Point(i, charCount - pos) ;
				}
				
				pos += len ;
			}
			return null ;	// Something went wrong
		}
		
		public String toString()
		{
			StringBuffer buffer = new StringBuffer() ;
			
			buffer.append("(") ;
			buffer.append(m_Start) ;
			buffer.append(m_CanExpand ? (m_IsExpanded ? "-" : "+") : "!") ;
			buffer.append(",") ;
			buffer.append(" Size ") ;
			buffer.append(getSize()) ;
			buffer.append(")") ;
			
			return buffer.toString() ;
		}
	}
	
	public FoldingText(Composite parent)
	{
		m_Container = new Composite(parent, 0) ;
		m_IconBar	= new Canvas(m_Container, 0) ;
		m_Text      = new Text(m_Container, SWT.MULTI | SWT.V_SCROLL | SWT.H_SCROLL | SWT.READ_ONLY) ;
		m_DrawingDisabled = false ;
		
		GridLayout layout = new GridLayout() ;
		layout.numColumns = 2 ;
		m_Container.setLayout(layout) ;
		
		GridData data1 = new GridData(GridData.FILL_VERTICAL) ;
		data1.widthHint = 13 ;
		m_IconBar.setLayoutData(data1) ;

		GridData data2 = new GridData(GridData.FILL_BOTH) ;
		m_Text.setLayoutData(data2) ;
		
		m_IconBar.addPaintListener(new PaintListener() { public void paintControl(PaintEvent e) { paintIcons(e) ; } } ) ;
		m_IconBar.setBackground(m_IconBar.getDisplay().getSystemColor(SWT.COLOR_WHITE)) ;

		m_IconBar.addMouseListener(new MouseAdapter() { public void mouseUp(MouseEvent e) { iconBarMouseClick(e) ; } } ) ;
		
		// Think we'll need this so we update the icons while we're scrolling
		m_Text.getVerticalBar().addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { scrolled() ; } }) ;
		
		// The user can trigger a scroll in the text window in other ways than grabbing the scroll bar thumb and moving it so we
		// need to detect those too.
		Listener listener = new Listener () {
			int lastIndex = m_Text.getTopIndex ();
			public void handleEvent (Event e) {
				int index = m_Text.getTopIndex ();
				if (index != lastIndex) {
					lastIndex = index;
					scrolled() ;
				}
			}
		};
		
		// NOTE: Only detects scrolling by the user and not quite correct if you drag out of the window
		// (This code came from the SWT web site itself)
		m_Text.addListener (SWT.MouseDown, listener);
		m_Text.addListener (SWT.MouseMove, listener);
		m_Text.addListener (SWT.MouseUp, listener);
		m_Text.addListener (SWT.KeyDown, listener);
		m_Text.addListener (SWT.KeyUp, listener);
		m_Text.addListener (SWT.Resize, listener);
		
		m_LastTopIndex = m_Text.getTopIndex() ;
	}
	
	private void scrolled()
	{
		m_IconBar.redraw() ; 
	}
	
	public void clear()
	{
		m_FilterDoc.clear() ;
		m_FoldingDoc.clear() ;
		m_Text.setText("") ;
		m_IconBar.redraw() ;
	}
	
	public boolean isFilteringEnabled()
	{
		return this.m_FilteringEnabled ;
	}
	
	public void setFilteringEnabled(boolean state)
	{
		m_FilteringEnabled = state ;
	}

	public void changeExclusionFilter(long type, boolean add, boolean regenerateDisplay)
	{
		long filter = m_FilterDoc.getExcludeFilter() ;

		// Add or remove the given type from the exclusion filter
		if (add)
			filter = filter | type ;
		else
			filter = filter & (~type) ;
		
		System.out.println("New filter is " + filter) ;
		
		m_FilterDoc.setExcludeFilter(filter) ;
		
		// If asked, recompute the entire display (this is a lot of work)
		if (regenerateDisplay)
			m_FilterDoc.regenerateDisplay() ;
	}
	
	public String toString()
	{
		return m_FoldingDoc.toString() ;
	}
	
	public void expandAll(boolean state)
	{
		// Stop redrawing while we expand/collapse everything then turn it back on
		setRedraw(false) ;

		// Show a wait cursor -- this can take a while
		Cursor wait = new Cursor(getWindow().getDisplay(), SWT.CURSOR_WAIT) ;
		getWindow().getShell().setCursor(wait) ;

		// Go through expanding/contracting all of the blocks
		for (int i = 0 ; i < m_FoldingDoc.getNumberBlocks() ; i++)
		{
			Block block = m_FoldingDoc.getBlock(i) ;
			m_FoldingDoc.expandBlock(block, state) ;
		}
		
		setRedraw(true) ;
		
		// Set the cursor back to normal
		getWindow().getShell().setCursor(null) ;
		wait.dispose() ;
	}
	
	/** Expand/contract all blocks currently on screen */
	public void expandPage(boolean state)
	{
		// Stop redrawing while we expand/collapse everything then turn it back on
		setRedraw(false) ;
		
		// Get all the information about which part of the text window is visible
		int topLine = m_Text.getTopIndex() ;
		int lineHeight = m_Text.getLineHeight() ;
		int visibleLines = m_Text.getClientArea().height / lineHeight ;
		int lastLine = Math.min(m_Text.getLineCount(),m_Text.getTopIndex() + visibleLines) ;
		
		// Start with the first block that starts at topLine or includes topLine.
		Block topBlock = m_FoldingDoc.getBlockByLineNumber(topLine) ;
		Block bottomBlock = m_FoldingDoc.getBlockByLineNumber(lastLine) ;
		
		if (topBlock == null)
			return ;

		// If the lastLine is after the bottom block, use the last block in the document
		if (bottomBlock == null)
			bottomBlock = m_FoldingDoc.getBlock(m_FoldingDoc.getNumberBlocks()-1) ;
		
		int topIndex = topBlock.getIndex() ;
		int bottomIndex = bottomBlock.getIndex() ;
		
		for (int i = topIndex ; i <= bottomIndex ; i++)
		{
			Block block = m_FoldingDoc.getBlock(i) ;
			m_FoldingDoc.expandBlock(block, state) ;			
		}

		// Redraw everything
		setRedraw(true) ;		
	}
	
	// Returns the line we clicked on based on mouse coordinates
	public int getLine(int mouseY)
	{
		int topLine = m_Text.getTopIndex() ;
		int lineHeight = m_Text.getLineHeight() ;
		int screenLine = mouseY / lineHeight ;
		int line = topLine + screenLine ;
		
		if (line > m_Text.getLineCount())
			return -1 ;

		return line ;
	}
	
	public String getTextForLine(int line)
	{
		if (line == -1)
			return null ;
		
		Block block = m_FoldingDoc.getBlockByLineNumber(line) ;
		if (block == null)
			return null ;
		
		String text = block.getTextForLine(line - block.getStart()) ;
		return text ;
	}
	
	public String getAllText(boolean includeHidden)
	{
		if (!includeHidden)
			return m_Text.getText() ;
		
		return m_FoldingDoc.getAllText(includeHidden) ;
	}
	
	public int convertVisibleToAllCharPos(int charPos)
	{
		return m_FoldingDoc.convertVisibleToAllCharPos(charPos) ;
	}

	public int convertAllToVisibleCharPos(int charPos)
	{
		return m_FoldingDoc.convertAllToVisibleCharPos(charPos) ;
	}
	
	public void setSelection(int start, int end)
	{
		m_Text.setSelection(start, end) ;
		
		// May have scrolled, so need to redraw the icons
		m_IconBar.redraw() ;
	}
	
	/********************************************************************************************
	 * 
	 * Takes a character position from anywhere in the "all text" of the window
	 * and forces it to be visible (i.e. expands it).
	 * 
	 * @param charPos
	********************************************************************************************/
	public void makeCharPosVisible(int charPos)
	{
		Block block = m_FoldingDoc.findBlockByAllCharPos(charPos) ;
		if (block == null)
			return ;
		
		if (block.isExpanded())
			return ;

		// Expand the block
		m_FoldingDoc.expandBlock(block, true) ;
		m_IconBar.redraw() ;
	}

	// Returns the (line, offset) information for the start of the current selection
	public Point getSelectionStartLineAndOffset()
	{
		Point pt = m_Text.getSelection() ;
		int start = pt.x ;
		
		Point lineOff = m_FoldingDoc.getLineAndOffset(start) ;
		System.out.println("Selection line + off is " + lineOff) ;
		return lineOff ;
	}
	
	/********************************************************************************************
	 * 
	 * Given a line of text and a position, returns the character position within this line.
	 * 
	 * @param text		The line of text.  Can be looked up through getLine() and getTextForLine()
	 * @param mouseX	The x coordinate in the line
	 * @return	The character clicked on (or -1 if none)
	********************************************************************************************/
	public int getCharacterPosition(String text, int mouseX)
	{
		// The only way to compute this I can think of to compute which character was clicked on
		// is to generate each substring in turn and check its length against the point.
		// When we reach the correct length of string we've found the character.
		// This is slow and a bit clumsy, but since it's just on a right-click I think it's ok.
		GC gc = new GC(m_Text) ;
		
		int selectionPoint = -1 ;
		for (int i = 0 ; i < text.length() ; i++)
		{
			Point extent = gc.textExtent(text.substring(0, i)) ;
			if (extent.x > mouseX)
			{
				selectionPoint = (i == 0) ? 0 : i-1 ;
				break ;
			}
		}

		gc.dispose() ;
		
		return selectionPoint ;		
	}
	
	// This method can be called either as a result of new input coming from outside
	// or because of a change to a filter as we re-process the old information.
	private void appendTextInternal(String text)
	{
		Block last = m_FoldingDoc.m_LastBlock ;
		
		if (last == null || last.canExpand())
		{
			// We create blocks that can't fold to hold top level text lines
			last = new Block(false) ;
			m_FoldingDoc.addBlock(last) ;
		}
		
		last.appendLine(text) ;
		m_Text.append(text) ;		
	}
	
	public void appendText(String text, long type)
	{
		// This is needed because the text control (on Windows) stores newlines as \r\n and selections and character counts will get out of synch if we
		// work in the text control but reason about the text and they have different newlines.
		// (We still use \n everywhere else as the newline marker because that's what Soar uses)
		text = text.replaceAll(AbstractView.kLineSeparator, AbstractView.kSystemLineSeparator) ;

		boolean show = true ;
		if (m_FilteringEnabled)
		{
			// Record the text in our master filter document.
			// Also decide if we should show this particular line of text.
			show = m_FilterDoc.addRecord(text, false, type) ;
		}

		if (show)
		{
			appendTextInternal(text) ;

			// Decide if we have caused the window to scroll or not
			if (m_LastTopIndex != m_Text.getTopIndex())
			{
				scrolled() ;
				m_LastTopIndex = m_Text.getTopIndex() ;
			}
		}
	}
	
	protected void appendSubTextInternal(String text, boolean autoExpand)
	{
		Block last = m_FoldingDoc.m_LastBlock ;
		
		if (last == null)
		{
			// This is an odd situation where we're adding subtext but have no supertext to append to.
			// It probably will never occur, but if it does we'll add a blank super text block just
			// to get us going.
			last = new Block(true) ;
			last.appendLine("") ;
			last.setExpand(autoExpand) ;
			m_FoldingDoc.addBlock(last) ;			
			m_Text.append("") ;
			m_IconBar.redraw() ;
		}

		if (!last.canExpand())
		{
			// Need to remove the last line from the block and move it to a new block
			// which can expand and then proceed with the addition to this new block
			// Thus we ensure that "canExpand" items have at least one child.
			int size = last.getSize() ;
			
			if (size == 1)
			{
				// If the last block only contains exactly one line we can convert it safely
				// to a block which does expand.  This way we also preserve the "all blocks contain
				// at least one line" rule which makes the code simpler.
				last.setCanExpand(true) ;
				last.setExpand(autoExpand) ;
			}
			else if (size > 1)
			{
				// NOTE: Blocks should always have at least one line so these calls
				// should never fail (if they do it's a programming error somewhere else that
				// allowed an empty block to be created).
				String lastLine = last.getLastLine() ;
				last.removeLastLine() ;
				
				last = new Block(true) ;
				last.appendLine(lastLine) ;
				last.setExpand(autoExpand) ;
				m_FoldingDoc.addBlock(last) ;
			} else if (size == 0)
			{
				throw new IllegalStateException("Last block should not be empty") ;
			}
			
			// There's no change to the text sprite (because the line is just moved between logical blocks)
			// but we will need to draw the icons to show there's a new block.
			m_IconBar.redraw() ;
		}
		
		last.appendLine(text) ;

		if (last.isExpanded())
		{
			m_Text.append(text) ;

			// Decide if we have caused the window to scroll or not
			if (!autoExpand && (m_LastTopIndex != m_Text.getTopIndex()))
			{
				scrolled() ;
				m_LastTopIndex = m_Text.getTopIndex() ;
			}			
		}		
	}
	
	/********************************************************************************************
	 * 
	 * Adds text to the view one level deep in the tree (so normally it is hidden until the user
	 * expands the block).
	 * 
	 * If autoexpand is true any newly created blocks are expanded immediately so the user doesn't
	 * need to expand the block manually.
	 * 
	 * @param text
	 * @param autoExpand
	********************************************************************************************/
	public void appendSubText(String text, boolean autoExpand, long type)
	{
		// This is needed because the text control (on Windows) stores newlines as \r\n and selections and character counts will get out of synch if we
		// work in the text control but reason about the text and they have different newlines.
		// (We still use \n everywhere else as the newline marker because that's what Soar uses)
		text = text.replaceAll(AbstractView.kLineSeparator, AbstractView.kSystemLineSeparator) ;
	
		boolean show = true ;
		if (m_FilteringEnabled)
		{
			// Record the text in our master filter document.
			// Also decide if we should show this particular line of text.
			show = m_FilterDoc.addRecord(text, true, type) ;
		}

		if (show)
			appendSubTextInternal(text, autoExpand) ;
	}
	
	public Composite getWindow() 	 { return m_Container ; }
	public Text      getTextWindow() { return m_Text ; }

	/** Turns drawing on or off.  When turned back on we refresh everything */
	public void setRedraw(boolean state)
	{
		// If we're already in the desired state do nothing
		if (m_DrawingDisabled == !state)
			return ;
		
		m_DrawingDisabled = !state ;
		m_Text.setRedraw(state) ;
		m_IconBar.redraw() ;
	}
	
	protected void iconBarMouseClick(MouseEvent e)
	{
		// Make sure the text control is properly initialized
		if (m_Text.getLineHeight() == 0)
			return ;

		int topLine = m_Text.getTopIndex() ;
		int lineHeight = m_Text.getLineHeight() ;

		int line = (e.y / lineHeight) + topLine ;
		
		// By using the "getBlockByLineNumber" method we get either the start
		// of a block or the block that encloses this line.
		// This means that clicking on the line beneath an expanded block will
		// cause it to collapse.  I think that's a nice feature, but if we'd rather
		// not have that happen switch this to getBlockStartsAtLineNumber() and
		// only clicks right on the icon will cause behavior.
		Block block = m_FoldingDoc.getBlockByLineNumber(line) ;

		if (block == null)
			return ;
		
		m_FoldingDoc.expandBlock(block, !block.isExpanded()) ;
		m_IconBar.redraw() ;
	}
	
	protected void paintIcons(PaintEvent e)
	{	
		// Check if we've turned off redraws
		if (m_DrawingDisabled)
			return ;
		
		GC gc = e.gc;
		
		Rectangle client = m_IconBar.getClientArea ();

		// Make sure the text control is properly initialized
		if (m_Text.getLineHeight() == 0)
			return ;

		// Get all the information about which part of the text window is visible
		int topLine = m_Text.getTopIndex() ;
		int lineHeight = m_Text.getLineHeight() ;
		int visibleLines = m_Text.getClientArea().height / lineHeight ;
		int lastLine = Math.min(m_Text.getLineCount(),m_Text.getTopIndex() + visibleLines) ;
		
		// Start with the first block that starts at topLine or includes topLine.
		Block topBlock = m_FoldingDoc.getBlockByLineNumber(topLine) ;
		int blockCount = m_FoldingDoc.getNumberBlocks() ;
		
		if (topBlock == null)
			return ;
		
		int blockIndex = topBlock.getIndex() ;

		int outerSize = 11 ;
		int innerSize = 7 ;
		int offset1 = (client.width - outerSize) / 2 ;
		int offset2 = (client.width - innerSize) / 2 ;
		
		Color gray  = m_IconBar.getDisplay().getSystemColor(SWT.COLOR_GRAY) ;
		Color black = m_IconBar.getDisplay().getSystemColor(SWT.COLOR_BLACK) ;

		// Go through each block in turn until we're off the bottom of the screen
		// or at the end of the list of blocks drawing icons
		while (blockIndex != -1 && blockIndex < blockCount)
		{
			Block block = m_FoldingDoc.getBlock(blockIndex) ;
			
			int line = block.getStart() ;
			
			// Once we drop off the bottom of the screen we're done
			if (line >= lastLine)
				break ;
		
			int pos = line - topLine ;
			int y = pos * lineHeight ;
				
			boolean expanded = block.isExpanded() ;
			
			if (block.canExpand())
			{
				gc.drawRectangle(offset1, y + offset1, outerSize-1, outerSize-1) ;

				// Start with a - sign
				int y1 = y + lineHeight/2 - 2;
				gc.drawLine(offset2, y1, offset2 + innerSize-1, y1) ;

				if (!expanded)
				{
					// If not expanded turn the - into a +
					int x1 = client.width / 2 ;
					gc.drawLine(x1, y + offset2, x1, y + offset2 + innerSize-1) ;
				}
				else
				{
					// If expanded draw a line to show what is in the expanded area
					gc.setForeground(gray) ;
					int x = client.width / 2 ;
					int yTop = y + offset1 + outerSize ;
					int yBottom = y + (block.getSize() * lineHeight) - lineHeight/2 ;
					gc.drawLine(x, yTop, x, yBottom) ;
					gc.drawLine(x, yBottom, client.width-1, yBottom) ;
					gc.setForeground(black) ;
				}
			}
			blockIndex++ ;				
		}
	}
}
