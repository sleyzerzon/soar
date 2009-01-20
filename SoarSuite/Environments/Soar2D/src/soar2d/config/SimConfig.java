package soar2d.config;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.lang.reflect.Field;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map.Entry;

import soar2d.Names;

public class SimConfig implements GameConfig {
	public enum Game { TANKSOAR, EATERS, KITCHEN, TAXI, ROOM }
	
	public static SimConfig load(String configPath) throws IOException, IllegalArgumentException {
		Config config = ConfigUtil.tryLoadConfig(configPath);
		if (config == null)
			throw new IOException();
		
		return new SimConfig(config);
	}
	
	private static class Keys {
		private static final String last_productions = "last_productions";
		private static final String window_position = "window_position";
		private static final String general = "general";
		private static final String logging = "logging";
		private static final String soar = "soar";
		private static final String terminals = "terminals";
		private static final String players = "players";
		private static final String active_players = players + ".active_players";
		private static final String clients = "clients";
		private static final String active_clients = clients + ".active_clients";
		private static final String points = "points"; // keep in synch with PlayerConfig
	}
	
	private Game game;
	private Config preferences;
	private File preferencesFile;

	private Config config;
	
	private GeneralConfig generalConfig;
	private String loggingConfig;
	private SoarConfig soarConfig;
	private TerminalsConfig terminalsConfig;
	
	private GameConfig gameConfig;
	
	private HashMap<String, PlayerConfig> playerConfigs = new HashMap<String, PlayerConfig>();
	private HashMap<String, ClientConfig> clientConfigs = new HashMap<String, ClientConfig>();;
	
	private SimConfig(Config config) throws IOException {
		this.config = config;
		
		generalConfig = new GeneralConfig();
		loadSubConfig(config.getChild(Keys.general), GeneralConfig.class.getFields(), generalConfig);

		loggingConfig = config.getString(Keys.logging);

		soarConfig = new SoarConfig();
		loadSubConfig(config.getChild(Keys.soar), SoarConfig.class.getFields(), soarConfig);

		terminalsConfig = new TerminalsConfig();
		loadSubConfig(config.getChild(Keys.terminals), TerminalsConfig.class.getFields(), terminalsConfig);

		try {
			game = Game.valueOf(generalConfig.game.toUpperCase());
		} catch (IllegalArgumentException e) {
			StringBuilder sb = new StringBuilder();
			sb.append("Unknown game type: " + generalConfig.game + "\nKnown game types:");
			for (Game gameType : Game.values()) {
				sb.append(" ");
				sb.append(gameType.toString().toLowerCase());
			}
			throw new IllegalArgumentException(sb.toString());
		}
		
		Config childConfig = config.getChild(game.toString().toLowerCase());
		switch (game) {
		case TANKSOAR:
			gameConfig = new TankSoarConfig();
			loadSubConfig(childConfig, TankSoarConfig.class.getFields(), gameConfig);
			break;
		case EATERS:
			gameConfig = new EatersConfig();
			loadSubConfig(childConfig, EatersConfig.class.getFields(), gameConfig);
			break;
		case KITCHEN:
			gameConfig = new KitchenConfig();
			loadSubConfig(childConfig, KitchenConfig.class.getFields(), gameConfig);
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

		try {
			preferencesFile = new File(generalConfig.preferences_file);
			if (preferencesFile.exists()) {
				preferences = new Config(new ConfigFile(preferencesFile.getAbsolutePath()));
			} else {
				preferences = new Config(new ConfigFile());
			}
		} catch (IOException e) {
			System.err.println("Exception loading preferences: " + e.getMessage());
			preferences = new Config(new ConfigFile());
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
			// This shouldn't happen as long as all fields are public.
			System.err.println("Reflection error loading " + game.toString() + " config.");
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
	
	public String loggingConfig() {
		return loggingConfig;
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
	
	public KitchenConfig kitchenConfig() {
		return (KitchenConfig)gameConfig;
	}
	
	public RoomConfig roomConfig() {
		return (RoomConfig)gameConfig;
	}
	
	public TaxiConfig taxiConfig() {
		return (TaxiConfig)gameConfig;
	}
	
	public HashMap<String, PlayerConfig> playerConfigs() {
		return playerConfigs;
	}
	
	public HashMap<String, ClientConfig> clientConfigs() {
		return clientConfigs;
	}
	
	public void saveLastProductions(String productionsPath) {
		String game_specific_key = game.toString().toLowerCase() + "." + Keys.last_productions;
		preferences.setString(game_specific_key, productionsPath);
	}
	
	public String getLastProductions() {
		String game_specific_key = game.toString().toLowerCase() + "." + Keys.last_productions;
		if (preferences.hasKey(game_specific_key)) {
			return preferences.getString(game_specific_key);
		}
		return null;
	}
	
	public void saveWindowPosition(int [] xy) {
		preferences.setInts(Keys.window_position, xy);
	}
	
	public int [] getWindowPosition() {
		if (preferences.hasKey(Keys.window_position)) {
			return preferences.getInts(Keys.window_position);
		}
		return null;
	}
	
	public void savePreferences() {
		// meta preferences that need to be preserved across runs
		try {
			preferences.save(preferencesFile.getAbsolutePath());
		} catch (FileNotFoundException e) {
			System.err.println("Couldn't save preferences: " + e.getMessage());
		}
	}
		
	public static void main(String [] args) {
		try {
			Config config = new Config(new ConfigFile("Environments/Soar2D/src/soar2d/config/sim-config-test.cnf"));
			SimConfig sc = new SimConfig(config);
			System.out.println("title: " + sc.title());
			RoomConfig rc = sc.roomConfig();
			System.out.println("colored_rooms: " + rc.colored_rooms);
			System.out.println("players:");
			for (Entry<String, PlayerConfig> entry : sc.playerConfigs.entrySet()) {
				System.out.println(entry.getKey() + ": " + Arrays.toString(entry.getValue().pos) + ", " + Arrays.toString(entry.getValue().shutdown_commands));
			}
			System.out.print("clients:");
			for (String clientName : sc.clientConfigs.keySet()) {
				System.out.print(" " + clientName);
			}
			System.out.println();
			
		} catch (Throwable e) {
			System.err.println("exception: " + e.getMessage());
		}
	}

	public boolean runTilOutput() {
		return gameConfig.runTilOutput();
	}

	public String title() {
		return gameConfig.title();
	}
}
