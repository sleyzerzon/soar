package eaters.visuals;

import org.eclipse.swt.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;

import eaters.EatersSimulation;
import eaters.SimulationListener;

import utilities.*;

public class MapButtons extends Composite {
	EatersSimulation m_Simulation;
	Button m_ChangeMapButton;
	//Button m_RandomMapButton;
	Logger m_Logger = Logger.logger;
	
	public MapButtons(Composite parent, EatersSimulation simulation) {
		super(parent, SWT.NONE);
		
		m_Simulation = simulation;
		
		setLayout(new FillLayout());
		
		m_ChangeMapButton = new Button(this, SWT.PUSH);
		m_ChangeMapButton.setText("Change Map");
		m_ChangeMapButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				FileDialog fd = new FileDialog(MapButtons.this.getShell(), SWT.OPEN);
				fd.setText("Open");
				fd.setFilterPath(m_Simulation.getMapPath());
				String map = fd.open();
				if (map != null) {
					m_Simulation.changeMap(map);
				}
			}
		});
		
//		m_RandomMapButton = new Button(this, SWT.PUSH);
//		m_RandomMapButton.setText("Random Map");
//		m_RandomMapButton.addSelectionListener(new SelectionAdapter() {
//			public void widgetSelected(SelectionEvent e) {
//			}
//		});
		
		updateButtons();
	}
	
	void updateButtons() {
		boolean running = m_Simulation.isRunning();
		boolean done = (m_Simulation.getWorld().getFoodCount() == 0);
		boolean eaters = (m_Simulation.getWorld().getEaters() != null);
		
		m_ChangeMapButton.setEnabled(!running);
		//m_RandomMapButton.setEnabled(!running);
	}
}
