package eaters;

import org.eclipse.swt.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;

import utilities.*;

public class SimButtons extends Composite implements SimulationListener {
	EatersSimulation m_Simulation;
	Button m_RunButton;
	Button m_StopButton;
	Button m_StepButton;
	Button m_ResetButton;
	Logger m_Logger = Logger.logger;
	
	public SimButtons(Composite parent, EatersSimulation simulation) {
		super(parent, SWT.NONE);
		
		m_Simulation = simulation;
		
		setLayout(new FillLayout());
		
		m_RunButton = new Button(this, SWT.PUSH);
		m_RunButton.setText("Run");
		m_RunButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				m_Simulation.startSimulation();
			}
		});
		
		m_StopButton = new Button(this, SWT.PUSH);
		m_StopButton.setText("Stop");
		m_StopButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				m_Simulation.interactiveStop();
				m_Simulation.stopSimulation();
			}
		});
		
		m_StepButton = new Button(this, SWT.PUSH);
		m_StepButton.setText("Step");
		m_StepButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				m_Simulation.stepSimulation();
			}
		});
		
		m_ResetButton = new Button(this, SWT.PUSH);
		m_ResetButton.setText("Reset");
		m_ResetButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				m_Simulation.resetSimulation();
			}
		});
		
		m_Simulation.addSimulationListener(this);
		updateButtons();
	}
	
	void updateButtons() {
		boolean running = m_Simulation.isRunning();
		boolean done = (m_Simulation.getWorld().getFoodCount() == 0);
		
		if (!running) {
			m_Logger.log("Stop disabled.");
		}
		
        m_RunButton.setEnabled(!running && !done);
        m_StopButton.setEnabled(running);
        m_ResetButton.setEnabled(!running);
        m_StepButton.setEnabled(!running && !done);
	}

	public void simulationEventHandler(int type, Object object) {
		if (type == SimulationListener.kStartEvent || type == SimulationListener.kStopEvent || type == SimulationListener.kNewWorldEvent) {
			this.getDisplay().asyncExec(new Runnable() { 
				public void run () { 
					updateButtons();
				} 
			});
		}
	}
}
