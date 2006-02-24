package utilities;

import java.io.*;
import java.sql.*;

public class Logger {
	public static Logger logger = new Logger();
	
	protected final String kDefaultFile = "EaterLog.txt";
	protected Writer m_Output;

	public void toFile() {
		toFile(null);
	}
	
	public void toFile(String filename) {
		if (filename == null) {
			filename = kDefaultFile;
		}
		
		try {
			m_Output = new BufferedWriter(new FileWriter(new File(filename), false));
		} catch (IOException e) {
			System.out.println("Exception creating logger: " + e.getMessage());
			System.exit(1);
		}

		Timestamp ts = new Timestamp(System.currentTimeMillis());
		log("Log started: " + ts);
	}

	public void log(String message) {
		if (m_Output == null) {
			System.out.println(message);
		}
		
		try {
			m_Output.write(message);
			m_Output.write("\n");
			m_Output.flush();
		} catch (IOException e) {
			System.out.println("Warning: logger write failed.");
		}
	}
}
