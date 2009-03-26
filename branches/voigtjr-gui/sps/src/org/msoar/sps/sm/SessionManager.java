package org.msoar.sps.sm;

import java.awt.BorderLayout;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.File;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.List;

import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.SwingUtilities;
import javax.swing.UIManager;
import javax.swing.UnsupportedLookAndFeelException;

import org.apache.log4j.Logger;
import org.flexdock.util.SwingUtility;
import org.flexdock.view.View;
import org.flexdock.view.Viewport;
import org.msoar.sps.config.Config;
import org.msoar.sps.config.ConfigFile;

final class SessionManager extends JPanel {
	private static final long serialVersionUID = -2198876413268591195L;

	private static class Names {
		private static final String USAGE = 
			"Usage: java -jar sps.jar CONFIG_FILE [wait]\n" +
			"       java -jar sps.jar COMPONENT@HOSTNAME\n" +
			"Run session manager master instance using CONFIG_FILE or run slave COMPONENT\n" +
			"on machine HOSTNAME. Master instance starts components locally by default but\n" +
			"can optionally be told to 'wait' for network connections.\n" +
			"\n" +
			"Working directory must be soar-robotics/sps.\n";
		private static final String INCLUDE = "include";
		
		private Names() { throw new AssertionError(); }
	}
	
	//private static final Logger logger = Logger.getLogger(SessionManager.class);
	private static final int PORT = 42140;
	
	private final Components components;
	private final CommandLineInterface cli;
    private JFrame frame;
    private Viewport viewport = new Viewport();
    private final List<View> views = new ArrayList<View>();
    

	private SessionManager(boolean autoStart, Config config) throws IOException {
        super(new BorderLayout());

		this.components = new Components(config);
		this.cli = new CommandLineInterface(components);
		new Acceptor(this.components, PORT);
	}
	
	private void initialize(JFrame parentFrame) {
        this.frame = parentFrame;
        
        this.add(viewport, BorderLayout.CENTER);
        
        initViews();
        
        this.frame.setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);
        this.frame.addWindowListener(new WindowAdapter() {

            @Override
            public void windowClosing(WindowEvent e)
            {
                exit();
            }});
	}
	
	private void exit() {
        System.exit(0);
	}

    private void initViews() {
    	final CLIView cliView = new CLIView(this);
    	views.add(cliView);
    	viewport.dock(cliView);
    }
    
	public static void main(final String[] args) {
		if (args.length > 2 || args.length < 1) {
			System.err.print(Names.USAGE);
			System.exit(1);
		}

		try {
			if (args[0].indexOf("@") != -1) {
				if (args.length == 2) {
					System.err.print(Names.USAGE);
					System.exit(1);
				}
				// start a slave
				String[] netInfo = args[0].split("@");
				if (netInfo.length != 2) {
					System.err.print(Names.USAGE);
					System.exit(1);
				}
				ClientConnection.newInstance(netInfo[0], netInfo[1], PORT);
				
			} else {
				// load config
				final Config config = loadConfig(args);
				final boolean autoStart = determineAutoStart(args);
				
				initializeLookAndFeel();
				
				SwingUtilities.invokeLater(new Runnable() {
					public void run() { initialize(autoStart, config); }
				});
			}
			
		} catch (IOException e) {
			System.err.println(e.getMessage());
			System.err.print(Names.USAGE);
			System.exit(1);
		}
	}
	
    private static void initializeLookAndFeel() {
        try
        {
            // Use the look and feel of the system we're running on rather
            // than Java. If an error occurs, we proceed normally using
            // whatever L&F we get.
            UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
        }
        catch (UnsupportedLookAndFeelException e) { }
        catch (ClassNotFoundException e) { }
        catch (InstantiationException e) { }
        catch (IllegalAccessException e) { }
    }
    
	private static boolean determineAutoStart(String[] args) {
		if (args.length == 2) {
			if (args[1].equals("wait")) {
				return false;
			}
		}
		return true;
	}
	
	private static Config loadConfig(String[] args) throws IOException {
		Config config = new Config(new ConfigFile(args[0]));
		for (String include : config.getStrings(Names.INCLUDE, new String[0])) {
			Config includedConfig = new Config(new ConfigFile("config" + File.separator + include));
			config.merge(includedConfig);
		}
		return config;
	}
	
	private static SessionManager initialize(boolean autoStart, Config config) {
		try {
			SessionManager sm = new SessionManager(autoStart, config);
			JFrame frame = new JFrame("Session Manager");
	        frame.setContentPane(sm);
	        frame.setSize(400, 600);

	        sm.initialize(frame);

	        SwingUtility.centerOnScreen(frame);
	        frame.setVisible(true);
			
	        return sm;
			
		} catch (IOException e) {
			System.err.println(e.getMessage());
			System.err.print(Names.USAGE);
			System.exit(1);
		}
		return null;
	}

	public void setWriter(PrintWriter writer) {
		cli.setWriter(writer);
		components.setWriter(writer);
	}

	public void executeCommand(String command) {
		if (cli.executeCommand(command) == false) {
			exit();
		}
	}
}
