package utilities;

import java.io.*;
import java.util.Calendar;
import java.util.GregorianCalendar;

public class Logger {
	public static Logger logger = new Logger();
	
	protected final String kDefaultFile = "EaterLog.txt";
	protected Writer m_Output;

	public Logger() {
		try {
			m_Output = new BufferedWriter(new FileWriter(new File(kDefaultFile), false));
		} catch (IOException e) {
			System.out.println("Exception creating logger: " + e.getMessage());
			System.exit(1);
		}

		Calendar cal = new GregorianCalendar();
		// TODO: Check the date, the month was off in preliminary testing
		log("Log started: " + cal.get(Calendar.YEAR) + "-"
				+ (cal.get(Calendar.MONTH) + 1) + "-" + cal.get(Calendar.DAY_OF_MONTH));
	}

	public void log(String message) {
		try {
			m_Output.write(message);
			m_Output.write("\n");
			m_Output.flush();
		} catch (IOException e) {
			// TODO: Warn here!
		}
	}
}
