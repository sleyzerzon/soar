/**
 *
 * @file DatamapEditorInput.java
 * @date Jun 14, 2004
 */
package edu.rosehulman.soar.datamap.editor;


import edu.rosehulman.soar.*;

import org.eclipse.core.resources.*;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.ui.IFileEditorInput;
import org.eclipse.ui.IPersistableElement;


/**
 * 
 * 
 * @author Tim Jasko (tj9582@yahoo.com)
 */
public class DatamapEditorInput implements IFileEditorInput {
	private IFile _file;
	private int _rootID;
	
	public DatamapEditorInput(IFile file, int rootID) {
		_file = file;
		_rootID = rootID;
	}

	public IFile getFile() {
		return _file;
	}
	
	
	public boolean equals( Object o) {
		if (o instanceof DatamapEditorInput) {
			DatamapEditorInput temp = (DatamapEditorInput) o;
			
			if (_rootID != temp.getRootID())
				return false;
			
			if (! _file.equals(temp.getFile()))
				return false;
			
			return true;
		}
		
		return false;
	}
	
	public int getRootID() {
		return _rootID;
	}


	public IStorage getStorage() throws CoreException {
		return _file;
	}


	public boolean exists() {
		return true;
	}


	public ImageDescriptor getImageDescriptor() {
		return SoarImages.getImageDescriptor(SoarImages.IMAGE_SOAR);
	}


	public String getName() {
		return _file.getName();
	}


	public IPersistableElement getPersistable() {
		return new DatamapInputPersistable(this);
	}


	public String getToolTipText() {
		return _file.getFullPath().toString();
	}


	public Object getAdapter(Class adapter) {
		return null;
	}

}
