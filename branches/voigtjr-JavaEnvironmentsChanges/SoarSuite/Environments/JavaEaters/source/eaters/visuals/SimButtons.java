package eaters.visuals;

import org.eclipse.swt.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;

import eaters.*;

import utilities.*;

public class SimButtons extends Composite {
	EatersSimulation m_Simulation;
	Button m_RunButton;
	Button m_StopButton;
	Button m_StepButton;
	Button m_ResetButton;
	Button m_ContinuousButton;
	Logger m_Logger = Logger.logger;
	
	public SimButtons(Composite parent, EatersSimulation simulation) {
		super(parent, SWT.NONE);
		
		m_Simulation = simulation;

		setLayout(new RowLayout(SWT.VERTICAL));
		
		Composite c = new Composite(this, SWT.NONE);
		c.setLayout(new FillLayout());
		
		m_RunButton = new Button(c, SWT.PUSH);
		m_RunButton.setText("Run");
		m_RunButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				m_Simulation.startSimulation();
			}
		});
		
		m_StopButton = new Button(c, SWT.PUSH);
		m_StopButton.setText("Stop");
		m_StopButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				m_Simulation.setContinuousMode(false);
				m_Simulation.stopSimulation();
			}
		});
		
		m_StepButton = new Button(c, SWT.PUSH);
		m_StepButton.setText("Step");
		m_StepButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				m_Simulation.stepSimulation();
			}
		});
		
		m_ResetButton = new Button(c, SWT.PUSH);
		m_ResetButton.setText("Reset");
		m_ResetButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				m_Simulation.resetSimulation();
			}
		});
		
		m_ContinuousButton = new Button(this, SWT.CHECK);
		m_ContinuousButton.setText("Continuous mode");
		m_ContinuousButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				m_Simulation.setContinuousMode(m_ContinuousButton.getSelection());
			}
		});		
		
		updateButtons();
	}
	
	void updateButtons() {
		boolean running = m_Simulation.isRunning();
		boolean done = (m_Simulation.getWorld().getFoodCount() == 0);
		boolean eaters = (m_Simulation.getWorld().getEaters() != null);
		boolean continuous = m_Simulation.getContinuousMode();
		
        m_RunButton.setEnabled(!running && !done && eaters);
        m_StopButton.setEnabled(running);
        m_ResetButton.setEnabled(!running);
        m_StepButton.setEnabled(!running && !done && eaters);
        m_ContinuousButton.setSelection(continuous);
	}
}
