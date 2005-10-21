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

			sml.Kernel.MyCallback callback = new sml.Kernel.MyCallback(MyTestCallback) ;
			kernel.RegisterTestMethod(callback) ;

			kernel.Shutdown() ;

			// C# delete
			kernel.Dispose() ;
		}
	}
}
