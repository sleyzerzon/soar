package edu.umich.soar.gridmap2d.core;

import java.io.File;
import java.util.Random;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.SynchronousQueue;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.prefs.Preferences;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.gridmap2d.Application;
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
import edu.umich.soar.gridmap2d.map.Robot;
import edu.umich.soar.gridmap2d.map.RoomMap;
import edu.umich.soar.gridmap2d.map.RoomWorld;
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
	private RoomWorld world;
	private CognitiveArchitecture cogArch;
	private int worldCount;
	private SimConfig config;
	private Preferences pref;
	private final String KEY_TICK_MSEC = "tickMsec";
	
	public RoomWorld initialize(SimConfig config) {
		this.config = config;

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
		world = new RoomWorld(this);
		
		String map = config.generalConfig().map;
		if (map == null) {
			map = "config/maps/default.txt";
		}
		changeMap(map);
		
		this.pref = Application.PREFERENCES.node("simulation");
		tickMsec = pref.getInt(KEY_TICK_MSEC, 50);

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

		Robot player = world.addPlayer(playerConfig);
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
	public void destroyPlayer(Robot player) {
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
	public void reloadPlayer(Robot player) {
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
		pref.putInt(KEY_TICK_MSEC, tickMsec);
		
		exec.shutdown();
		try {
			// The delay is high here so that headless runs can complete.
			exec.awaitTermination(5, TimeUnit.MINUTES);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		
		schexec.shutdown();
		try {
			// The delay is high here so that headless runs can complete.
			schexec.awaitTermination(5, TimeUnit.MINUTES);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}

		for (Robot player : world.getPlayers()) {
			destroyPlayer(player);
		}

		if (cogArch != null) {
			cogArch.shutdown();
		}
		
		// FIXME BADBAD: hack: stepping in the debugger spawns a mysterious non-daemon
		// thread that won't shut down. This allows everything else to shut down
		// before killing itself off.
		try {
			Thread.sleep(2000);
		} catch (InterruptedException ignored) {
		}
		System.exit(0);
	}

	public boolean isDone() {
		return world.isTerminal();
	}

	public String getMapPath() {
		return SimConfig.getHome() + "config" + File.separator + "maps"
				+ File.separator + "room";
	}
	
	public RoomMap getMap() {
		return world.getMap();
	}
	
	private static final ExecutorService exec = Executors
		.newSingleThreadExecutor();
	private static final ScheduledExecutorService schexec = Executors
		.newSingleThreadScheduledExecutor();
	private AtomicBoolean running = new AtomicBoolean(false);
	private AtomicBoolean stopRequested = new AtomicBoolean(false);
	private SimEventManager eventManager = new SimEventManager();
	private BlockingQueue<Boolean> canceller = new SynchronousQueue<Boolean>();
	
	public void run() {
		run(0,tickMsec,TimeUnit.MILLISECONDS);
	}
	
	public void step(int quantity) {
		run(quantity,0,null);
	}
	
	private void run(final int ticks, final long period, final TimeUnit unit) {
		if (!running.getAndSet(true)) {
			stopRequested.set(false);
			exec.submit(new Runnable() {
				@Override
				public void run() {
					logger.trace("firing StartEvent");
					eventManager.fireEvent(new StartEvent());
					
					final int initialWorldCount = worldCount;
					
					if (period <= 0) {
						do {
							tick();
							if (ticks > 0) {
								if (worldCount - initialWorldCount >= ticks) {
									logger.trace("requesting stop due to tick count");
									stopRequested.set(true);
								}
							}
						} while(!stopRequested.get());
						logger.trace("ending tick loop due to stop request");
						
					} else {
						final ScheduledFuture<?> ticker = schexec.scheduleWithFixedDelay(new Runnable() {
							
							@Override
							public void run() {
								if (stopRequested.get()) {
									canceller.offer(Boolean.TRUE);
									return;
								}
								tick();
								if (ticks > 0) {
									if (worldCount - initialWorldCount >= ticks) {
										logger.trace("requesting stop due to tick count");
										canceller.offer(Boolean.TRUE);
									}
								}
							}
						}, period, period, unit);
						
						try {
							canceller.take();
						} catch (InterruptedException e) {
						} finally {
							ticker.cancel(false);
						}
					}
					
					running.set(false);
					
					logger.trace("firing StopEvent");
					eventManager.fireEvent(new StopEvent());
				}
			});
		} else {
			logger.trace("run called while running");
		}
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
		logger.trace("firing BeforeTickEvent");
		eventManager.fireEvent(new BeforeTickEvent());
		
		worldCount += 1;
		
		logger.trace("tick " + worldCount);
		world.update(worldCount);

		logger.trace("firing AfterTickEvent");
		eventManager.fireEvent(new AfterTickEvent());
	}

	public boolean isRunning() {
		return running.get();
	}
	
	public void stop() {
		logger.trace("stop received");
		stopRequested.compareAndSet(false, true);
	}
	
	public SimEventManager getEvents() {
		return eventManager;
	}

	public SimConfig getConfig() {
		return config;
	}

	public void addInitialPlayers() {
		// add initial players
		for (PlayerConfig playerConfig : config.playerConfigs().values()) {
			createPlayer(playerConfig);
		}
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

	int tickMsec;
	public int getTickMSec() {
		return tickMsec;
	}

	public void setTickMSec(int setting) {
		tickMsec = setting;
	}

	public RoomWorld getWorld() {
		return world;
	}
}
