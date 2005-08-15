package edu.rosehulman.soar.navigator;


import edu.rosehulman.soar.*;

import org.eclipse.jface.viewers.*;
import org.eclipse.core.resources.*;

public class SoarLightweightDecorator implements ILightweightLabelDecorator {

	public void decorate(Object object, IDecoration decoration) {
		if (object instanceof IResource) {
			IResource file = (IResource)object;
			String type = FileMarker.getSoarType(file);
			
			
			if (type.equals("file")) {
				decoration.addOverlay(SoarImages.getImageDescriptor(SoarImages.FILE));
			} else if (type.equals("operator")) {
				decoration.addOverlay(SoarImages.getImageDescriptor(SoarImages.OPERATOR));
			} else if (type.equals("impasse")) {
				decoration.addOverlay(SoarImages.getImageDescriptor(SoarImages.IMPASSE));
			}
		}
	}

	public void addListener(ILabelProviderListener listener) {
		// TODO Auto-generated method stub

	}

	public void dispose() {
		// TODO Auto-generated method stub

	}

	public boolean isLabelProperty(Object element, String property) {
		System.out.println(element + ":" + property);
		return false;
	}

	public void removeListener(ILabelProviderListener listener) {
		// TODO Auto-generated method stub
	}

}
