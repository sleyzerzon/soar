package eaters;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.events.SelectionListener;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.layout.FillLayout;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Menu;
import org.eclipse.swt.widgets.MenuItem;
import org.eclipse.swt.widgets.Shell;

import utilities.Logger;

public class EatersWindowManager {
	public static Color white;
	public static Color blue;
	public static Color red;
	public static Color widget_background;
	public static Color yellow;
	public static Color orange;
	public static Color black;
	public static Color green;
	public static Color purple;

	Display m_Display;
	Shell m_Shell;
	protected Logger m_Logger = Logger.logger;

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

	public EatersWindowManager(EatersSimulation control) {
		m_Display = new Display();
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
				System.exit(1);
			}
			public void widgetDefaultSelected(SelectionEvent e) {
				
			}
		});
		
		m_Shell.setMenuBar(menu);	
		
		m_Shell.setLayout(new FillLayout());
		
		new VisualWorld(m_Shell, control);
		
		m_Shell.open();
		while (!m_Shell.isDisposed()) {
			if (!m_Display.readAndDispatch()) {
				m_Display.sleep();
			}
		}
		m_Display.dispose();		
	}
	
}
