package soar2d.map;

import soar2d.config.Config;
import soar2d.config.ConfigFile;

public class CellObjectHelper {
	static Config createNewConfig(String objectName) {
		ConfigFile cf = new ConfigFile();
		Config config = cf.getConfig();
		config.setString("name", objectName);
		return config;
	}
}
