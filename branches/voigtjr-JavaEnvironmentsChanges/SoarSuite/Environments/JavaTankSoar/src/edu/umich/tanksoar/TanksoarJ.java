package edu.umich.tanksoar;

/* File: TanksoarJ.java
 * Aug 11, 2004
 */

import java.io.File;

/**
 * @author John Duchi
 * @author Jonathan Voigt
 */
public class TanksoarJ {

	public static void main(String[] args) {
		
		TankSoarArgumentContainer ac = new TankSoarArgumentContainer(); 
		if (!ac.parse(args)) {
			return;
		}

		TankSoarJControl tc = new TankSoarJControl(ac);
		
		if(ac.soarFiles != null){
			for(int i = 0; i < ac.soarFiles.length; i++){
				tc.loadAgent(new File(ac.soarFiles[i]));
			}
		}

		//Have to update all tanks here, because a tool might cause the Soar kernel
		// to start running at any moment. The tanks must already be set to run
		// TODO: check if this is necessary in eaters
		tc.updateAllTanks();

		if (ac.run) {
			tc.run();
		}
		
		if(!ac.nowindows){
			TankSWindowManager wm = new TankSWindowManager(tc, !ac.nowindows, !ac.nomap, !ac.nocp, ac.run);
		} else {
			// TODO: check if this is necessary in eaters
			if (tc.getAllAgents().length != 0) {
				tc.runSimulation();
				tc.printMap();
				System.out.println("World count: " + tc.getWorldCount());
			} else {
				System.out.println("No agents specified. Quitting.");
				//return;
			}
			tc.simQuit();
		}
		
		System.exit(0);//Oddity in SML requires this to ensure no memory leaked
	}
}