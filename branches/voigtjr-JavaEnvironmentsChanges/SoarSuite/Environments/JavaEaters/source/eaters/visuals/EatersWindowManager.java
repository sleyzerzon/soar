package eaters.visuals;

import org.eclipse.swt.*;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.graphics.*;
import org.eclipse.swt.layout.*;

import eaters.*;
import utilities.*;

public class EatersWindowManager extends Thread implements SimulationListener {
	public static final int kMainMapCellSize = 20;
	public static final String kFoodRemaining = "Food remaining: ";
	public static final String kColors[] = { "white", "blue", "red", "yellow", "orange", "black", "green", "purple" };
	
	public static Color white;
	public static Color blue;
	public static Color red;
	public static Color widget_background;
	public static Color yellow;
	public static Color orange;
	public static Color black;
	public static Color green;
	public static Color purple;

	public static void initColors(Display d) {
	    white = d.getSystemColor(SWT.COLOR_WHITE);
		widget_background = d.getSystemColor(SWT.COLOR_WIDGET_BACKGROUND);
		blue = d.getSystemColor(SWT.COLOR_BLUE);
		red = d.getSystemColor(SWT.COLOR_RED);
		yellow = d.getSystemColor(SWT.COLOR_YELLOW);
		green = d.getSystemColor(SWT.COLOR_GREEN);
		purple = d.getSystemColor(SWT.COLOR_DARK_MAGENTA);
		orange = new Color(d, 255, 127, 0);
		black = d.getSystemColor(SWT.COLOR_BLACK);
	}
	
	public static Color getColor(String color) {
		if (color.equalsIgnoreCase("white")) {
			return white;
		}
		if (color.equalsIgnoreCase("blue")) {
			return blue;
		}
		if (color.equalsIgnoreCase("red")) {
			return red;
		}
		if (color.equalsIgnoreCase("yellow")) {
			return yellow;
		}
		if (color.equalsIgnoreCase("green")) {
			return green;
		}
		if (color.equalsIgnoreCase("purple")) {
			return purple;
		}
		if (color.equalsIgnoreCase("orange")) {
			return orange;
		}
		if (color.equalsIgnoreCase("black")) {
			return black;
		}
		return null;
	}

	Logger m_Logger = Logger.logger;
	Display m_Display;
	Shell m_Shell;
	EatersSimulation m_Simulation;
	final Label m_FoodCount;
	final SimButtons m_SimButtons;
	final VisualWorld m_VisualWorld;
	final AgentDisplay m_AgentDisplay;
	
	public EatersWindowManager(EatersSimulation simulation) {
		m_Display = new Display();
		m_Simulation = simulation;

		initColors(m_Display);
		
		m_Shell = new Shell(m_Display);
		m_Shell.setLayout(new RowLayout(SWT.VERTICAL));
		
		createMenu();
		
		m_SimButtons = new SimButtons(m_Shell, m_Simulation);
		
		m_VisualWorld = new VisualWorld(m_Shell, SWT.NONE, m_Simulation, kMainMapCellSize);
		m_VisualWorld.setLayoutData(new RowData(m_VisualWorld.getWidth(), m_VisualWorld.getHeight()));
		
		m_FoodCount = new Label(m_Shell, SWT.NONE);
		m_FoodCount.setText(kFoodRemaining + m_Simulation.getWorld().getFoodCount());

		m_AgentDisplay = new AgentDisplay(m_Shell, m_Simulation);
		m_AgentDisplay.setLayoutData(new RowData());
		
		m_Simulation.addSimulationListener(this);

		m_Shell.setText("Java Eaters");
		//m_Shell.setSize(420,300);
		m_Shell.setSize(m_Shell.computeSize(SWT.DEFAULT, 280));
		
		m_Shell.open();

		while (!m_Shell.isDisposed()) {
			if (!m_Display.readAndDispatch()) {
				m_Display.sleep();
			}
		}
		m_Display.dispose();		
	}
	
	public void createMenu() {
		final Menu menu = new Menu(m_Shell, SWT.BAR);
		
		final MenuItem file = new MenuItem(menu, SWT.CASCADE);		
		file.setText("File");
		
		final Menu filemenu = new Menu(m_Shell, SWT.DROP_DOWN);
		file.setMenu(filemenu);
		
		final MenuItem exit = new MenuItem(filemenu, SWT.PUSH);
		exit.setText("Exit");
		exit.addSelectionListener(new SelectionListener() {
			public void widgetSelected(SelectionEvent e) {
				m_Simulation.shutdown(0);
			}
			public void widgetDefaultSelected(SelectionEvent e) {
				
			}
		});
		
		m_Shell.setMenuBar(menu);			
	}
	
	void dispatchEvent(int type) {
		switch (type) {
		case SimulationListener.kStartEvent:
			m_SimButtons.updateButtons();
			m_AgentDisplay.updateButtons();
			return;
			
		case SimulationListener.kStopEvent:
			m_VisualWorld.redraw();
			m_SimButtons.updateButtons();
			m_AgentDisplay.updateButtons();
			return;
			
		case SimulationListener.kShutdownEvent:
			return;
			
		case SimulationListener.kUpdateEvent:
			m_VisualWorld.redraw();
			updateFoodCount();
			m_AgentDisplay.worldChangeEvent();
			return;
			
		case SimulationListener.kNewWorldEvent:
			m_VisualWorld.redraw();
			updateFoodCount();
			m_SimButtons.updateButtons();
			m_AgentDisplay.worldChangeEvent();
			return;
			
		case SimulationListener.kAgentCreatedEvent:
			m_VisualWorld.redraw();
			updateFoodCount();
			m_SimButtons.updateButtons();
			m_AgentDisplay.agentEvent();
			return;
			
		case SimulationListener.kAgentDestroyedEvent:
			m_VisualWorld.redraw();
			m_SimButtons.updateButtons();
			m_AgentDisplay.agentEvent();
			return;
			
		default:
			m_Logger.log("Invalid event type received: " + new Integer(type));
			return;
		}		
	}
	
	void updateFoodCount() {
		m_FoodCount.setText(kFoodRemaining + new Integer(m_Simulation.getWorld().getFoodCount()));
	}

	public void simulationEventHandler(final int type) {
		if (m_Display.isDisposed()) {
			return;
		}
		m_Display.asyncExec(new Runnable() {
			public void run() {
				dispatchEvent(type);
			}
		});
	}
}
