package eaters;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.events.SelectionListener;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.layout.FillLayout;
import org.eclipse.swt.layout.RowLayout;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Menu;
import org.eclipse.swt.widgets.MenuItem;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.Text;

import utilities.Logger;

public class EatersWindowManager {
	public static final int kMainMapCellSize = 20;
	
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

	protected Logger m_Logger = Logger.logger;
	protected Display m_Display;
	protected Shell m_Shell;
	protected EatersSimulation m_Simulation;

	public EatersWindowManager(EatersSimulation simulation) {
		m_Display = new Display();
		m_Simulation = simulation;
		initColors(m_Display);
		
		m_Shell = new Shell(m_Display);
		
		Menu menu = new Menu(m_Shell, SWT.BAR);
		
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
		
		m_Shell.setLayout(new FillLayout());
		
		new VisualWorld(m_Shell, simulation, kMainMapCellSize);
		
		m_Shell.open();
		while (!m_Shell.isDisposed()) {
			if (!m_Display.readAndDispatch()) {
				m_Display.sleep();
			}
		}
		m_Display.dispose();		
	}
	
}
