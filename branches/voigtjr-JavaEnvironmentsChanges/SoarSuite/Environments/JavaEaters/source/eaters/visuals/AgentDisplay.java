package eaters.visuals;

import org.eclipse.swt.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;

import eaters.Eater;
import eaters.EatersSimulation;
import eaters.SimulationListener;

import utilities.*;

public class AgentDisplay extends Composite implements SimulationListener {
	public static final int kAgentMapCellSize = 20;
	public static final int kListWidth = 75;
	public static final int kListHeight = 75;
	
	Logger m_Logger = Logger.logger;
	EatersSimulation m_Simulation;
	List m_AgentList;
	VisualWorld m_AgentWorld;
	Eater m_SelectedEater;
	Eater[] m_Eaters;
	Composite m_AgentButtons;
	Button m_NewAgentButton;
	Button m_DestroyAgentButton;

	public AgentDisplay(Composite parent, EatersSimulation simulation) {
		super(parent, SWT.BORDER);
		m_Simulation = simulation;
		
		setLayout(new RowLayout(SWT.VERTICAL));

		m_AgentButtons = new Composite(this, SWT.NONE);
		m_AgentButtons.setLayout(new FillLayout());
		
		m_NewAgentButton = new Button(m_AgentButtons, SWT.PUSH);
		m_NewAgentButton.setText("New");
		m_NewAgentButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
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
				deselect();
			}
		});
				
		m_AgentList = new List(this, SWT.SINGLE);
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
			}
		});
		updateEaterList();
		
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
		String[] names = new String[m_Eaters.length];
		for (int i = 0; i < m_Eaters.length; ++i) {
			names[i] = m_Eaters[i].getName();
		}
		m_AgentList.setItems(names);
		this.layout();
	}
	
	public void simulationEventHandler(int type, Object object) {
		if (isDisposed()) {
			return;
		}
		
		if (type == SimulationListener.kAgentCreatedEvent || type == SimulationListener.kAgentDestroyedEvent) {
			deselect();
			updateEaterList();
		} else if (type == SimulationListener.kUpdateEvent || type == SimulationListener.kNewWorldEvent) {
			if (m_SelectedEater != null) {
				m_AgentWorld.setAgentLocation(m_SelectedEater.getLocation());
			}
		}
	}
}
