package edu.umich.soar.gridmap2d.soar;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.HashMap;
import java.util.Map;
import java.util.Map.Entry;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.SynchronousQueue;
import java.util.concurrent.TimeUnit;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.gridmap2d.config.ClientConfig;
import edu.umich.soar.gridmap2d.config.SimConfig;
import edu.umich.soar.gridmap2d.config.SoarConfig;
import edu.umich.soar.gridmap2d.core.CognitiveArchitecture;
import edu.umich.soar.gridmap2d.core.Names;
import edu.umich.soar.gridmap2d.core.Simulation;
import edu.umich.soar.gridmap2d.core.events.AfterTickEvent;
import edu.umich.soar.gridmap2d.core.events.BeforeTickEvent;
import edu.umich.soar.gridmap2d.core.events.StartEvent;
import edu.umich.soar.gridmap2d.core.events.StopEvent;
import edu.umich.soar.gridmap2d.events.SimEvent;
import edu.umich.soar.gridmap2d.events.SimEventListener;
import edu.umich.soar.gridmap2d.map.Eater;
import edu.umich.soar.gridmap2d.map.EaterCommander;
import edu.umich.soar.gridmap2d.map.Robot;
import edu.umich.soar.gridmap2d.map.RobotCommander;
import edu.umich.soar.gridmap2d.map.RoomWorld;
import edu.umich.soar.gridmap2d.map.Tank;
import edu.umich.soar.gridmap2d.map.TankCommander;
import edu.umich.soar.gridmap2d.map.Taxi;
import edu.umich.soar.gridmap2d.map.TaxiCommander;

import sml.Agent;
import sml.ConnectionInfo;
import sml.Kernel;
import sml.smlPrintEventId;
import sml.smlUpdateEventId;
import sml.sml_Names;
import sml.smlRunFlags;

public class Soar implements CognitiveArchitecture, Kernel.UpdateEventInterface, SimEventListener {

	private static Log logger = LogFactory.getLog(Soar.class);

	private boolean runTilOutput = false;
	private Kernel kernel = null;

	private class AgentData {
		AgentData(Agent agent, File productions) {
			this.agent = agent;
			this.productions = productions;
		}

		Agent agent;
		File productions;
		SoarAgent sa;
	}

	private final Map<String, AgentData> agents = new HashMap<String, AgentData>();
	private final Map<String, ClientConfig> clients;
	private final int maxMemoryUsage;
	private final boolean soarPrint;
	private final int port;
	private boolean debug;
	private final Simulation sim;

	/**
	 * @param config
	 * @param clients
	 * @param game
	 * @param basePath
	 * 
	 * @throws IllegalStateException
	 *             If there is an unrecoverable error initializing Soar
	 */
	public Soar(Simulation sim) {
		this.sim = sim;
		SoarConfig config = sim.getConfig().soarConfig();
		this.runTilOutput = config.runTilOutput(sim.getConfig().game());

		this.clients = sim.getConfig().clientConfigs();
		this.maxMemoryUsage = config.max_memory_usage;
		this.soarPrint = config.soar_print;
		this.port = config.port;
		this.debug = config.spawn_debuggers;

		if (config.remote != null) {
			kernel = Kernel.CreateRemoteConnection(true, config.remote,
					config.port);
		} else {
			// Create kernel
			kernel = Kernel.CreateKernelInNewThread("SoarKernelSML",
					config.port);
			// kernel = Kernel.CreateKernelInCurrentThread("SoarKernelSML",
			// true);
		}

		if (kernel.HadError()) {
			throw new IllegalStateException(Names.Errors.kernelCreation
					+ kernel.GetLastErrorDescription());
		}

		// We want the most performance
		logger.debug(Names.Debug.autoCommit);
		kernel.SetAutoCommit(false);

		// Register for Soar events
		logger.trace(Names.Trace.eventRegistration);
		if (runTilOutput) {
			logger.debug(Names.Debug.runTilOutput);
			kernel.RegisterForUpdateEvent(
					smlUpdateEventId.smlEVENT_AFTER_ALL_GENERATED_OUTPUT, this,
					null);
		} else {
			logger.debug(Names.Debug.noRunTilOutput);
			kernel.RegisterForUpdateEvent(
					smlUpdateEventId.smlEVENT_AFTER_ALL_OUTPUT_PHASES, this,
					null);
		}

		// Register for Sim events
		sim.getEvents().addListener(StartEvent.class, this);
		sim.getEvents().addListener(StopEvent.class, this);
		sim.getEvents().addListener(BeforeTickEvent.class, this);
		sim.getEvents().addListener(AfterTickEvent.class, this);
	}

	@Override
	public boolean debug() {
		return debug;
	}

	@Override
	public void seed(int seed) {
		kernel.ExecuteCommandLine("srand " + seed, null);
	}

	@Override
	public void doBeforeClients() {
		// Start or wait for clients (false: before agent creation)
		logger.trace(Names.Trace.beforeClients);
		doClients(false);

	}

	@Override
	public void doAfterClients() {
		// Start or wait for clients (true: after agent creation)
		logger.trace(Names.Trace.afterClients);
		doClients(true);

	}

	private void doClients(boolean after) {
		for (Entry<String, ClientConfig> entry : clients.entrySet()) {
			if (entry.getValue().after != after) {
				continue;
			}

			if (entry.getKey().equals(Names.kDebuggerClient)) {
				continue;
			}

			if (entry.getValue().command != null) {
				spawnClient(entry.getKey(), entry.getValue());
			} else {
				if (!waitForClient(entry.getKey(), entry.getValue())) {
					sim
							.error("Soar", Names.Errors.clientSpawn
									+ entry.getKey());
					return;
				}
			}
		}
	}

	private class Redirector extends Thread {
		BufferedReader br;

		public Redirector(BufferedReader br) {
			this.br = br;
		}

		@Override
		public void run() {
			String line;
			try {
				while ((line = br.readLine()) != null) {
					System.out.println(line);
				}
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}

	public void spawnClient(String clientID, ClientConfig clientConfig) {
		Runtime r = java.lang.Runtime.getRuntime();
		logger.trace(Names.Trace.spawningClient + clientID);

		try {
			Process p = r.exec(clientConfig.command);

			InputStream is = p.getInputStream();
			InputStreamReader isr = new InputStreamReader(is);
			BufferedReader br = new BufferedReader(isr);
			Redirector rd = new Redirector(br);
			rd.start();

			is = p.getErrorStream();
			isr = new InputStreamReader(is);
			br = new BufferedReader(isr);
			rd = new Redirector(br);
			rd.start();

			if (!waitForClient(clientID, clientConfig)) {
				sim.error("Soar", Names.Errors.clientSpawn + clientID);
				return;
			}

		} catch (IOException e) {
			e.printStackTrace();
			sim.error("Soar", "IOException spawning client: " + clientID);
			return;
		}
	}

	/**
	 * @param client
	 *            the client to wait for
	 * @return true if the client connected within the timeout
	 * 
	 *         waits for a client to report ready
	 */
	public boolean waitForClient(String clientID, ClientConfig clientConfig) {
		boolean ready = false;
		// do this loop if timeout seconds is 0 (code for wait indefinitely) or
		// if we have tries left
		for (int tries = 0; (clientConfig.timeout == 0)
				|| (tries < clientConfig.timeout); ++tries) {
			kernel.GetAllConnectionInfo();
			if (kernel.HasConnectionInfoChanged()) {
				for (int i = 0; i < kernel.GetNumberConnections(); ++i) {
					ConnectionInfo info = kernel.GetConnectionInfo(i);
					if (info.GetName().equalsIgnoreCase(clientID)) {
						if (info.GetAgentStatus().equalsIgnoreCase(
								sml_Names.getKStatusReady())) {
							ready = true;
							break;
						}
					}
				}
				if (ready) {
					break;
				}
			}
			try {
				logger.trace(Names.Trace.waitClient + clientID);
				Thread.sleep(1000);
			} catch (InterruptedException ignored) {
			}
		}
		return ready;
	}

	@Override
	public void destroyPlayer(String name) {
		// get the agent (human agents return null here)
		AgentData agentData = agents.remove(name);
		if (agentData == null) {
			return;
		}
		// there was an agent, destroy it
		kernel.DestroyAgent(agentData.agent);
		agentData.agent.delete();
	}

	@Override
	public void shutdown() {
		exec.shutdown();
		try {
			exec.awaitTermination(5, TimeUnit.MINUTES);
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		if (kernel != null) {
			logger.trace(Names.Trace.kernelShutdown);
			kernel.Shutdown();
			kernel.delete();
		}

	}

	/**
	 * Logger for Kernel print events
	 * 
	 * @author Scott Lathrop
	 * 
	 */
	private PrintLogger getLogger() {
		return PrintLogger.getLogger();
	}

	private static class PrintLogger implements Agent.PrintEventInterface {
		protected static PrintLogger m_Logger = null;

		public static PrintLogger getLogger() {
			if (m_Logger == null) {
				m_Logger = new PrintLogger();
			}

			return m_Logger;
		}

		/**
		 * @brief - callback from SoarKernel for print events
		 */
		@Override
		public void printEventHandler(int eventID, Object data, Agent agent,
				String message) {
			if (eventID == smlPrintEventId.smlEVENT_PRINT.swigValue()) {
				logger.info(message);
			}

		} // SoarAgentprintEventHandler

		private PrintLogger() {
		}

	} // Logger

	private Agent createSoarAgent(String name, String productions, boolean spawnDebuggers) {
		Agent agent = kernel.CreateAgent(name);
		if (agent == null) {
			sim.error("Soar", "Error creating agent " + name + ", "
					+ kernel.GetLastErrorDescription());
			return null;
		}

		// now load the productions
		File productionsFile = new File(productions);
		if (!agent.LoadProductions(productionsFile.getAbsolutePath())) {
			sim.error("Soar", "Error loading productions " + productionsFile
					+ " for " + name + ", " + agent.GetLastErrorDescription());
			return null;
		}

		// if requested, set max memory usage
		int maxmem = maxMemoryUsage;
		if (maxmem > 0) {
			agent.ExecuteCommandLine("max-memory-usage "
					+ Integer.toString(maxmem));
		}

		// Scott Lathrop -- register for print events
		if (soarPrint) {
			agent.RegisterForPrintEvent(smlPrintEventId.smlEVENT_PRINT,
					getLogger(), null, true);
		}

		// save the agent
		agents.put(name, new AgentData(agent, productionsFile));

		// spawn the debugger if we're supposed to
		if (spawnDebuggers && !isClientConnected(Names.kDebuggerClient)) {
			ClientConfig debuggerConfig = clients.get(Names.kDebuggerClient);
			debuggerConfig.command = getDebuggerCommand(name);

			spawnClient(Names.kDebuggerClient, debuggerConfig);
		}

		return agent;
	}

	/**
	 * @param client
	 *            the client in question
	 * @return true if it is connected
	 * 
	 *         check to see if the client specified by the client config is
	 *         connected or not
	 */
	@Override
	public boolean isClientConnected(String clientId) {
		boolean connected = false;
		kernel.GetAllConnectionInfo();
		for (int i = 0; i < kernel.GetNumberConnections(); ++i) {
			ConnectionInfo info = kernel.GetConnectionInfo(i);
			if (info.GetName().equalsIgnoreCase(clientId)) {
				connected = true;
				break;
			}
		}
		return connected;
	}

	/**
	 * @param agentName
	 *            tailor the command to this agent name
	 * @return a string command line to execute to spawn the debugger
	 */
	public String getDebuggerCommand(String agentName) {
		// Figure out whether to use java or javaw
		String os = System.getProperty("os.name");
		String commandLine;
		if (os.matches(".+indows.*") || os.matches("INDOWS")) {
			commandLine = "javaw -jar \""
					+ SimConfig.getHome()
					+ File.separator
					+ "..\\..\\SoarLibrary\\bin\\SoarJavaDebugger.jar\" -cascade -remote -agent "
					+ agentName + " -port " + port;
		} else {
			commandLine = System.getProperty("java.home")
					+ "/bin/java -jar "
					+ SimConfig.getHome()
					+ File.separator
					+ "../../SoarLibrary/bin/SoarJavaDebugger.jar -XstartOnFirstThread -cascade -remote -agent "
					+ agentName + " -port " + port;
		}

		return commandLine;
	}

	@Override
	public void reload(String player) {
		AgentData agentData = agents.get(player);
		if (agentData == null) {
			logger.warn("Didn't find player to reload: " + player);
			return;
		}

		agentData.agent
				.LoadProductions(agentData.productions.getAbsolutePath());
	}

	@Override
	public EaterCommander createEaterCommander(Eater eater, String productions,
			int vision, String[] shutdownCommands) {
		Agent agent = createSoarAgent(eater.getName(), productions, debug);
		if (agent == null) {
			return null;
		}
		SoarEater commander = new SoarEater(sim, eater, agent, vision, shutdownCommands);
		agents.get(eater.getName()).sa = commander;
		return commander;
	}

	@Override
	public TankCommander createTankCommander(Tank tank, String productions,
			String[] shutdownCommands) {
		Agent agent = createSoarAgent(tank.getName(), productions, debug);
		if (agent == null) {
			return null;
		}
		SoarTank commander = new SoarTank(sim, tank, agent, shutdownCommands);
		agents.get(tank.getName()).sa = commander;
		return commander;
	}

	@Override
	public TaxiCommander createTaxiCommander(Taxi taxi, String productions,
			String[] shutdownCommands) {
		Agent agent = createSoarAgent(taxi.getName(), productions, debug);
		if (agent == null) {
			return null;
		}
		SoarTaxi commander = new SoarTaxi(sim, taxi, agent, shutdownCommands);
		agents.get(taxi.getName()).sa = commander;
		return commander;
	}

	@Override
	public RobotCommander createRoomCommander(Robot player, RoomWorld world,
			String productions, String[] shutdownCommands) {
		Agent agent = createSoarAgent(player.getName(), productions, debug);
		if (agent == null) {
			return null;
		}
		SoarRobot commander = new SoarRobot(sim, player, agent, kernel, world, shutdownCommands);
		agents.get(player.getName()).sa = commander;
		return commander;
	}

	private static final ExecutorService exec = Executors
			.newSingleThreadExecutor();
	

	private BlockingQueue<TickStatus> tickReady = new SynchronousQueue<TickStatus>();
	private BlockingQueue<Boolean> outputReady = new SynchronousQueue<Boolean>();
	private BlockingQueue<Boolean> tickDone = new SynchronousQueue<Boolean>();
	
	@Override
	public void onEvent(SimEvent event) {
		if (event instanceof StartEvent) {
			if (agents.size() != 0) {
				firstUpdate = true;
				exec.submit(new Callable<Void>() {
					@Override
					public Void call() {
						logger.trace("soar alive, committing");

						for (AgentData ad : agents.values()) {
							ad.agent.Commit();
						}
						
						logger.trace("running all agents forever");
						logger.trace(kernel.RunAllAgentsForever());
						logger.trace("run returned");
						return null;
					}
				});
			}
		} else if (event instanceof BeforeTickEvent) {
			try {
				// wait for update
				logger.trace("before tick, waiting for update");
				tickReady.put(TickStatus.GO);
				outputReady.take();
			} catch (InterruptedException e) {
				// TODO: handle correctly
			}
			logger.trace("before tick, returning");

		} else if (event instanceof AfterTickEvent) {
			try {
				logger.trace("after tick, notifying done");
				// tell that tick is done
				tickDone.take();
				
			} catch (InterruptedException e) {
				// TODO: handle correctly
			}
			logger.trace("after tick returning");

		} else if (event instanceof StopEvent) {
			try {
				logger.trace("stop event issuing notice");
				tickReady.put(TickStatus.STOP);
			} catch (InterruptedException e) {
				// TODO: handle correctly
			}
			logger.trace("before tick, returning");
		}
	}
	
	private enum TickStatus { GO, STOP };
	
	boolean firstUpdate;
	
	@Override
	public void updateEventHandler(int eventID, Object data, Kernel kernel, int runFlags) {
		for (AgentData ad : agents.values()) {
			logger.trace("processing output for " + ad.agent.GetAgentName());
			ad.sa.processSoarOuput();
		}
		
		try {
			if (firstUpdate) {
				firstUpdate = false;
				logger.trace("waiting for tick to become ready");
				TickStatus tick = tickReady.take();
				if (tick == TickStatus.GO) {
					logger.trace("tick is ready");
				} else {
					throw new IllegalStateException("before-tick event must follow start event");
				}
			}

			// let the environment tick
			outputReady.put((runFlags & smlRunFlags.sml_DONT_UPDATE_WORLD.swigValue()) == 0);
			
			logger.trace("waiting for tick to complete");
			tickDone.put(Boolean.TRUE);
			
			for (AgentData ad : agents.values()) {
				logger.trace("updating input for " + ad.agent.GetAgentName());
				ad.sa.updateSoarInput();
				ad.agent.Commit();
			}
			
			logger.trace("waiting for tick or stop");
			TickStatus tick = tickReady.take();
			
			if (tick == TickStatus.STOP) {
				logger.trace("issuing stop");
				kernel.StopAllAgents();
			} else {
				logger.trace("tick is ready");
			}
		} catch (InterruptedException e1) {
			// TODO handle correctly
		}
		
		logger.trace("update done");
	}

	@Override
	public void setDebug(boolean setting) {
		this.debug = setting;
	}
}
