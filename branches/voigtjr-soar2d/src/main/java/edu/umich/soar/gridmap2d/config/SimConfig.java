package edu.umich.soar.gridmap2d.config;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.Field;
import java.net.URISyntaxException;
import java.util.HashMap;
import java.util.Map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.config.Config;
import edu.umich.soar.config.ConfigFile;
import edu.umich.soar.config.ParseError;
import edu.umich.soar.gridmap2d.Application;
import edu.umich.soar.gridmap2d.core.Game;
import edu.umich.soar.gridmap2d.core.Names;
import edu.umich.soar.gridmap2d.core.PlayerColor;


public class SimConfig implements GameConfig {	
	private static final Log logger = LogFactory.getLog(SimConfig.class);
	
	private static final String EATERS_CNF = "eaters.cnf";
	private static final String EATERS_CONSOLE_CNF = "eaters-console.cnf";
	private static final String TANKSOAR_CNF = "tanksoar.cnf";
	private static final String TANKSOAR_CONSOLE_CNF = "tanksoar-console.cnf";
	private static final String ROOM_CNF = "room.cnf";
	private static final String TAXI_CNF = "taxi.cnf";

	private static final File home;
	
	static {
		home = figureOutHome();
		
		try {
			boolean installed = false; 
			installed |= install(TANKSOAR_CNF);
			installed |= install(TANKSOAR_CONSOLE_CNF);
			installed |= install(EATERS_CNF);
			installed |= install(EATERS_CONSOLE_CNF);
			installed |= install(ROOM_CNF);
			installed |= install(TAXI_CNF);
			if (installed) {
				System.err.println("Installed at least one config file." +
						"\nYou may need to refresh the project if you are working inside of Eclipse.");
			}
		} catch (IOException e) {
			e.printStackTrace();
			logger.error("Unable to install config file(s): " + e.getMessage());
		}
	}
	
	private static File figureOutHome() {
		File home = null;
		String homeProperty = System.getProperty("soar2d.home");
		if (homeProperty != null) {
			home = new File(homeProperty);
			return home;
		}

		try {
			home = new File(SimConfig.class.getProtectionDomain()
					.getCodeSource().getLocation().toURI());
		} catch (URISyntaxException e) {
			e.printStackTrace();
			throw new IllegalStateException("Internal error: getCodeSource returned bad URL");
		}

		// should point to parent of jar file
		if (!home.isDirectory()) {
			home = home.getParentFile();
		}

		// usually, we'll be in SoarLibrary/lib or SoarLibrary/bin
		String soarlib = "SoarLibrary";
		String soarjava = "soar-java";
		String soar2d = "soar-soar2d";
		String hstr = home.toString();
		int pos = hstr.lastIndexOf(soarlib);
		if (pos >= 0) {
			home = new File(home.toString().substring(0, pos) + soarjava
					+ File.separator + soar2d);
		} else {
			// sometimes in soar-java somewhere
			pos = hstr.lastIndexOf(soarjava);
			if (pos >= 0) {
				home = new File(home.toString().substring(0,
						pos + soarjava.length())
						+ File.separator + soar2d);
			} else {
				// maybe in SoarSuite root?
				home = new File(home.toString() + File.separator + soarjava
						+ File.separator + soar2d);
			}
		}

		// verify exists
		if (!home.exists()) {
			throw new IllegalStateException("Can't figure out where the " + soar2d + " folder is.");
		}
		return home;
	}
	
	public static File getHome() {
		return home;
	}

	private static boolean install(String file) throws IOException {
		File cnf = new File(file);
		File cnfDest = new File(home + File.separator + "config"
				+ File.separator + file);

		if (cnfDest.exists()) {
			return false;
		}

		// Get the DLL from inside the JAR file
		// It should be placed at the root of the JAR (not in a subfolder)
		String jarpath = "/" + cnf.getPath();
		InputStream is = SimConfig.class.getResourceAsStream(jarpath);

		if (is == null) {
			System.err.println("Failed to find " + jarpath
					+ " in the JAR file");
			return false;
		}

		// Create the new file on disk
		FileOutputStream os = new FileOutputStream(cnfDest);

		// Copy the file onto disk
		byte bytes[] = new byte[2048];
		int read;
		while (true) {
			read = is.read(bytes);

			// EOF
			if (read == -1)
				break;

			os.write(bytes, 0, read);
		}

		is.close();
		os.close();
		return true;
	}

	/**
	 * @param path
	 * @return
	 * 
	 * @throws IllegalStateException If there is an error loading the config file
	 * @throws FileNotFoundException If the specified path doesn't point to a valid file
	 */
	public static SimConfig getInstance(String path) throws FileNotFoundException {
		// See if it exists:
		File configFile = new File(path);
		if (!configFile.exists()) {
			configFile = new File(home + File.separator + path);
			if (!configFile.exists()) {
				configFile = new File(home + File.separator + "config"
						+ File.separator + path);
				if (!configFile.exists()) {
					throw new FileNotFoundException(path);
				}
			}
		}

		if (!configFile.isFile()) {
			throw new FileNotFoundException(path);
		}

		// Read config file
		SimConfig config = null;
		try {
			config = new SimConfig(new Config(new ConfigFile(path)));
		} catch (IOException e) {
			throw new IllegalStateException("Error loading " + path, e);
		} catch (ParseError e) {
			throw new IllegalStateException("Error loading " + path, e);
		} catch (IllegalArgumentException e) {
			throw new IllegalStateException("Error loading " + path, e);
		}

		return config;
	}
	
	private static class Keys {
		private static final String last_productions = "last_productions";
		private static final String window_position_x = "window_position.x";
		private static final String window_position_y = "window_position.y";
		private static final String general = "general";
		private static final String soar = "soar";
		private static final String terminals = "terminals";
		private static final String players = "players";
		private static final String active_players = players + ".active_players";
		private static final String clients = "clients";
		private static final String active_clients = clients + ".active_clients";
		private static final String points = "points"; // keep in synch with PlayerConfig
	}
	
	private Game game;

	private Config config;
	
	private GeneralConfig generalConfig;
	private SoarConfig soarConfig;
	private TerminalsConfig terminalsConfig;
	
	private GameConfig gameConfig;
	
	private Map<String, PlayerConfig> playerConfigs = new HashMap<String, PlayerConfig>();
	private Map<String, ClientConfig> clientConfigs = new HashMap<String, ClientConfig>();;
	
	/**
	 * @param config
	 * 
	 * @throws IllegalArgumentException If an unknown game type is passed.
	 */
	private SimConfig(Config config) {
		this.config = config;
		
		// verify we have a map
		config.requireString("general.map");
		
		generalConfig = new GeneralConfig();
		loadSubConfig(config.getChild(Keys.general), GeneralConfig.class.getFields(), generalConfig);

		soarConfig = new SoarConfig();
		loadSubConfig(config.getChild(Keys.soar), SoarConfig.class.getFields(), soarConfig);

		terminalsConfig = new TerminalsConfig();
		loadSubConfig(config.getChild(Keys.terminals), TerminalsConfig.class.getFields(), terminalsConfig);

		try {
			game = Game.valueOf(generalConfig.game.toUpperCase());
		} catch (IllegalArgumentException e) {
			e.printStackTrace();
			StringBuilder sb = new StringBuilder();
			sb.append("Unknown game type: ");
			sb.append(generalConfig.game);
			sb.append("\nKnown game types:");
			for (Game gameType : Game.values()) {
				sb.append(" ");
				sb.append(gameType.id());
			}
			throw new IllegalArgumentException(sb.toString(), e);
		}
		
		Config childConfig = config.getChild(game.id());
		switch (game) {
		case TANKSOAR:
			gameConfig = new TankSoarConfig();
			loadSubConfig(childConfig, TankSoarConfig.class.getFields(), gameConfig);
			break;
		case EATERS:
			gameConfig = new EatersConfig();
			loadSubConfig(childConfig, EatersConfig.class.getFields(), gameConfig);
			break;
		case TAXI:
			gameConfig = new TaxiConfig();
			loadSubConfig(childConfig, TaxiConfig.class.getFields(), gameConfig);
			break;
		case ROOM:
			gameConfig = new RoomConfig();
			loadSubConfig(childConfig, RoomConfig.class.getFields(), gameConfig);
			break;
		}

		if (config.hasKey(Keys.active_players)) {
			for (String playerID : config.getStrings(Keys.active_players)) {
				PlayerConfig playerConfig = new PlayerConfig();
				loadSubConfig(config.getChild(Keys.players + "." + playerID), PlayerConfig.class.getFields(), playerConfig);
				
				// process special has points key
				playerConfig.hasPoints = config.hasKey(Keys.players + "." + playerID + "." + Keys.points);
				
				playerConfigs.put(playerID, playerConfig);
			}
		}

		if (config.hasKey(Keys.active_clients)) {
			for(String clientName : config.getStrings(Keys.active_clients)) {
				ClientConfig clientConfig = new ClientConfig();
				loadSubConfig(config.getChild(Keys.clients + "." + clientName), ClientConfig.class.getFields(), clientConfig);
				clientConfigs.put(clientName, clientConfig);
				
			}
		}
		
		// Add default debugger client to configuration, overwriting any existing java-debugger config:
		ClientConfig clientConfig = new ClientConfig();
		clientConfig.timeout = 15;
		clientConfigs.put(Names.kDebuggerClient, clientConfig);
	}
	
	private void loadSubConfig(Config childConfig, Field [] fields, Object target) {
		// use reflection to load fields
		try {
			for (Field f : fields) {
				if (f.getType().getName() == "boolean") {
					f.set(target, childConfig.getBoolean(f.getName(), f.getBoolean(target)));
					
				} else if (f.getType().getName() == "double") {
					f.set(target, childConfig.getDouble(f.getName(), f.getDouble(target)));
					
				} else if (f.getType().getName() == "int") {
					f.set(target, childConfig.getInt(f.getName(), f.getInt(target)));
					
				} else if (f.getType().getName() == "java.lang.String") {
					f.set(target, childConfig.getString(f.getName(), (String)f.get(target)));
					
				} else if (f.getType().getName() == "edu.umich.soar.gridmap2d.core.PlayerColor") {
					String colorString = childConfig.getString(f.getName(), null);
					if (colorString != null) {
						f.set(target, PlayerColor.valueOf(colorString.toUpperCase()));
					}
					
				} else 	if (f.getType().getName() == "[Z") {
					f.set(target, childConfig.getBooleans(f.getName(), (boolean [])f.get(target)));
					
				} else if (f.getType().getName() == "[D") {
					f.set(target, childConfig.getDoubles(f.getName(), (double [])f.get(target)));
					
				} else if (f.getType().getName() == "[I") {
					f.set(target, childConfig.getInts(f.getName(), (int [])f.get(target)));
					
				} else if (f.getType().getName() == "[Ljava.lang.String;") {
					f.set(target, childConfig.getStrings(f.getName(), (String [])f.get(target)));
				} else {
					System.out.println("Unsupported type encountered: " + f.getType().getName());
				}
			}
		} catch (IllegalAccessException e) {
			e.printStackTrace();
			// This shouldn't happen as long as all fields are public.
			assert false;
		}
	}
	
	public boolean hasSeed() {
		return config.hasKey("general.seed");
	}
	
	public Game game() {
		return game;
	}
	
	public GeneralConfig generalConfig() {
		return generalConfig;
	}
	
	public SoarConfig soarConfig() {
		return soarConfig;
	}
	
	public TerminalsConfig terminalsConfig() {
		return terminalsConfig;
	}
	
	public EatersConfig eatersConfig() {
		return (EatersConfig)gameConfig;
	}
	
	public TankSoarConfig tanksoarConfig() {
		return (TankSoarConfig)gameConfig;
	}
	
	public TaxiConfig taxiConfig() {
		return (TaxiConfig)gameConfig;
	}
	
	public RoomConfig roomConfig() {
		return (RoomConfig)gameConfig;
	}
	
	public Map<String, PlayerConfig> playerConfigs() {
		return playerConfigs;
	}
	
	public Map<String, ClientConfig> clientConfigs() {
		return clientConfigs;
	}
	
	public void saveLastProductions(String productionsPath) {
		String game_specific_key = game.id() + "." + Keys.last_productions;
		Application.PREFERENCES.put(game_specific_key, productionsPath);
	}
	
	public String getLastProductions() {
		String game_specific_key = game.id() + "." + Keys.last_productions;
		return Application.PREFERENCES.get(game_specific_key, null);
	}
	
	public void saveWindowPosition(int [] xy) {
		Application.PREFERENCES.putInt(Keys.window_position_x, xy[0]);
		Application.PREFERENCES.putInt(Keys.window_position_y, xy[1]);
	}
	
	public int [] getWindowPosition() {
		int [] xy = new int[] { 0, 0 };
		xy[0] = Application.PREFERENCES.getInt(Keys.window_position_x, xy[0]);
		xy[1] = Application.PREFERENCES.getInt(Keys.window_position_y, xy[1]);
		return xy;
	}
	
	@Override
	public String title() {
		return gameConfig.title();
	}
}