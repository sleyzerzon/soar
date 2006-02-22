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
	final MapButtons m_MapButtons;
	final VisualWorld m_VisualWorld;
	final AgentDisplay m_AgentDisplay;
	
	public EatersWindowManager(EatersSimulation simulation) {
		m_Display = new Display();
		m_Simulation = simulation;

		initColors(m_Display);
		
		m_Shell = new Shell(m_Display);
		GridLayout gl = new GridLayout();
		gl.numColumns = 2;
		m_Shell.setLayout(gl);
		
		GridData gd;
		
		m_VisualWorld = new VisualWorld(m_Shell, SWT.NONE, m_Simulation, kMainMapCellSize);
		gd = new GridData();
		gd.widthHint = m_VisualWorld.getWidth();
		gd.heightHint = m_VisualWorld.getHeight();
		gd.verticalSpan = 3;
		m_VisualWorld.setLayoutData(gd);

		Group group1 = new Group(m_Shell, SWT.NONE);
		gd = new GridData();
		group1.setLayoutData(gd);
		group1.setText("Simulation");
		group1.setLayout(new FillLayout());
		m_SimButtons = new SimButtons(group1, m_Simulation);
		
		Group group2 = new Group(m_Shell, SWT.NONE);
		gd = new GridData();
		group2.setLayoutData(gd);
		group2.setText("Map");
		group2.setLayout(new RowLayout(SWT.VERTICAL));
		m_FoodCount = new Label(group2, SWT.NONE);
		m_FoodCount.setLayoutData(new RowData());
		m_FoodCount.setText(kFoodRemaining + m_Simulation.getWorld().getFoodCount());
		m_MapButtons = new MapButtons(group2, m_Simulation);
		m_MapButtons.setLayoutData(new RowData());

		m_AgentDisplay = new AgentDisplay(m_Shell, m_Simulation);
		gd = new GridData();
		m_AgentDisplay.setLayoutData(gd);
		
		m_Simulation.addSimulationListener(this);

		m_Shell.setText("Java Eaters");
		m_Shell.setSize(m_Shell.computeSize(SWT.DEFAULT, SWT.DEFAULT));
		
		m_Shell.open();

		while (!m_Shell.isDisposed()) {
			if (!m_Display.readAndDispatch()) {
				m_Display.sleep();
			}
		}
		m_Display.dispose();		
	}
	
	void dispatchEvent(int type) {
		switch (type) {
		case SimulationListener.kStartEvent:
			m_SimButtons.updateButtons();
			m_MapButtons.updateButtons();
			m_AgentDisplay.updateButtons();
			return;
			
		case SimulationListener.kStopEvent:
			m_VisualWorld.redraw();
			m_SimButtons.updateButtons();
			m_MapButtons.updateButtons();
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
