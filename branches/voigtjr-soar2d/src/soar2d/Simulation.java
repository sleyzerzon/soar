package soar2d;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Random;
import java.util.Map.Entry;

import org.apache.log4j.Logger;

import sml.Agent;
import sml.ConnectionInfo;
import sml.Kernel;
import sml.smlPrintEventId;
import sml.smlRunStepSize;
import sml.smlSystemEventId;
import sml.smlUpdateEventId;
import sml.sml_Names;
import soar2d.config.ClientConfig;
import soar2d.config.PlayerConfig;
import soar2d.config.SimConfig;
import soar2d.players.CommandInfo;
import soar2d.players.Player;
import soar2d.world.BookWorld;
import soar2d.world.EatersWorld;
import soar2d.world.World;
import soar2d.world.KitchenWorld;
import soar2d.world.TankSoarWorld;
import soar2d.world.TaxiWorld;

/**
 * @author voigtjr
 *
 * Keeps track of the meta simulation state. The world keeps track of more state and
 * is the major member of this class. Creates the soar kernel and registers events.
 */
public class Simulation {
	private static Logger logger = Logger.getLogger(Simulation.class);

	/**
	 * True if we want to use the run-til-output feature
	 */
	boolean runTilOutput = false;
	/**
	 * The soar kernel
	 */
	Kernel kernel = null;
	/**
	 * The random number generator used throughout the program
	 */
	public static Random random = null;
	
	/**
	 * The world and everything associated with it
	 */
	private World world;
	
	/**
	 * Legal colors (see PlayerConfig)
	 */
	public final String kColors[] = { "red", "blue", "yellow", "purple", "orange", "green", "black",  };
	/**
	 * A list of colors not currently taken by a player
	 */
	private ArrayList<String> unusedColors = new ArrayList<String>(kColors.length);
	/**
	 * String agent name to agent mapping
	 */
	private HashMap<String, Agent> agents = new HashMap<String, Agent>();

	private Game game;
	
	World initialize(SimConfig config) throws Exception {
		this.game = config.game();
		
		// keep track of colors
		for (String color : kColors) {
			unusedColors.add(color);
		}
		
		runTilOutput = config.runTilOutput();
		
		// Initialize Soar
		if (config.soarConfig().remote != null) {
			kernel = Kernel.CreateRemoteConnection(true, config.soarConfig().remote, config.soarConfig().port);
		} else {
			// Create kernel
			kernel = Kernel.CreateKernelInNewThread("SoarKernelSML", config.soarConfig().port);
			//kernel = Kernel.CreateKernelInCurrentThread("SoarKernelSML", true);
		}

		if (kernel.HadError()) {
			throw new Exception(Names.Errors.kernelCreation + kernel.GetLastErrorDescription());
		}
		
		// We want the most performance
		logger.debug(Names.Debug.autoCommit);
		kernel.SetAutoCommit(false);

		// Make all runs non-random if asked
		// For debugging, set this to make all random calls follow the same sequence
		if (config.hasSeed()) {
			// seed the generators
			int seed = config.generalConfig().seed;
			logger.debug(Names.Debug.seed + seed);
			kernel.ExecuteCommandLine("srand " + seed, null) ;
			random = new Random(seed);
		} else {
			logger.debug(Names.Debug.noSeed);
			random = new Random();
		}
		
		// Register for events
		logger.trace(Names.Trace.eventRegistration);
		kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_START, Soar2D.control, null);
		kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_STOP, Soar2D.control, null);
		if (runTilOutput) {
			logger.debug(Names.Debug.runTilOutput);
			kernel.RegisterForUpdateEvent(smlUpdateEventId.smlEVENT_AFTER_ALL_GENERATED_OUTPUT, Soar2D.control, null);
		} else {
			logger.debug(Names.Debug.noRunTilOutput);
			kernel.RegisterForUpdateEvent(smlUpdateEventId.smlEVENT_AFTER_ALL_OUTPUT_PHASES, Soar2D.control, null);
		}
		
		// Load the world
		logger.trace(Names.Trace.loadingWorld);
		switch (game) {
		case TANKSOAR:
			world = new TankSoarWorld(config.generalConfig().map);
			break;
		case EATERS:
			world = new EatersWorld(config.generalConfig().map);
			break;
		case KITCHEN:
			world = new KitchenWorld(config.generalConfig().map);
			break;
		case TAXI:
			world = new TaxiWorld(config.generalConfig().map);
			break;
		case ROOM:
			world = new BookWorld(config.generalConfig().map);
			break;
		}
		Soar2D.control.setRunsTerminal(config.generalConfig().runs);
		Soar2D.control.resetTime();

		// Start or wait for clients (false: before agent creation)
		logger.trace(Names.Trace.beforeClients);
		doClients(false);
		
		// add initial players
		logger.trace(Names.Trace.initialPlayers);
		for ( Entry<String, PlayerConfig> entry : config.playerConfigs().entrySet()) {
			createPlayer(entry.getKey(), entry.getValue());
		}
		
		// Start or wait for clients (true: after agent creation)
		logger.trace(Names.Trace.afterClients);
		doClients(true);
		return world;
	}
	
//	private void error(String message) {
//		logger.fatal(message);
//		Soar2D.control.errorPopUp(message);
//	}
	
	public ArrayList<String> getUnusedColors() {
		return unusedColors;
	}
	
	/**
	 * @param color the color to use, or null if any will work
	 * @return null if the color is not available for whatever reason
	 * 
	 * removes a color from the unused colors list (by random if necessary)
	 * a return of null indicates failure, the color is taken or no more
	 * are available
	 */
	public String useAColor(String color) {
		if (unusedColors.size() < 1) {
			return null;
		}
		if (color == null) {
			int pick = random.nextInt(unusedColors.size());
			color = unusedColors.get(pick);
			unusedColors.remove(pick);
			return color;
		}
		Iterator<String> iter = unusedColors.iterator();
		while (iter.hasNext()) {
			if (color.equalsIgnoreCase(iter.next())) {
				iter.remove();
				return color;
			}
		}
		return null;
	}
	
	/**
	 * @param color the color to free up, must be not null
	 * @return false if the color wasn't freed up
	 * 
	 * The opposite of useAColor
	 * a color wouldn't be freed up if it wasn't being used in the first place
	 * or if it wasn't legal
	 */
	public boolean freeAColor(String color) {
		assert color != null;
		boolean legal = false;
		for (String knownColor : kColors) {
			if (color.equals(knownColor)) {
				legal = true;
			}
		}
		if (!legal) {
			return false;
		}
		if (unusedColors.contains(color)) {
			return false;
		}
		unusedColors.add(color);
		return true;
	}

	/**
	 * @param playerConfig configuration data for the future player
	 * 
	 * create a player and add it to the simulation and world
	 */
	public void createPlayer(String playerId, PlayerConfig playerConfig) throws Exception {
		if (Game.TAXI == game && (world.numberOfPlayers() > 1)) {
			// if this is removed, revisit white color below!
			throw new Exception(Names.Errors.taxi1Player);
		}
		
		// if a color was specified
		if (playerConfig.color != null) {
			//make sure it is unused
			if (!unusedColors.contains(playerConfig.color)) {
				throw new Exception(Names.Errors.usedColor + playerConfig.color);
			}
			// it is unused, so use it
			useAColor(playerConfig.color);
		} else {
			
			// no color specified, pick on at random
			String color = useAColor(null);
			
			// make sure we got one
			if (color == null) {
				
				// if we didn't then they are all gone
				throw new Exception(Names.Errors.noMoreSlots);
			}
			playerConfig.color = color;
		}
		
		// if we don't have a name, use our color
		if (playerConfig.name == null) {
			playerConfig.name = playerConfig.color;
		}
		
		try {
			world.addPlayer(playerId, playerConfig);
		} catch (Exception e) {
			freeAColor(playerConfig.color);
			throw e;
		}
		
		// the agent list has changed, notify things that care
		Soar2D.control.playerEvent();
	}
	
	public Agent createSoarAgent(String name, String productions) throws Exception {
		Agent agent = kernel.CreateAgent(name);
		if (agent == null) {
			throw new Exception("Agent " + name + " creation failed: " + kernel.GetLastErrorDescription());
		}
		
		// now load the productions
		File productionsFile = new File(productions);
		if (!agent.LoadProductions(productionsFile.getAbsolutePath())) {
			throw new Exception("Agent " + name + " production load failed: " + agent.GetLastErrorDescription());
		}
		
		// if requested, set max memory usage
		int maxmem = Soar2D.config.soarConfig().max_memory_usage;
		if (maxmem > 0) {
			agent.ExecuteCommandLine("max-memory-usage " + Integer.toString(maxmem));
		}
		
		// Scott Lathrop --  register for print events
		if (Soar2D.config.soarConfig().soar_print) {
			agent.RegisterForPrintEvent(smlPrintEventId.smlEVENT_PRINT, Soar2D.control.getLogger(), null,true);
		}
		
		// save the agent
		agents.put(name, agent);
		
		// spawn the debugger if we're supposed to
		if (Soar2D.config.soarConfig().spawn_debuggers && !isClientConnected(Names.kDebuggerClient)) {
			ClientConfig debuggerConfig = Soar2D.config.clientConfigs().get(Names.kDebuggerClient);
			debuggerConfig.command = getDebuggerCommand(name);

			spawnClient(Names.kDebuggerClient, debuggerConfig);
		}
		
		return agent;
	}

	/**
	 * @param client the client in question
	 * @return true if it is connected
	 * 
	 * check to see if the client specified by the client config is connected or not
	 */
	public boolean isClientConnected(String clientId) {
		boolean connected = false;
		kernel.GetAllConnectionInfo();
		for (int i = 0; i < kernel.GetNumberConnections(); ++i) {
			ConnectionInfo info =  kernel.GetConnectionInfo(i);
			if (info.GetName().equalsIgnoreCase(clientId)) {
				connected = true;
				break;
			}
		}
		return connected;
	}
	
	/**
	 * @param agentName tailor the command to this agent name
	 * @return a string command line to execute to spawn the debugger
	 */
	public String getDebuggerCommand(String agentName) {
		// Figure out whether to use java or javaw
		String os = System.getProperty("os.name");
		String commandLine;
		if (os.matches(".+indows.*") || os.matches("INDOWS")) {
			commandLine = "javaw -jar \"" + getBasePath() 
			+ "..\\..\\SoarLibrary\\bin\\SoarJavaDebugger.jar\" -cascade -remote -agent " 
			+ agentName + " -port " + Soar2D.config.soarConfig().port;
		} else {
			commandLine = System.getProperty("java.home") + "/bin/java -jar " + getBasePath()
			+ "../../SoarLibrary/bin/SoarJavaDebugger.jar -XstartOnFirstThread -cascade -remote -agent " 
			+ agentName + " -port " + Soar2D.config.soarConfig().port;
		}
		
		return commandLine;
	}

	/**
	 * @param player the player to remove
	 * 
	 * removes the player from the world and blows away any associated data, 
	 * frees up its color, etc.
	 */
	public void destroyPlayer(Player player) throws Exception {
		world.removePlayer(player.getName());
		
		// free its color
		freeAColor(player.getColor());
		
		// get the agent (human agents return null here)
		Agent agent = agents.remove(player.getName());
		if (agent != null) {
			// there was an agent, destroy it
			kernel.DestroyAgent(agent);
			agent.delete();
			agent = null;
		}
		
		// the player list has changed, notify those who care
		Soar2D.control.playerEvent();
	}
	
	/**
	 * @param player the player to reload
	 * 
	 * reload the player. only currently makes sense to reload a soar agent.
	 * this re-loads the productions
	 */
	public void reloadPlayer(Player player) {
		Agent agent = agents.get(player.getName());
		if (agent == null) {
			return;
		}
		
		PlayerConfig playerConfig = Soar2D.config.playerConfigs().get(player.getID());
		assert playerConfig != null;
		assert playerConfig.productions != null;
		File productionsFile = new File(playerConfig.productions);
		agent.LoadProductions(productionsFile.getAbsolutePath());
	}
	
	/**
	 * @param after do the clients denoted as "after" agent creation
	 * @return true if the clients all connected.
	 */
	private void doClients(boolean after) throws Exception {
		for ( Entry<String, ClientConfig> entry : Soar2D.config.clientConfigs().entrySet()) {
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
					throw new Exception(Names.Errors.clientSpawn + entry.getKey());
				}
			}
		}
	}

	/**
	 * @author voigtjr
	 *
	 * This handles some nitty gritty client spawn stuff
	 */
	private class Redirector extends Thread {
		BufferedReader br;
		public Redirector(BufferedReader br) {
			this.br = br;
		}
		
		public void run() {
			String line;
			try {
				while ((line = br.readLine()) != null) {
					System.out.println(line);
				}
			} catch (IOException e) {
				System.err.println(e.getMessage());
			}
		}
	}
	
	/**
	 * @param client the client to spawn
	 * 
	 * spawns a client, waits for it to connect
	 */
	public void spawnClient(String clientID, ClientConfig clientConfig) throws Exception {
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
				throw new Exception(Names.Errors.clientSpawn + clientID);
			}
			
		} catch (IOException e) {
			throw new Exception(Names.Errors.clientSpawn + clientID + ": " + e.getMessage());
		}
	}
	
	/**
	 * @param client the client to wait for
	 * @return true if the client connected within the timeout
	 * 
	 * waits for a client to report ready
	 */
	public boolean waitForClient(String clientID, ClientConfig clientConfig) {
		boolean ready = false;
		// do this loop if timeout seconds is 0 (code for wait indefinitely) or if we have tries left
		for (int tries = 0; (clientConfig.timeout == 0) || (tries < clientConfig.timeout); ++tries) {
			kernel.GetAllConnectionInfo();
			if (kernel.HasConnectionInfoChanged()) {
				for (int i = 0; i < kernel.GetNumberConnections(); ++i) {
					ConnectionInfo info =  kernel.GetConnectionInfo(i);
					if (info.GetName().equalsIgnoreCase(clientID)) {
						if (info.GetAgentStatus().equalsIgnoreCase(sml_Names.getKStatusReady())) {
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
			} catch (InterruptedException ignored) {}
		}
		return ready;
	}
	
	/**
	 * update the sim, or, in this case, the world
	 */
	public void update() throws Exception {
		world.update();
	}

	/**
	 * run soar forever
	 */
	public void runForever() {
		if (runTilOutput) {
			kernel.RunAllAgentsForever(smlRunStepSize.sml_UNTIL_OUTPUT);
		} else {
			kernel.RunAllAgentsForever();
		}
		
	}

	/**
	 * run soar one step
	 */
	public void runStep() {
		if (runTilOutput) {
			kernel.RunAllTilOutput(smlRunStepSize.sml_UNTIL_OUTPUT);
		} else {
			kernel.RunAllAgents(1);
		}
	}

	/**
	 * @return true if the map reset was successful
	 * 
	 * resets the world, ready for a new run
	 */
	public void reset() throws Exception {
		logger.info(Names.Info.reset);
		world.reset();
		Soar2D.control.resetTime();
	}

	/**
	 * shuts things down, including the kernel, in preparation for an exit to dos
	 */
	public void shutdown() throws Exception {
		for(Entry<String, Agent> entry : agents.entrySet()) {
			if (world != null) {
				world.removePlayer(entry.getKey());
			}
			if (kernel != null) {
				Agent agent = entry.getValue();
				// human agents return null (I think)
				if (agent != null) {
					// there was an agent, destroy it
					kernel.DestroyAgent(agent);
					agent.delete();
					agent = null;
				}
			}
		}
		agents.clear();
		
		if (kernel != null) {
			logger.trace(Names.Trace.kernelShutdown);
			kernel.Shutdown();
			kernel.delete();
		}
	}
	
	/**
	 * @return true if there are human agents present
	 */
	public boolean hasHumanAgents() {
		return agents.size() < world.numberOfPlayers();
	}
	
	/**
	 * @return true if there are soar agents present
	 */
	public boolean hasSoarAgents() {
		return agents.size() > 0;
	}
	
	public boolean isDone() {
		return world.isTerminal();
	}

	public String getBasePath() {
		return System.getProperty("user.dir") + System.getProperty("file.separator");
	}
	public String getMapPath() {
		return Soar2D.simulation.getBasePath() + "maps" + System.getProperty("file.separator");
	}
	public String getMapExt() {
		switch (Soar2D.config.game()) {
		case TANKSOAR:
			return "tmap";
		case EATERS:
			return "emap";
		case ROOM:
			return "bmap";
		case KITCHEN:
		case TAXI:
			return "xml";
		}
		return null;
	}
	public String getAgentPath() {
		return Soar2D.simulation.getBasePath() + "agents" + System.getProperty("file.separator");
	}
	
	public void interrupted(String agentName) throws Exception {
		if (Soar2D.wm.using()) {
			return;
		}

		world.interrupted(agentName);
	}
	
	public CommandInfo getHumanCommand(Player player) {
		return Soar2D.wm.getHumanCommand(player);
	}
}
