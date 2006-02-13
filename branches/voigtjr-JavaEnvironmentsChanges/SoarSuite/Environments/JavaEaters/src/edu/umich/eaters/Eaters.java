/* File: Eaters.java
 * Jul 12, 2004
 */

package edu.umich.eaters;

import edu.umich.JavaBaseEnvironment.Logger;

import java.io.File;

/**
 * The main class to run Eaters.
 * 
 * @author John Duchi
 * @author Jonathan Voigt
 */
public class Eaters {

	public static void main(String[] args)
	{
		
		EatersArgumentContainer ac = new EatersArgumentContainer(); 
		if (!ac.parse(args)) {
			return;
		}

		EaterControl ec = new EaterControl(ac);
		
		if(ac.soarFiles != null){
			for(int i = 0; i < ac.soarFiles.length; i++){
				ec.loadAgent(new File(ac.soarFiles[i]));
			}
		}
		if (ac.run)
			ec.run();

		if(!ac.nowindows){
			EaterSWindowManager wm = new EaterSWindowManager(ec, !ac.nowindows, !ac.nomap, !ac.nocp, ac.run);
		}
		
		System.exit(0);//Oddity in SML requires this to ensure no memory leaked
	}
}
