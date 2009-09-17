package edu.umich.soar.gridmap2d.core;

import java.io.File;
import java.util.ArrayList;
import java.util.List;
import java.util.Random;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.gridmap2d.config.PlayerConfig;
import edu.umich.soar.gridmap2d.config.SimConfig;
import edu.umich.soar.gridmap2d.core.events.AfterTickEvent;
import edu.umich.soar.gridmap2d.core.events.BeforeTickEvent;
import edu.umich.soar.gridmap2d.core.events.ErrorEvent;
import edu.umich.soar.gridmap2d.core.events.InfoEvent;
import edu.umich.soar.gridmap2d.core.events.PlayerAddedEvent;
import edu.umich.soar.gridmap2d.core.events.PlayerRemovedEvent;
import edu.umich.soar.gridmap2d.core.events.ResetEvent;
import edu.umich.soar.gridmap2d.core.events.StartEvent;
import edu.umich.soar.gridmap2d.core.events.StopEvent;
import edu.umich.soar.gridmap2d.events.SimEventManager;
import edu.umich.soar.gridmap2d.map.EatersWorld;
import edu.umich.soar.gridmap2d.map.GridMap;
import edu.umich.soar.gridmap2d.map.Player;
import edu.umich.soar.gridmap2d.map.RoomWorld;
import edu.umich.soar.gridmap2d.map.TankSoarWorld;
import edu.umich.soar.gridmap2d.map.TaxiWorld;
import edu.umich.soar.gridmap2d.map.World;
import edu.umich.soar.gridmap2d.soar.Soar;

/**
 * Keeps track of the meta simulation state. The world keeps track of more state
 * and is the major member of this class. Creates the soar kernel and registers
 * events.
 * 
 * @author voigtjr
 * 
 */
public class Simulation {
	private static Log logger = LogFactory.getLog(Simulation.class);

	public static Random random = new Random();
	private World world;
	private Game game;
	private CognitiveArchitecture cogArch;
	private int worldCount;
	private SimConfig config;

	public World initialize(SimConfig config) {
		this.config = config;
		this.game = config.game();

		// Make all runs non-random if asked
		// For debugging, set this to make all random calls follow the same
		// sequence
		if (config.hasSeed()) {
			// seed the generators
			logger.debug(Names.Debug.seed + config.generalConfig().seed);
			random.setSeed(config.generalConfig().seed);
		} else {
			logger.debug(Names.Debug.noSeed);
		}

		logger.trace(Names.Trace.loadingWorld);
		switch (game) {
		case TANKSOAR:
			world = new TankSoarWorld(this);
			break;
		case EATERS:
			world = new EatersWorld(this);
			break;
		case TAXI:
			world = new TaxiWorld(this);
			break;
		case ROOM:
			world = new RoomWorld(this);
			break;
		}
		
		changeMap(config.generalConfig().map);

		// TODO: reimplement
//		cogArch.doBeforeClients();
//		cogArch.doAfterClients();

		// add initial players
		logger.trace(Names.Trace.initialPlayers);
		for (PlayerConfig playerConfig : config.playerConfigs().values()) {
			createPlayer(playerConfig);
		}

		return world;
	}

	public void changeMap(String mapPath) {
		logger.debug(Names.Debug.changingMap + mapPath);
		world.setAndResetMap(mapPath);
		worldCount = 0;
		// Gridmap2D.wm.reset();
	}

	/**
	 * @param playerConfig
	 *            configuration data for the future player
	 * @throws IllegalStateException
	 *             If there are no colors available. This indicates that there
	 *             are too many players already on the map.
	 * 
	 *             create a player and add it to the simulation and world
	 */
	public void createPlayer(PlayerConfig playerConfig) {

		if ((playerConfig.color == null) || (playerConfig.color != null && playerConfig.color.isUsed())) {
			playerConfig.color = PlayerColor.useNext();
		}
		if (playerConfig.color == null) {
			throw new IllegalStateException("No colors are available");
		}

		// if we don't have a name, use our color
		if (playerConfig.name == null) {
			playerConfig.name = playerConfig.color.toString().toLowerCase();
		}

		// verify name is available
		if (world.hasPlayer(playerConfig.name)) {
			playerConfig.color.free();
			return;
		}

		Player player = world.addPlayer(playerConfig);
		if (player == null) {
			playerConfig.color.free();
			return;
		}
		
		eventManager.fireEvent(new PlayerAddedEvent(player));
	}

	/**
	 * @param player
	 *            the player to remove
	 * 
	 *            removes the player from the world and blows away any
	 *            associated data, frees up its color, etc.
	 */
	public void destroyPlayer(Player player) {
		world.removePlayer(player.getName());

		// free its color
		player.getColor().free();

		cogArch.destroyPlayer(player.getName());

		eventManager.fireEvent(new PlayerRemovedEvent(player));
	}

	/**
	 * @param player
	 *            the player to reload
	 * 
	 *            reload the player. only currently makes sense to reload a soar
	 *            agent. this re-loads the productions
	 */
	public void reloadPlayer(Player player) {
		cogArch.reload(player.getName());
	}

	public int getWorldCount() {
		return worldCount;
	}

	public void reset() {
		logger.info(Names.Info.reset);
		world.reset();
		worldCount = 0;
		eventManager.fireEvent(new ResetEvent());
	}

	public void shutdown() {
		exec.shutdown();
		try {
			exec.awaitTermination(5, TimeUnit.MINUTES);
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		List<Player> players = new ArrayList<Player>(7);
		players.addAll(world.getPlayers());
		for (Player player : players) {
			destroyPlayer(player);
		}

		cogArch.shutdown();
	}

	public boolean isDone() {
		return world.isTerminal();
	}

	public String getMapPath() {
		return SimConfig.getHome() + "config" + File.separator + "maps"
				+ File.separator + game.id();
	}
	
	public Game getGame() {
		return game;
	}

	public void interrupted(String agentName) {
		world.interrupted(agentName);
	}

	public GridMap getMap() {
		return world.getMap();
	}
	
	private static final ExecutorService exec = Executors
		.newSingleThreadExecutor();
	private AtomicBoolean running = new AtomicBoolean(false);
	private AtomicBoolean stopRequested = new AtomicBoolean(false);
	private SimEventManager eventManager = new SimEventManager();

	public void run(final int ticks) {
		
		if (!running.getAndSet(true)) {
			exec.submit(new Runnable() {
				@Override
				public void run() {
					// TODO: thread interruption
					logger.trace("firing start");
					eventManager.fireEvent(new StartEvent());
					int ticksDone = 0;
					do {
						logger.trace("firing before tick");
						eventManager.fireEvent(new BeforeTickEvent());
						tick();
						if (ticks > 0) {
							ticksDone += 1;
							if (ticksDone >= ticks) {
								logger.trace("requesting stop due to tick count");
								stopRequested.set(true);
							}
						}

						logger.trace("firing after tick");
						eventManager.fireEvent(new AfterTickEvent());
						
					} while(!stopRequested.getAndSet(false));
					logger.trace("firing stop");
					running.set(false);
					eventManager.fireEvent(new StopEvent());
				}
			});
		} else {
			logger.trace("run called while running");
		}
	}
	
	public void run() {
		run(0);
	}
	
	public CognitiveArchitecture getCogArch() {
		if (cogArch == null) {
			initCogArch();
		}
		return cogArch;
	}
	
	private void initCogArch() {
		if (cogArch == null) {
			cogArch = new Soar(this);
			if (config.hasSeed()) {
				cogArch.seed(config.generalConfig().seed);
			}
		}
	}
	
	private void tick() {
		worldCount += 1;
		logger.trace("tick " + worldCount);
		world.update(worldCount);
	}

	public boolean isRunning() {
		return running.get();
	}
	
	public void stop() {
		stopRequested.compareAndSet(false, true);
	}
	
	public SimEventManager getEvents() {
		return eventManager;
	}

	public SimConfig getConfig() {
		return config;
	}

	public void error(String title, String message) {
		logger.error(title + ": " + message);
		eventManager.fireEvent(new ErrorEvent(title, message));
	}

	public void error(String message) {
		logger.error(message);
		eventManager.fireEvent(new ErrorEvent(message));
	}

	public boolean isShuttingDown() {
		return false;
	}

	public void info(String message) {
		logger.info(message);
		eventManager.fireEvent(new InfoEvent(message));
	}
	
	public void info(String title, String message) {
		logger.info(title + ": " + message);
		eventManager.fireEvent(new InfoEvent(title, message));
	}

	public double getTimeSlice() {
		return 0.005;
	}

	public World getWorld() {
		return world;
	}
}
