/* File: Eaters.java
 * Jul 12, 2004
 */

package eaters;

public class Eaters {

	public static void main(String[] args)
	{
		
		EatersSettings settings = new EatersSettings(); 
		if (!settings.parseCommandLine(args)) {
			return;
		}

		EaterControl control = new EaterControl(settings);
		
		if(!settings.nowindows){
			new EatersWindowManager(control);
		}
	}
}
