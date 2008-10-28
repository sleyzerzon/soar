package splintersoar.laserloc;

import java.io.DataInputStream;
import java.io.IOException;
import java.util.Arrays;
import java.util.logging.Level;
import java.util.logging.Logger;

import lcm.lcm.LCM;
import lcm.lcm.LCMSubscriber;
import lcmtypes.laser_t;

import splintersoar.LCMInfo;
import splintersoar.LogFactory;
import splintersoar.lcmtypes.xy_t;

import april.config.Config;
import april.config.ConfigFile;

/**
 * @author voigtjr
 * Laser localizer class. Takes as input SICK data and generates x,y position of beacon.
 */
public class LaserLoc extends Thread implements LCMSubscriber {
	private class Configuration {
		double [] laserxyt = { 0, 0, 0 }; // laser location
		double tubeRadius = 0; 
		int updatePeriod = 5; // seconds between status updates
		int activityTimeout = 5;
		double[] maxRanges;
		String mapFile = "map.txt";
		
		long updatePeriodNanos;
		long activityTimeoutNanos;

		Configuration(Config config) {
			if (config != null) {
				laserxyt = config.getDoubles("laserxyt", laserxyt);
				tubeRadius = config.getDouble("tubeRadius", tubeRadius);
				updatePeriod = config.getInt("updatePeriod", updatePeriod);
				activityTimeout = config.getInt("activityTimeout", activityTimeout);
				mapFile = config.getString("mapFile", mapFile);
			}

			try {
				Config mapConfig = new ConfigFile(mapFile).getConfig();
				maxRanges = mapConfig.getDoubles("map");
			} catch (IOException e) {
			}

			updatePeriodNanos = updatePeriod * 1000000000L;
			activityTimeoutNanos = activityTimeout * 1000000000L;
		}
	}

	private Configuration configuration;

	// Assumptions:
	// robot_z is constant
	// robot starts at a known angle
	// this level only reports laser data (next layer up decided whether to use
	// odometry or not)
	// distances in meters

	// regular state
	laser_t laserData;

	int droppedLocPackets = 0;
	long lastStatusUpdate = System.nanoTime();

	LCM lcm;

	boolean inactive = true;
	long nanolastactivity = System.nanoTime();
	long currentTimeout;

	private Logger logger;
	RoomMapper mapper;

	public LaserLoc(Config config) {
		configuration = new Configuration(config);

		logger = LogFactory.createSimpleLogger("LaserLoc", Level.INFO);

		lcm = LCM.getSingleton();
		lcm.subscribe(LCMInfo.LASER_LOC_CHANNEL, this);

		if (configuration.maxRanges == null) {
			logger.warning("No map file found, using infinite maximums");
			configuration.maxRanges = new double[180];
			Arrays.fill(configuration.maxRanges, Double.MAX_VALUE);
		}

		currentTimeout = configuration.activityTimeoutNanos;
	}

	private void updatePose() {
		long nanotime = System.nanoTime();

		if (nanotime - nanolastactivity > currentTimeout) {
			inactive = true;
			logger.warning(String.format("no activity in last " + (currentTimeout / 1000000000.0) + " seconds"));
			nanolastactivity = nanotime;
			currentTimeout += configuration.activityTimeoutNanos;
		}

		// occasionally print out status update
		long nanoelapsed = nanotime - lastStatusUpdate;
		if (nanoelapsed > configuration.updatePeriodNanos) {
			if (droppedLocPackets > 0) {
				double dropRate = (double) droppedLocPackets / (nanoelapsed / 1000000000.0);
				logger.warning(String.format("LaserLoc: dropping %5.1f packets/sec", dropRate));
			}

			droppedLocPackets = 0;
			lastStatusUpdate = nanotime;
		}

		if (laserData == null) {
			try {
				Thread.sleep(50);
			} catch (InterruptedException ignored) {
			}
			return;
		}

		nanolastactivity = nanotime;
		currentTimeout = configuration.activityTimeoutNanos;

		if (inactive) {
			logger.info("receiving data");
			inactive = false;
		}

		laser_t ld = laserData.copy();
		laserData = null;
		if (lastutime == ld.utime) {
			if (logger.isLoggable(Level.FINE))
				logger.fine("Skipping message, time hasn't changed");

			try {
				Thread.sleep(50);
			} catch (InterruptedException ignored) {
			}
			return;
		}
		lastutime = ld.utime;

		xy_t estimatedCoords = getRobotXY(ld);
		if (estimatedCoords == null)
			return;

		if (logger.isLoggable(Level.FINE))
			logger.fine(String.format("publishing %10.3f %10.3f", estimatedCoords.xy[0], estimatedCoords.xy[1]));

		lcm.publish(LCMInfo.COORDS_CHANNEL, estimatedCoords);
	}
	
	long lastutime = 0;

	private xy_t getRobotXY(laser_t ld) {
		int nranges = Math.min( configuration.maxRanges.length, ld.nranges);
		if (configuration.maxRanges.length != ld.nranges)
		{
			logger.fine(String.format("maxRanges array not equal in size to nranges, %d %d", configuration.maxRanges.length, ld.nranges));
		}
		
		double smallestRange = Double.MAX_VALUE;
		int smallestRangeIndex = -1;
		for (int index = 0; index < nranges; ++index) {
			if (ld.ranges[index] < configuration.maxRanges[index]) {
				if (ld.ranges[index] < smallestRange) {
					smallestRange = ld.ranges[index];
					smallestRangeIndex = index;
				}
			}
		}

		if (smallestRangeIndex == -1)
		{
			logger.warning("did not find smallest range (is there nothing in view?");
			return null;
		}
		
		double laserAngle = configuration.laserxyt[2] + ld.rad0 + ld.radstep * smallestRangeIndex;

		double laserDist = smallestRange + configuration.tubeRadius;

		xy_t newCoords = new xy_t();
		newCoords.utime = ld.utime;
		newCoords.xy[0] = configuration.laserxyt[0] + laserDist * Math.cos(laserAngle);
		newCoords.xy[1] = configuration.laserxyt[1] + laserDist * Math.sin(laserAngle);
		
		return newCoords;
	}

	@Override
	public void messageReceived(LCM lcm, String channel, DataInputStream ins) {
		if (channel.equals(LCMInfo.LASER_LOC_CHANNEL)) {
			if (laserData != null) {
				droppedLocPackets += 1;
				return;
			}

			try {
				laserData = new laser_t(ins);
			} catch (IOException ex) {
				System.err.println("Error decoding laser message: " + ex);
			}
		}

	}

	@Override
	public void run() {
		while (true) {
			updatePose();
		}
	}

	public static void main(String[] args) {
		Config config = null;
		if (args.length == 1) {
			try {
				config = (new ConfigFile(args[0])).getConfig();
			} catch (IOException ex) {
				System.err.println("Couldn't open config file: " + args[0]);
				return;
			}
		}

		LaserLoc lloc = new LaserLoc(config);
		lloc.run();
	}

}
