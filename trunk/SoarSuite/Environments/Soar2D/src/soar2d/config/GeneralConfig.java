package soar2d.config;

public class GeneralConfig {
	public double cycle_time_slice = 50;
	public int default_points = 0;
	public boolean force_human = false;
	public String game = "unspecified";
	public boolean hidemap = false;
	public String map = null;
	public boolean nogui = false;
	public String preferences_file = "preferences";
	public int runs = 0;
	public int seed = 0; // Note: has special field in SimConfig for null case TODO: make Integer and null
	public boolean tosca = false;
}
