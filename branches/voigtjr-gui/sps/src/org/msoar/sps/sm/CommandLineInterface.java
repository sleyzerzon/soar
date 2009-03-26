package org.msoar.sps.sm;

import java.io.PrintWriter;

import org.apache.log4j.Logger;

final class CommandLineInterface {
	private static final Logger logger = Logger.getLogger(CommandLineInterface.class);

	private enum Command { EXIT, QUIT, START, STOP, RESTART; }
	private enum CommandResult { CONTINUE, STOP; }
	
	private final Components components;
    private PrintWriter writer = new PrintWriter(new NullWriter(), true);

	CommandLineInterface(Components components) {
		this.components = components;
	}
	
	boolean executeCommand(String command) {
		if (handleCommand(command) == CommandResult.STOP) {
			return false;
		}
		return true;
	}
	
	void setWriter(PrintWriter writer) {
		this.writer.flush();
		this.writer = writer != null ? writer : new PrintWriter(new NullWriter(), true);
	}
	
	private CommandResult handleCommand(String command) {
		logger.trace("command: " + command);
		String[] args = command.split("\\s+");
		if (args[0].length() != 0) {
			Command cmd = null;
			try {
				cmd = Command.valueOf(args[0].toUpperCase());
			} catch (IllegalArgumentException ignored) {
			}
			if (cmd == null) {
				String message = "Unknown command: " + args[0];
				writer.println(message);
				logger.error(message);
			} else {
				String component = args.length > 1 ? args[1] : null;
				switch (cmd) {
				case QUIT:
				case EXIT:
					components.close();
					return CommandResult.STOP;
					
				case START:
					components.start(component);
					break;
					
				case STOP:
					components.stop(component);
					break;
					
				case RESTART:
					components.stop(component);
					try {
						String message = "Sleeping for 5 seconds.";
						writer.println(message);
						logger.info(message);
						Thread.sleep(5000);
					} catch (InterruptedException ignored) {
					}
					components.start(component);
					break;
				}
			}
		}
		return CommandResult.CONTINUE;
	}
}

