package org.msoar.sps.splinter;

import java.io.DataInputStream;
import java.io.IOException;
import java.util.Timer;
import java.util.TimerTask;

import lcm.lcm.LCM;
import lcm.lcm.LCMSubscriber;
import lcmtypes.differential_drive_command_t;
import lcmtypes.pose_t;

import orc.Motor;
import orc.Orc;
import orc.OrcStatus;

import org.apache.log4j.Logger;
import org.msoar.sps.SharedNames;
import org.msoar.sps.Odometry;
import org.msoar.sps.config.Config;
import org.msoar.sps.config.ConfigFile;
import org.msoar.sps.lcmtypes.odom_t;

public final class Splinter extends TimerTask implements LCMSubscriber {
	private static final Logger logger = Logger.getLogger(Splinter.class);
	private static final int LEFT = 0;
	private static final int RIGHT = 1;
	private static final long DELAY_BEFORE_WARN_NO_FIRST_INPUT_MILLIS = 5000;
	private static final long DELAY_BEFORE_WARN_NO_INPUT_MILLIS = 1000;
	
	//green
	//public static final double DEFAULT_BASELINE = 0.383;
	//public static final double DEFAULT_TICKMETERS = 0.000043225;
	
	// blue
	public static final double DEFAULT_BASELINE = 0.37405;
	// TODO: should use two tickmeters, left/right
	public static final double DEFAULT_TICKMETERS = 0.0000428528;
	
	private final Timer timer = new Timer();
	private final Orc orc;
	private final Motor[] motor = new Motor[2];
	private final int[] ports;
	private final boolean[] invert;
	private final LCM lcm;
	private final double tickMeters;
	private final double baselineMeters;
	private final double[] command = { 0, 0 };
	private final double maxThrottleChangePerUpdate;

	private OdometryLogger capture;
	private differential_drive_command_t dc;
	private long lastSeenDCTime = 0;
	private long lastUtime = 0;
	private boolean failsafeSpew = false;

	// for odometry update
	private final Odometry odometry;
	private final odom_t oldOdom = new odom_t();
	private final pose_t pose = new pose_t();
	private final odom_t newOdom = new odom_t();
	
	private Splinter(Config config) {
		tickMeters = config.getDouble("tickMeters", DEFAULT_TICKMETERS);
		baselineMeters = config.getDouble("baselineMeters", DEFAULT_BASELINE);
		odometry = new Odometry(tickMeters, baselineMeters);
		
		double maxThrottleAccelleration = config.getDouble("maxThrottleAccelleration", 2.0);
		double updateHz = config.getDouble("updateHz", 30.0);
		maxThrottleChangePerUpdate = maxThrottleAccelleration / updateHz;
		
		orc = Orc.makeOrc(config.getString("orcHostname", "192.168.237.7"));

		// ports: what port the motors are hooked up to, invert: polarity
		ports = config.getInts("ports", new int[] {0,1});
		invert = config.getBooleans("invert", new boolean[] {true, true});
		
		motor[LEFT] = new Motor(orc, ports[LEFT], invert[LEFT]);
		motor[RIGHT] = new Motor(orc, ports[RIGHT], invert[RIGHT]);
		
		OrcStatus currentStatus = orc.getStatus();
		getOdometry(oldOdom, currentStatus);
		
		if (config.getBoolean("captureOdometry", false)) {
			try {
				capture = new OdometryLogger();
				capture.record(oldOdom);
			} catch (IOException e) {
				logger.error("Error opening odometry logger: " + e.getMessage());
			}
		}
		
		// drive commands
		lcm = LCM.getSingleton();
		lcm.subscribe(SharedNames.DRIVE_CHANNEL, this);
	
		double updatePeriodMS = 1000 / updateHz;
		logger.debug("Splinter thread running, period " + updatePeriodMS);
		timer.schedule(this, 0, (long)updatePeriodMS); 
	}
	
	private void getOdometry(odom_t dest, OrcStatus currentStatus) {
		dest.utime = currentStatus.utime;
		dest.left = currentStatus.qeiPosition[ports[LEFT]] * (invert[LEFT] ? -1 : 1);
		dest.right = currentStatus.qeiPosition[ports[RIGHT]] * (invert[RIGHT] ? 1 : -1);
	}
	
	@Override
	public void run() {
		// Get OrcStatus
		OrcStatus currentStatus = orc.getStatus();
		
		boolean moving = (currentStatus.qeiVelocity[0] != 0) || (currentStatus.qeiVelocity[1] != 0);
		
		getOdometry(newOdom, currentStatus);
		
		// don't update odom unless moving
		if (moving) {
			odometry.propagate(newOdom, oldOdom, pose);

			if (capture != null) {
				try {
					capture.record(newOdom);
				} catch (IOException e) {
					logger.error("IOException while writing odometry: " + e.getMessage());
				}
			}
		}

		// save old state
		oldOdom.left = newOdom.left;
		oldOdom.right = newOdom.right;
		
		pose.utime = currentStatus.utime;
		lcm.publish(SharedNames.POSE_CHANNEL, pose);
		
		commandMotors();
	}
	
	private void commandMotors() {
		if (dc == null) {
			if (lastSeenDCTime == 0) {
				lastSeenDCTime = System.currentTimeMillis();
			}
			long millisSinceLastCommand = System.currentTimeMillis() - lastSeenDCTime;
			if (millisSinceLastCommand > DELAY_BEFORE_WARN_NO_FIRST_INPUT_MILLIS) {
				logger.warn("No drive command yet"); 
				lastSeenDCTime = System.currentTimeMillis();
			}
			// we haven't seen a drive command yet
			commandFailSafe();
			return;
		}

		differential_drive_command_t dcNew = dc.copy();

		// is it a new command? 
		if (lastUtime != dcNew.utime) {
			// it is, save state
			lastSeenDCTime = System.currentTimeMillis();
			lastUtime = dcNew.utime;
		} else {
			// have we not seen a command in the last second?
			long millisSinceLastCommand = System.currentTimeMillis() - lastSeenDCTime;
			if (millisSinceLastCommand > DELAY_BEFORE_WARN_NO_INPUT_MILLIS) {
				if (failsafeSpew == false) {
					logger.error("No recent drive command " 
							+ millisSinceLastCommand / 1000.0 
							+ " seconds");
					failsafeSpew = true;
				}
				commandFailSafe();
				return;
			}
		}
		
		failsafeSpew = false;		
		if (logger.isTraceEnabled()) {
			logger.trace(String.format("Got input %1.2f %1.2f", dcNew.left, dcNew.right));
		}
		
		if (dcNew.left_enabled) {
			double delta = dcNew.left - command[LEFT];
			
			if (delta > 0) {
				delta = Math.min(delta, maxThrottleChangePerUpdate);
			} else if (delta < 0) {
				delta = Math.max(delta, -1 * maxThrottleChangePerUpdate);
			}

			command[LEFT] += delta;
			motor[LEFT].setPWM(command[LEFT]);
		} else {
			motor[LEFT].idle();
		}

		if (dcNew.right_enabled) {
			double delta = dcNew.right - command[RIGHT];
			
			if (delta > 0) {
				delta = Math.min(delta, maxThrottleChangePerUpdate);
			} else if (delta < 0) {
				delta = Math.max(delta, -1 * maxThrottleChangePerUpdate);
			}

			command[RIGHT] += delta;
			motor[RIGHT].setPWM(command[RIGHT]);
		} else {
			motor[RIGHT].idle();
		}

		if (logger.isTraceEnabled()) {
			logger.trace("Sending input " + (dcNew.left_enabled ? command[LEFT] : "(idle)") 
					+ " "
					+ (dcNew.right_enabled ? command[RIGHT] : "(idle)"));
		}
	}
	
	private void commandFailSafe() {
		motor[LEFT].setPWM(0);
		motor[RIGHT].setPWM(0);
	}

	public static void main(String[] args) {
		Config config = null;
		if (args.length > 0) {
			try {
				config = new Config(new ConfigFile(args[0]));
			} catch (IOException e) {
				logger.error(e.getMessage());
				System.exit(1);
			}
		} else {
			config = new Config(new ConfigFile());
		}
		new Splinter(config);
	}

	public void messageReceived(LCM lcm, String channel, DataInputStream ins) {
		if (channel.equals(SharedNames.DRIVE_CHANNEL)) {
			try {
				dc = new differential_drive_command_t(ins);
			} catch (IOException e) {
				logger.error("Error decoding differential_drive_command_t message: " + e.getMessage());
			}
		}
	}
}
