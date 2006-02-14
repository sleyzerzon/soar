/* File: Eaters.java
 * Jul 12, 2004
 */

package eaters;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

public class Eaters {
	
	public Eaters(String[] args) {
		
		// Install default settings file
		try {
			Install("eaters-default-settings.xml");
		} catch (IOException e) {
			System.out.println("Error installing default settings: " + e.getMessage());
			System.exit(1);
		}
		
		// Deal with the command line
		EatersSettings settings = new EatersSettings(); 
		if (!settings.parseCommandLine(args)) {
			return;
		}
		
		// Initialize the simulation
		EatersSimulation simulation = new EatersSimulation(settings);
		
		// Initialize the window manager, if applicable.
		if(!settings.isQuiet()){
			new EatersWindowManager(simulation);
		}
		
		// TODO: Run simulation if quiet!
	}

	private void Install(String file) throws IOException
	{	
		// We just work relative to the current directory because that's how
		// load library will work.
		File library = new File(file) ;

		if (library.exists())
		{
			System.out.println(library + " already exists so not installing from the JAR file") ;
			return;
		}
		
		// Get the DLL from inside the JAR file
		// It should be placed at the root of the JAR (not in a subfolder)
		String jarpath = "/" + library.getPath() ;
		InputStream is = this.getClass().getResourceAsStream(jarpath) ;
		
		if (is == null)
		{
			System.out.println("Failed to find " + jarpath + " in the JAR file") ;
			return;
		}
		
		// Make sure we can delete the library.  This is actually here to cover the
		// case where we're running in Eclipse without a JAR file.  The getResourceAsStream()
		// call can end up loading the same library that we're trying to save to and we
		// end up with a blank file.  Explicitly trying to delete it first ensures that
		// we're not reading the same file that we're writing.
		if (library.exists() && !library.delete())
		{
			System.out.println("Failed to remove the existing layout file " + library) ;
			return;
		}
		
		// Create the new file on disk
		FileOutputStream os = new FileOutputStream(library) ;
		
		// Copy the file onto disk
		byte bytes[] = new byte[2048];
		int read;
		while (true)
		{
			read = is.read( bytes) ;
			
			// EOF
			if ( read == -1 ) break;
			
			os.write( bytes, 0, read);
		}

		is.close() ;
		os.close() ;
		
		System.out.println("Installed " + library + " onto the local disk from JAR file") ;
	}

	public static void main(String[] args)
	{
		new Eaters(args);
	}
}
