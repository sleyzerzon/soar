package edu.umich.soar;

import sml.Agent;
import sml.Kernel;
import sml.Kernel.UpdateEventInterface;

public class Leaky {

	static final private boolean REG = true;
	
	/**
	 * @param args
	 */
	public static void main(String[] args) {
		Kernel kernel = Kernel.CreateKernelInNewThread();
		Agent agent = kernel.CreateAgent("leaky");
		
		agent.ExecuteCommandLine(
				"sp {proposal-elaboration " +
				"   (state <s> ^superstate nil " +
				"             -^operator) " +
				"--> " +
				"   (<s> ^operator <op> +) " +
				"}");

		long e = 0;
		if (REG) {
			e = kernel.RegisterForUpdateEvent(sml.smlUpdateEventId.smlEVENT_AFTER_ALL_OUTPUT_PHASES, new UpdateEventInterface() {
				@Override
				public void updateEventHandler(int arg0, Object arg1, Kernel arg2, int arg3) {
				}
			}, null);
		}
		
		//agent.RunSelf(100000);
		agent.RunSelfForever();
		
		if (REG) {
			kernel.UnregisterForUpdateEvent(e);
		}
		
		kernel.DestroyAgent(agent);
		kernel.Shutdown();
	}

}
