/********************************************************************************************
*
* DocumentThread.java
* 
* Description:	
* 
* Created on 	Feb 21, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package doc;

import java.util.ArrayList;

import sml.Agent;
import sml.AnalyzeXML;

/************************************************************************
 * 
 * Executing Soar commands can take a long time (e.g. "run 1000").
 * 
 * If we execute them in the main UI thread then the UI will be locked until
 * Soar returns control to us (menus can't be selected etc.).
 * 
 * One solution to this is to execute the Soar commands in a separate thread
 * within the debugger.  This class does just that, queueing up commands
 * and executing them.
 * 
 ************************************************************************/
public class DocumentThread extends Thread
{
	// BUGBUG: There's a chance that the agent object is invalid
	// by the time we go to execute the command (if it's been deleted while this command was pending to execute).
	// We need a way to detect that
	// the C++ object is no good before we try to run the command.
	public static class Command
	{
		private Agent m_Agent ;
		private String m_Command ;
		private String m_Result ;
		private AnalyzeXML m_Response ;
		private boolean m_Executed ;
		
		public Command(Agent agent, String command, AnalyzeXML response)
		{
			m_Agent = agent ;
			m_Command = command ;
			m_Executed = false ;
			m_Response = response ;
		}
	}
	
	/** The commands waiting to be executed */
	private ArrayList m_ToExecuteQueue = new ArrayList() ;
	
	/** A flag used when we wish to stop this thread (during system shutdown) */
	private boolean   m_AskedToStop = false ;
	
	/** The main document (which owns the Soar kernel object etc.) */
	private Document  m_Document = null ;
	
	private boolean	  m_IsExecutingCommand = false ;
	
	public DocumentThread(Document doc)
	{
		m_Document = doc ;
	}
	
	/** Ask this thread to halt */
	public synchronized void askToStop()
	{
		m_AskedToStop = true ;
	}

	/** Returns true if we're actively executing a command or are just about to */
	public synchronized boolean isBusy()
	{
		return m_IsExecutingCommand || m_ToExecuteQueue.size() > 0 ;
	}
	
	/** Schedule a command to execute later (response can be null) */
	public synchronized Command scheduleCommandToExecute(Agent agent, String commandLine, AnalyzeXML response)
	{
		Command command = new Command(agent, commandLine, response) ;
		m_ToExecuteQueue.add(command) ;
		
		return command ;
	}
	
	/** Get the next command from the queue.  Returns null if there are no commands */
	private synchronized Command popNextCommand()
	{
		if (m_ToExecuteQueue.size() == 0)
			return null ;
		
		Command command = (Command)m_ToExecuteQueue.get(0) ;
		m_ToExecuteQueue.remove(0) ;

		return command ;
	}
	
	public synchronized boolean isExecutedCommand(Command command)
	{
		return command.m_Executed ;
	}	

	public synchronized String getExecutedCommandResult(Command command)
	{
		if (!command.m_Executed)
			throw new IllegalStateException("Command has not been executed yet -- should check isExecutedCommand first") ;
		
		return command.m_Result ;
	}
	
	private synchronized void recordCommandResult(Command command, String result)
	{
		command.m_Result = result ;
		command.m_Executed = true ;
	}

	// This function pulls waiting commands off the queue and executes them in Soar.
	// We expose this so that we can call it during the execution of a run to check for "stop-soar" commands.
	public void executePending()
	{
		Command command ;
		while ( (command = popNextCommand()) != null )
		{
			m_IsExecutingCommand = true ;
			
			if (command.m_Response == null)
			{
				// If there's no XML object to receive the response we just execute and get the string form.
				String result = command.m_Agent.ExecuteCommandLine(command.m_Command) ;
				recordCommandResult(command, result) ;
			}
			else
			{
				// If we were provided an XML object for the response, record the result in that object
				boolean success = command.m_Agent.ExecuteCommandLineXML(command.m_Command, command.m_Response) ;
				recordCommandResult(command, success ? "true" : "false") ;
			}
		}
		
		m_IsExecutingCommand = false ;
	}
	
	public void run()
	{
		while (!m_AskedToStop)
		{
			executePending() ;
			
			// The pause as we sleep here is just how quickly we respond to incoming commands from the user.
			// It won't affect the speed Soar executes.
			try { Thread.sleep(10) ; } catch (InterruptedException e) { } 
		}
	}
}