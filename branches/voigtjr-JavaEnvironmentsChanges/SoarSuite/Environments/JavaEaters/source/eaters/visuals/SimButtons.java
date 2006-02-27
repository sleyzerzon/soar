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
//	Text m_RunsText;
	Logger m_Logger = Logger.logger;
	
	public SimButtons(Composite parent, EatersSimulation simulation) {
		super(parent, SWT.NONE);
		
		m_Simulation = simulation;

		GridLayout gl = new GridLayout();
		gl.numColumns = 4;
		setLayout(gl);
				
		GridData gd;
		
		m_RunButton = new Button(this, SWT.PUSH);
		gd = new GridData();
		m_RunButton.setLayoutData(gd);
		m_RunButton.setText("Run");
		m_RunButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
//				int input = 0;
//				try {
//					input = Integer.valueOf(m_RunsText.getText()).intValue();
//				} catch (NumberFormatException exception) {
//					if (m_RunsText.getText().equalsIgnoreCase("forever")) {
//						m_RunsText.setText("-1");
//						input = -1;
//					}
//					m_RunsText.setText("0");
//				}
//				if (input < 0) {
//					input = -1;
//				}
//				m_Simulation.setRuns(input);
				m_Simulation.startSimulation(true);
			}
		});
		
		m_StopButton = new Button(this, SWT.PUSH);
		gd = new GridData();
		m_StopButton.setLayoutData(gd);
		m_StopButton.setText("Stop");
		m_StopButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				m_Simulation.setRuns(0);
				m_Simulation.stopSimulation();
			}
		});
		
		m_StepButton = new Button(this, SWT.PUSH);
		gd = new GridData();
		m_StepButton.setLayoutData(gd);
		m_StepButton.setText("Step");
		m_StepButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				m_Simulation.stepSimulation();
			}
		});
		
		m_ResetButton = new Button(this, SWT.PUSH);
		gd = new GridData();
		m_ResetButton.setLayoutData(gd);
		m_ResetButton.setText("Reset");
		m_ResetButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				m_Simulation.resetSimulation();
			}
		});
		
//		Label runsLabel = new Label(this, SWT.NONE);
//		gd = new GridData();
//		gd.horizontalSpan = 2;
//		gd.horizontalAlignment = SWT.END;
//		runsLabel.setLayoutData(gd);
//		runsLabel.setText("Runs:");
//		
//		m_RunsText = new Text(this, SWT.BORDER);
//		gd = new GridData();
//		gd.horizontalSpan = 2;
//		gd.horizontalSpan = 2;
//		gd.widthHint = 67;
//		m_RunsText.setLayoutData(gd);
//		m_RunsText.setTextLimit(10);
		
		updateButtons();
	}
	
	void updateButtons() {
		boolean running = m_Simulation.isRunning();
		boolean done = (m_Simulation.getWorld().getFoodCount() == 0);
		boolean eaters = (m_Simulation.getWorld().getEaters() != null);
		
        m_RunButton.setEnabled(!running && !done && eaters);
        m_StopButton.setEnabled(running);
        m_ResetButton.setEnabled(!running);
        m_StepButton.setEnabled(!running && !done && eaters);
//        m_RunsText.setText(Integer.toString(m_Simulation.getRuns()));
	}
}
