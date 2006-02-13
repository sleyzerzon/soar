/* File: Eaters.java
 * Jul 12, 2004
 */

package eaters;

import base.Logger;

public class Eaters {

	public static void main(String[] args)
	{
		
		EatersSettings settings = new EatersSettings(); 
		if (!settings.parseCommandLine(args)) {
			return;
		}

		EaterControl control = new EaterControl(settings);
		
		// TODO: implement run call
		//if (settings.run) {
		//	control.run();
		//}

		// TODO: implement window mananger
		//if(!settings.nowindows){
		//	EaterSWindowManager wm = new EaterSWindowManager(control);
		//}
		
		// TODO: Check if this next call is still necessary.
		System.exit(0);
	}
}
