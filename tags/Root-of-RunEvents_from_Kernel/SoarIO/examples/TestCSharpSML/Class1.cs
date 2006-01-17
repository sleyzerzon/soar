using System;

namespace TestCSharpSML
{
	/// <summary>
	/// Summary description for Class1.
	/// </summary>
	class Class1
	{
		static void MyTestCallback()
		{
			System.Console.Out.WriteLine("Called back successfully") ;
		}

		static void MyTestRunCallback(sml.smlRunEventId eventID)
		{
			System.Console.Out.WriteLine("Called back successfully " + eventID) ;
		}

		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main(string[] args)
		{
			//
			// TODO: Add code to start application here
			//
			sml.Kernel kernel = sml.Kernel.CreateKernelInNewThread("SoarKernelSML") ;

			sml.Agent agent = kernel.CreateAgent("First") ;
			bool ok = agent.LoadProductions("testcsharpsml.soar") ;

			System.Console.Out.WriteLine(ok ? "Loaded successfully" : "Failed") ; 

			sml.Agent.MyRunCallback call = new sml.Agent.MyRunCallback(MyTestRunCallback) ;			
			int callbackID = agent.RegisterForRunEvent(sml.smlRunEventId.smlEVENT_AFTER_DECISION_CYCLE, call) ;

			agent.RunSelf(3) ;

			sml.Kernel.MyCallback callback = new sml.Kernel.MyCallback(MyTestCallback) ;
			kernel.RegisterTestMethod(callback) ;

			ok = agent.UnregisterForRunEvent(callbackID) ;

			System.Console.Out.WriteLine(ok ? "Unregistered run event successfully" : "Failed to unregister") ;

			kernel.Shutdown() ;

			// C# delete
			kernel.Dispose() ;

			System.Console.Out.WriteLine("Press return to exit") ;
			System.Console.In.ReadLine() ;
		}
	}
}
