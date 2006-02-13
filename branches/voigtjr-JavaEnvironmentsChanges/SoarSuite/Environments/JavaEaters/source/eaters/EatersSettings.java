package eaters;

import java.util.ArrayList;

import base.Settings;

public class EatersSettings extends Settings {
	
	// Internal settings (not command line)
	public final int NORMAL_FOOD_WORTH = 5;
	public final int BONUS_FOOD_WORTH = 10;
	public final int BONUS_FOOD_COLUMNS = 3;
	public static final int Wall = 0;
	public static final int NormalFood = 1;
	public static final int BonusFood = 2;
	public static final int Empty = 3;
	public final int JUMP_COST = 5;
	public final double PROBABILITY = .15;			// For map generation
	public final double BETTER_PROBABILITY = .65;	// For map generation
	public final String FOLDER_NAME = "JavaEaters";

	// Command line settings
	public boolean parseCommandLine(String[] args) {
		ArrayList seaterFileList = new ArrayList();
		for (int i = 0; i < args.length; i++) {
			// TODO: should use constant here!
			if (args[i].toLowerCase().endsWith(".seater")) {
				seaterFileList.add(args[i]);
			}
		}
		if (seaterFileList.size() > 0) {
			soarFiles = (String[]) seaterFileList.toArray(new String[0]);
		}
	
		if (!super.parseCommandLine(args)) {
			printCommandLineHelp();
			return false;
		}
		return true;
	}

	protected void printCommandLineHelp() {
		System.out.println("Java Eaters help");
		System.out.println("\t*.seater: Names of .seater files for eater agents.");
		super.printCommandLineHelp();
	}
	
	public String toString() {
		// TODO: add internal settings!
		// soarFiles get printed in higher level
		return super.toString();
	}
}
