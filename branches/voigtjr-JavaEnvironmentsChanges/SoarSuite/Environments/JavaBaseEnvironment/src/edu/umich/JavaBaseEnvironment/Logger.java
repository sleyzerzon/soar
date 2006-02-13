package edu.umich.JavaBaseEnvironment;

import java.io.*;

public class Logger {
	private Writer output = null;

	private String path = null;

	private boolean failSilently = true;//Pretend to log but don't really

	private boolean clear = false;

	private int level = 0;

	public Logger(String path, boolean clear, int level) {

		if (path != null) {
			this.clear = clear;
			this.path = path;
			this.level = level;
	
			failSilently = false;
		}
	}

	public Logger(String path) {
		if (path != null) {
			this.path = path;

			failSilently = false;
		}
	}

	public Logger() {
	}

	public void log(String message, int minLevel) {
		if (level >= minLevel)
			log(message);
	}

	public void log(String message) {
		if (!failSilently) {
			if (output == null) {
				try {
					output = new BufferedWriter(new FileWriter(new File(path),
							!clear));
				} catch (IOException e) {
				}
			}//if 

			try {
				output.write(message);
				output.flush();
			} catch (IOException e) {
			}
		}//log
	}

	public void close() {
		if (!failSilently) {
			try {
				output.close();
			} catch (IOException e) {
			}
		}
	}
}
