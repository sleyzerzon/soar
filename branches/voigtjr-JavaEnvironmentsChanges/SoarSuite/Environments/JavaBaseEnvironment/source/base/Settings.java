package base;

import java.util.ArrayList;

public abstract class Settings {

	// Internal settings (not command line)
	protected final String AGENT_FOLDER = "agents";
	protected final String MAP_FOLDER = "maps";
	protected final String PROJECT_FOLDER = "Environments";
	public  final int MAP_WIDTH = 17;
	public  final int MAP_HEIGHT = 17;
		
	// Command line settings
	public boolean nomap = false;
	public boolean nocp = false;
	public boolean noagents = false;
	public boolean nodebuggers = false;
	public boolean nowindows = false;
	public boolean run = false;
	public String logfile = null;
	public boolean clearlog = false;
	public int loglevel = 0;
	public int errorlevel = 0;
	public String map = null;
	public String[] soarFiles = null;

	public String toString() {
		// TODO: add internal settings!
		String ret = new String();
		if (nomap) ret += "-nomap ";
		if (nocp) ret += "-nocp ";
		if (noagents) ret += "-noagents ";
		if (nodebuggers) ret += "-nodebuggers ";
		if (nowindows) ret += "-nowindows ";
		if (run) ret += "-run ";
		if (logfile != null) ret += "-logfile " + logfile + " ";
		if (clearlog) ret += "-clearlog ";
		ret += "-loglevel " + loglevel + " ";
		ret += "-errorlevel " + errorlevel + " ";
		if (map != null) ret += "-map " + map + " ";
		if (nomap) ret += "-nomap ";
		
		if (soarFiles != null) {
			for (int i = 0; i < soarFiles.length; ++i) ret += soarFiles[i] + " ";
		}
			
		if (nowindows) {
			nomap = true;
			noagents = true;
			nocp = true;
			nodebuggers = true;
		}
		
		return ret;
	}
	
	protected void printCommandLineHelp() {
		System.out.println("\t-nomap: Disables the map window.");
		System.out.println("\t-nocp: Disables the control panel.");
		System.out.println("\t-noagents: Disables creation of agent view window when agent created.");
		System.out.println("\t-nodebuggers: Disables debugger spawn on agent creation.");
		System.out.println("\t-nowindows: Disables all windows, runs simulation quietly.");
		System.out.println("\t-run: Start running immediately after agents connect.");
		System.out.println("\t-andy: Update the world once per cycle instead of the perscribed 15 cycles (tanksoar only).");
		System.out.println("\t-logfile: Name of a log file to store log information");
		System.out.println("\t-clearlog: Clears log before running.");
		System.out.println("\t-loglevel: Log verbosity: ranges from 0 to 5 (inclusive) see Eaters Notes.txt");
		System.out.println("\t-errorlevel: Seater error level: 0 = none, 1 = warn, 2 = quit (see Eaters Notes.txt)");
		System.out.println("\t-map: Name of a map file to load when simulation begins.");
		System.out.println("\t*.soar: Names of .soar files for soar agents.");
	}

	protected boolean parseCommandLine(String[] args) {

		if (hasOption(args, "-?") || hasOption(args, "-help")) {
			return false;
		}
		
		nomap 		= hasOption(args, "-nomap");
		nocp 		= hasOption(args, "-nocp");
		noagents 	= hasOption(args, "-noagents");
		nowindows 	= hasOption(args, "-nowindows");
		nodebuggers = hasOption(args, "-nodebuggers");
		run 		= hasOption(args, "-run");
		clearlog 	= hasOption(args, "-clearlog");
		
		map 		= getOptionValue(args, "-map");
		logfile 	= getOptionValue(args, "-logfile");
		
		String errorlevelString = getOptionValue(args, "-errorlevel");
		if (errorlevelString != null) {			
			errorlevel 	= Integer.parseInt(errorlevelString);
		}
		
		String loglevelString = getOptionValue(args, "-loglevel");
		if (loglevelString != null) {			
			loglevel 	= Integer.parseInt(loglevelString);
		}
		
		ArrayList soarFileList = new ArrayList();
		if (soarFiles != null) {
			for (int i = 0; i < soarFiles.length; ++i) {
				soarFileList.add(soarFiles[i]);
			}
		}
		for (int i = 0; i < args.length; ++i) {
			if (args[i].toLowerCase().endsWith(".soar")) {
				soarFileList.add(args[i]);
			}
		}
		if (soarFileList.size() > 0) {
			soarFiles = (String[]) soarFileList.toArray(new String[0]);
		}
	
		return true;
	}
	
	// Returns true if a given option appears in the list
	// (Use this for simple flags like -remote)
	protected boolean hasOption(String[] args, String option)
	{
		for (int i = 0 ; i < args.length ; i++)
		{
			if (args[i].equalsIgnoreCase(option))
				return true ;
		}
		
		return false ;
	}	
	
	// Returns the next argument after the matching option.
	// (Use this for parameters like -port ppp)
	protected String getOptionValue(String[] args, String option)
	{
		for (int i = 0 ; i < args.length-1 ; i++)
		{
			if (args[i].equalsIgnoreCase(option))
				return args[i+1] ;
		}
		
		return null ;
	}
}
