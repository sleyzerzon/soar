package eaters;

import org.eclipse.swt.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;

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

	public AgentDisplay(Composite parent, EatersSimulation simulation) {
		super(parent, SWT.BORDER);
		m_Simulation = simulation;

		m_AgentList = new List(this, SWT.SINGLE);
		m_AgentList.setBounds(0,0,kListWidth,kListHeight);
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
		
		m_AgentWorld = new VisualWorld(AgentDisplay.this, SWT.BORDER, m_Simulation, kAgentMapCellSize);
		m_AgentWorld.setBounds(kListWidth,0, m_AgentWorld.getMiniWidth() + 4, m_AgentWorld.getMiniHeight() + 4);
		m_AgentWorld.disable();

		setLayoutData(new RowData(kListWidth + m_AgentWorld.getMiniWidth() + 4, m_AgentWorld.getMiniHeight() + 4));
		m_Simulation.addSimulationListener(this);
	}
	
	void updateEaterList() {
		m_Eaters = m_Simulation.getWorld().getEaters();
		String[] names = new String[m_Eaters.length];
		for (int i = 0; i < m_Eaters.length; ++i) {
			names[i] = m_Eaters[i].getName();
		}
		m_AgentList.setItems(names);
	}
	
	public void simulationEventHandler(int type, Object object) {
		if (type == SimulationListener.kAgentCreatedEvent || type == SimulationListener.kAgentDestroyedEvent) {
			updateEaterList();
		} else if (type == SimulationListener.kUpdateEvent) {
			if (m_SelectedEater != null) {
				m_AgentWorld.setAgentLocation(m_SelectedEater.getLocation());
			}
		}
	}
}
