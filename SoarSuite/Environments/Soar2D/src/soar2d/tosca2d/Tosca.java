/**
 * 
 */
package soar2d.tosca2d;

import tosca.FunctionModule;
import tosca.StateVariable;
import tosca.Value;
import tosca.Vector;

/**
 * @author Doug
 *
 * High level interface to the Tosca framework.
 * 
 * This class handles startup, shutdown etc.
 * The specific agent I/O happens in ToscaEater.
 */
public class Tosca {
	private static Tosca s_Tosca = null ;
	private LoaderJNI m_Loader = null ;
	private Value	  m_Value = null ;
	
	public static void test() {
		// Only do this once for now
		if (s_Tosca != null)
			return ;
		
		Tosca tosca = new Tosca() ;
		s_Tosca = tosca ;
		
		try {
			tosca.testGroup() ;
		}
		catch (Exception ex) {
			ex.printStackTrace() ;
		}
	}
	
	public void testGroup() {
		// Loads our test C++ code and gets a pointer to it
		LoaderJNI loader = new LoaderJNI("EatersDLL") ;
		long rawLibrarian = loader.Librarian_GetExistingLibrarian() ;
		m_Loader = loader ;
		
	    //tosca.Librarian librarian = tosca.Librarian.GetLibrarian() ;
		tosca.Librarian librarian = tosca.Librarian.UseExistingLibrarian(rawLibrarian) ;
	    int libs = librarian.GetNumberLibraries() ;
	    System.out.println("Number of libraries is " + libs) ;
	    tosca.Library library = librarian.GetLibraryByName("eaters") ;
	    if (library == null)
	    	System.out.println("Failed to find C++ library") ;
	    else
	    	System.out.println("Found C++ library") ;

	    // Initialize the input in Java (this is the key process--providing input from Java as an environment to C++ clustering code)
	    int size = 5 ;
	    Vector inputVector = new Vector(size) ;
	    for (int i = 0 ; i < size ; i++)
	    	inputVector.Set(i, 1.0) ;
	    Value inputValue = new Value(inputVector) ;
	    
	    // BUGBUG: If this gets garbage collected after threads have shutdown/everything's over we seem
	    // to get a crash.  Need to explore the behavior of Value objects in Java better.
	    // OK, in C++ the ref count for the inputVector's internal object would increase.
	    // The problem is perhaps that we delete that object rather than releasing it.
	    //m_Value = inputValue ;
	    
	    final StateVariable input = library.GetStateVariable("Input") ;
	    input.SetValue(inputValue, 1) ;
	    
		final tosca.Clock	clock = library.GetClock() ;

		boolean start = true ;
		
		// Print out all function modules
		int nModules = library.GetNumberFunctionModules() ;
		for (int i = 0 ; i < nModules ; i++)
			System.out.println("Module " + i + " has name " + library.GetFunctionModuleName(i)) ;
		
		if (start)
		{
			clock.SetTimeLimit(100) ;
			//clock.StartOwnThread() ;
			
			/*
			final FunctionModule findWinner = library.GetFunctionModule("findWinner") ;
			final FunctionModule updateWeights = library.GetFunctionModule("updateWeights") ;
			final FunctionModule updateInput = library.GetFunctionModule("updateInput") ;
			findWinner.StartOwnThread() ;
			updateWeights.StartOwnThread() ;
			updateInput.StartOwnThread() ;
			*/
			
			library.StartAllThreads(true);
	
			// Sleep for 2 secs, 0ms
			try { Thread.sleep(2000) ; } catch (InterruptedException ex) { }

			library.StopAllThreads(true, true);
			//System.exit(0) ;
		}
		
		System.out.println("===================================") ;
		System.out.println("====    Finished TestGroup     ====") ;
		System.out.println("===================================") ;
	}
}
