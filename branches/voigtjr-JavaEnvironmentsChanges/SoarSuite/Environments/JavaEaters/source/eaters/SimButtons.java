package eaters;

import org.eclipse.swt.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;

public class SimButtons extends Composite {
	EatersSimulation m_Simulation;
	
	public SimButtons(Composite parent, EatersSimulation simulation) {
		super(parent, SWT.NONE);
		
		m_Simulation = simulation;
		
		setLayout(new FillLayout());
		
		final Button runButton = new Button(this, SWT.PUSH);
		runButton.setText("Run");
		runButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				m_Simulation.startSimulation();
			}
		});
		
		final Button stopButton = new Button(this, SWT.PUSH);
		stopButton.setText("Stop");
		stopButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				m_Simulation.stopSimulation();
			}
		});
		
		final Button stepButton = new Button(this, SWT.PUSH);
		stepButton.setText("Step");
		stepButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				m_Simulation.stepSimulation();
			}
		});
		
		final Button resetButton = new Button(this, SWT.PUSH);
		resetButton.setText("Reset");
		resetButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				m_Simulation.resetSimulation();
			}
		});
	}
}
