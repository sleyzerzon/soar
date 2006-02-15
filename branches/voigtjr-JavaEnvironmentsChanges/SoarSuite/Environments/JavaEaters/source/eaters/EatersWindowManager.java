package eaters;

import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Menu;
import org.eclipse.swt.widgets.MenuItem;
import org.eclipse.swt.widgets.Shell;

import utilities.Logger;

public class EatersWindowManager {
	Display m_Display;
	Shell m_Shell;
	protected Logger m_Logger = Logger.logger;

	public EatersWindowManager(EatersSimulation control) {
		m_Display = new Display();
		m_Shell = new Shell(m_Display);
		
		Menu menu = new Menu(m_Shell, SWT.BAR);
		final MenuItem file = new MenuItem(menu, SWT.CASCADE);
		file.setText("File");
		final Menu filemenu = new Menu(m_Shell, SWT.DROP_DOWN);
		file.setMenu(filemenu);
		final MenuItem exit = new MenuItem(filemenu, SWT.PUSH);
		exit.setText("Exit");
		
		m_Shell.setMenuBar(menu);		
		m_Shell.open();
		while (!m_Shell.isDisposed()) {
			if (!m_Display.readAndDispatch()) {
				m_Display.sleep();
			}
		}
		m_Display.dispose();		
	}
}
