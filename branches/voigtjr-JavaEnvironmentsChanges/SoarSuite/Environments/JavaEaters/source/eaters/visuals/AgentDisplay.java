package eaters.visuals;

import org.eclipse.swt.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.graphics.*;

import eaters.Eater;
import eaters.EatersSimulation;
import eaters.SimulationListener;

import utilities.*;

public class AgentDisplay extends Composite implements SimulationListener {
	public static final int kAgentMapCellSize = 20;
	public static final int kListWidth = 75;
	
	Logger m_Logger = Logger.logger;
	EatersSimulation m_Simulation;
	List m_AgentList;
	VisualWorld m_AgentWorld;
	Eater m_SelectedEater;
	Eater[] m_Eaters;
	Composite m_AgentButtons;
	Button m_NewAgentButton;
	Button m_DestroyAgentButton;

	public AgentDisplay(final Composite parent, EatersSimulation simulation) {
		super(parent, SWT.BORDER);
		m_Simulation = simulation;
		
		setLayout(new RowLayout(SWT.VERTICAL));

		m_AgentButtons = new Composite(this, SWT.NONE);
		m_AgentButtons.setLayout(new FillLayout());
		
		m_NewAgentButton = new Button(m_AgentButtons, SWT.PUSH);
		m_NewAgentButton.setText("New");
		m_NewAgentButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				new CreateEaterDialog(parent.getShell(), m_Simulation).open();
			}
		});
		
		m_DestroyAgentButton = new Button(m_AgentButtons, SWT.PUSH);
		m_DestroyAgentButton.setText("Destroy");
		m_DestroyAgentButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				if (m_SelectedEater == null) {
					return;
				}
				m_Simulation.getWorld().destroyEater(m_SelectedEater);
			}
		});
				
		m_AgentList = new List(this, SWT.SINGLE);
		RowData rd = new RowData();
		FontData fd = getFont().getFontData()[0];
		rd.width = kListWidth;
		rd.height = fd.getHeight() * EatersSimulation.kMaxEaters;
		m_AgentList.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				if (m_Eaters == null) {
					return;
				}
				for (int i = 0; i < m_Eaters.length; ++i) {
					String selected = m_AgentList.getSelection()[0];
					if (m_Eaters[i].getName().equals(selected)) {
						m_SelectedEater = m_Eaters[i];
						m_AgentWorld.setAgentLocation(m_SelectedEater.getLocation());
						m_AgentWorld.enable();
						m_AgentWorld.redraw();
					}
				}
				updateButtons();
			}
		});
		updateEaterList();
		updateButtons();
		
		m_AgentWorld = new VisualWorld(AgentDisplay.this, SWT.BORDER, m_Simulation, kAgentMapCellSize, true);
		m_AgentWorld.disable();

		setLayoutData(new RowData(kListWidth + m_AgentWorld.getMiniWidth() + 4, m_AgentWorld.getMiniHeight() + 4 + 75));
		m_Simulation.addSimulationListener(this);
	}
	
	void deselect() {
		m_AgentList.deselectAll();
		m_SelectedEater = null;
		m_AgentWorld.disable();
		m_AgentWorld.redraw();
	}
	
	void updateEaterList() {
		m_Eaters = m_Simulation.getWorld().getEaters();
		if (m_Eaters == null) {
			m_AgentList.removeAll();
			return;
		}
		String[] names = new String[m_Eaters.length];
		for (int i = 0; i < m_Eaters.length; ++i) {
			names[i] = m_Eaters[i].getName();
		}
		m_AgentList.setItems(names);
		this.layout();
	}
	
	void updateButtons() {
		boolean running = m_Simulation.isRunning();
		boolean agentsFull = false;
		boolean noAgents = false;
		if (m_Eaters != null) {
			agentsFull = (m_Eaters.length == EatersSimulation.kMaxEaters);
		} else {
			noAgents = true;
		}
		boolean selectedEater = (m_SelectedEater != null);
		
		m_NewAgentButton.setEnabled(!running && !agentsFull);
		m_DestroyAgentButton.setEnabled(!running && !noAgents && selectedEater);
 	}

	public void simulationEventHandler(int type, Object object) {
		if (isDisposed()) {
			return;
		}
		
		if (type == SimulationListener.kAgentCreatedEvent || type == SimulationListener.kAgentDestroyedEvent) {
			deselect();
			updateEaterList();
			updateButtons();
		} 
		
		if (type == SimulationListener.kUpdateEvent || type == SimulationListener.kNewWorldEvent) {
			if (m_SelectedEater != null) {
				m_AgentWorld.setAgentLocation(m_SelectedEater.getLocation());
			}
		}
		
		if (type == SimulationListener.kStartEvent || type == SimulationListener.kStopEvent || type == SimulationListener.kNewWorldEvent) {
			this.getDisplay().asyncExec(new Runnable() { 
				public void run () { 
					updateButtons();
				} 
			});
		}
	}
}
