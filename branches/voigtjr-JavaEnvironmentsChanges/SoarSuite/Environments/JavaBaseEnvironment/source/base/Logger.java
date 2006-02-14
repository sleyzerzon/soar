package base;

import java.io.*;
import java.util.Calendar;
import java.util.GregorianCalendar;

public class Logger {
	private Writer output = null;
	private int level = 0;

	public Logger() {
	}

	public Logger(String path, boolean clear, int level) throws IOException {
		if (path == null)
			return;
		output = new BufferedWriter(new FileWriter(new File(path), !clear));
		this.level = level;

		Calendar cal = new GregorianCalendar();
		// TODO: Check the date, the month was off in preliminary testing
		log("Log started: " + cal.get(Calendar.YEAR) + "-"
				+ cal.get(Calendar.MONTH) + "-" + cal.get(Calendar.DAY_OF_MONTH));
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
				output.write("\n");
				output.flush();
			} catch (IOException e) {
				// TODO: Warn here!
			}
		} else {
			System.out.println(message);
		}
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
