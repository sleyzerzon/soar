package org.msoar.sps.control;

import java.io.IOException;
import java.util.List;
import java.util.Timer;
import java.util.TimerTask;

import org.apache.log4j.Logger;
import org.msoar.sps.config.Config;
import org.msoar.sps.config.ConfigFile;

final class Controller extends TimerTask {
	private static final Logger logger = Logger.getLogger(Controller.class);
	
	private final Config config;
	private final Gamepad gp;
	private final SoarInterface soar;
	private final Timer timer = new Timer();
	private final HttpController httpController = HttpController.newInstance();
	private final SplinterModel splinter = SplinterModel.newInstance();
	private DifferentialDriveCommand ddc;
	private boolean override = false;
	
	private Controller(Config config) {
		if (config == null) {
			throw new NullPointerException();
		}
		this.config = config;
		
		Gamepad gamepad = null;
		try {
			gamepad = new Gamepad();
			Buttons.setGamepad(gamepad);
		} catch (IllegalStateException e) {
			logger.warn("Disabling gamepad: " + e.getMessage());
		}
		gp = gamepad;

		soar = SoarInterface.newInstance(this.config, splinter);

		Runtime.getRuntime().addShutdownHook(new ShutdownHook());

		// TODO: make configurable
		timer.schedule(this, 0, 1000 / 20); // 20 Hz	
	}
	
	private class ShutdownHook extends Thread {
		@Override
		public void run() {
			soar.shutdown();

			System.out.flush();
			System.err.println("Terminated");
			System.err.flush();
		}
	}

	@Override
	public void run() {
		for (Buttons button : Buttons.values()) {
			button.update();
		}

		List<String> messageTokens = httpController.getMessageTokens();
		if (messageTokens != null) {
			soar.setStringInput(messageTokens);
		}

		if (Buttons.SOAR.checkAndDisable()) {
			soar.changeRunningState();
		}
		
		if (Buttons.OVERRIDE.checkAndDisable()) {
			override = !override;
			ddc = DifferentialDriveCommand.newMotorCommand(0, 0);
		}

		if (override) {
			ddc = getGPDDCommand();
			logger.trace("gmpd: " + ddc);
		} else {
			if (httpController.hasDDCommand()) {
				ddc = httpController.getDDCommand();
				logger.trace("http: " + ddc);
			} else {
				if (soar.hasDDCommand()) {
					ddc = soar.getDDCommand();
					logger.trace("soar: " + ddc);
				} else {
					logger.trace("cont: " + ddc);
				}
			}
		}

		splinter.update(ddc);
	}
	
	private DifferentialDriveCommand getGPDDCommand() {
		double left;
		double right;
		
		if (Buttons.TANK.isEnabled()) {
			left = gp.getAxis(1) * -1;
			right = gp.getAxis(3) * -1;
		} else {
			// this should not be linear, it is difficult to precicely control
			double fwd = -1 * gp.getAxis(3); // +1 = forward, -1 = back
			double lr = -1 * gp.getAxis(2); // +1 = left, -1 = right

			left = fwd - lr;
			right = fwd + lr;

			double max = Math.max(Math.abs(left), Math.abs(right));
			if (max > 1) {
				left /= max;
				right /= max;
			}
		}
		
		if (Buttons.SLOW.isEnabled()) {
			left *= 0.5;
			right *= 0.5;
		}
		
		return DifferentialDriveCommand.newMotorCommand(left, right);
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
		new Controller(config);
	}

}
