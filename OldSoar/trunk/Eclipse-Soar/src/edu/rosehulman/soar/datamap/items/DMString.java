/*
 * Created on Dec 28, 2003
 *
 * To change the template for this generated file go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
package edu.rosehulman.soar.datamap.items;

import org.eclipse.ui.part.*;
import org.eclipse.jface.dialogs.*;
import org.eclipse.core.resources.*;

import edu.rosehulman.soar.datamap.*;
import edu.rosehulman.soar.datamap.validators.*;

/**
 *
 * Represents a string in the Datamap. See {@link DMItem} for details}
 * 
 * @author Tim Jasko
 */
public class DMString extends DMItem {

	public DMString() {
		super();
	}
	
	public DMString(String name) {
		super(name);
	}
	
	public String toString() {
		String temp = getName() + ": string";
		
		String comment = getComment(); 
		
		if (comment != null && !comment.equals("")) {
			temp += "       * " + comment + " *";
		} // if
		
		return temp;
	}

	public String getTypeName() {
		return "String";
	}

	
	public int initValuesDialog(WorkbenchPart parent) {
		InputDialog input = new InputDialog(parent.getSite().getShell(),
			"Enter String", "Attribute name:", "", new AttributeNameValidator());
		
		int res = input.open();
		
		_name = input.getValue();
		
		return res;
	}


	public boolean canEditValues() {
		return false;
	}

	public int editValuesDialog(WorkbenchPart parent) {
		// No editing of values here!
		return 0;
	}
	
	public DMItem createNew(IFile file) {
		DMItem ret = new DMString();
		
		ret._id = DataMap.getCurrentID(file);
		DataMap.incrementCurrentID(file);
		
		return ret;
	}
	
	public String getXML(int depth) {
		String ret = "";
		String tabs = "";
		
		for (int i=0; i<depth; i++) {
			tabs += '\t';
		} // for i
		
		ret += tabs;
		
		ret += "<" + getTypeName() + getSharedXML() 
			+ " />\n";
		
		
		return ret;
	}
	
	public boolean isValidValue(String val) {
		return true;
		
	}
	
	
	public DMItem copy() {
		DMString ret = new DMString(getName());
		
		ret._comment = this._comment;
		
		return ret;
	}


}
