/********************************************************************************************
*
* Module.java
* 
* Created on 	Nov 22, 2003
*
* @author 		Doug
* @version
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package doc;

import general.* ;
import modules.* ;
import debugger.* ;

/********************************************************************************************
* 
* Instances of this class represent modules (types of windows) that can be created in
* the debugger.
* 
* So for example, a the ComboCommandWindow class might be recorded in an instance of this class.
* 
********************************************************************************************/
public class Module
{
	/** The name shown to the user for this class */
	private String 		m_Name ;
	
	/** A description shown to the user for this class */
	private String		m_Description ;
	
	/** The class itself */
	private Class		m_Class ;
	
	public Module(String name, String description, Class c)
	{
		m_Name  = name ;
		m_Description = description ;
		m_Class = c ;
	}
	
	public String toString() 		{ return m_Name ; }
	
	public String getDescription() 	{ return m_Description ; }
	
	/** Creates a new instance of this class or returns null if that fails */
	
	public AbstractView createInstance(MainFrame frame, Document doc)
	{
		try
		{
			// Construct the new object and set the document pointer and channel immediately.
			AbstractView window = (AbstractView)m_Class.newInstance() ;
			//window.setDocument(doc) ;
			//window.setMainFrame(frame) ;
			
			return window ;
		}
		catch (Exception e)
		{
			Debug.println("Exception thrown while trying to instantiate " + m_Name) ;
			Debug.println(e.getMessage()) ;
			return null ;
		}
	}
	
}
