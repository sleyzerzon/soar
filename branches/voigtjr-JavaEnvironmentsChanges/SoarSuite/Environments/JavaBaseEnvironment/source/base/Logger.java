package base;

import java.io.*;

public class Logger {
	private Writer output = null;
	private int level = 0;

	public Logger() {
	}

	public Logger(String path, boolean clear, int level) throws IOException {
		output = new BufferedWriter(new FileWriter(new File(path), !clear));
		this.level = level;
	}

	public void log(String message, int minLevel) {
		if (level >= minLevel) {
			log(message);			
		}
	}

	public void log(String message) {
		if (output != null) {
			try {
				output.write(message);
				output.flush();
			} catch (IOException e) {
				// TODO: Warn here!
			}
		}//log
	}

	public void close() {
		if (output != null) {
			try {
				output.close();
			} catch (IOException e) {
				// TODO: Warn here!
			}
		}
	}
}
