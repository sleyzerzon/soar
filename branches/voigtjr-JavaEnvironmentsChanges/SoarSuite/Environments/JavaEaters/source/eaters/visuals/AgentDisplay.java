package eaters.visuals;

import org.eclipse.swt.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;

import eaters.*;

import utilities.*;

public class AgentDisplay extends Composite {
	public static final int kAgentMapCellSize = 20;
	static final int kTableHeight = 120;
	static final int kNameWidth = 75;
	static final int kScoreWidth = 40;
	
	Group m_Group;
	Logger m_Logger = Logger.logger;
	EatersSimulation m_Simulation;
	Table m_AgentTable;
	VisualWorld m_AgentWorld;
	Eater m_SelectedEater;
	TableItem[] m_Items;
	Eater[] m_Eaters;
	Composite m_AgentButtons;
	Button m_NewAgentButton;
	Button m_CloneAgentButton;
	Button m_DestroyAgentButton;

	public AgentDisplay(final Composite parent, EatersSimulation simulation) {
		super(parent, SWT.NONE);
		m_Simulation = simulation;

		setLayout(new FillLayout());
		
		m_Group = new Group(this, SWT.NONE);
		m_Group.setText("Agents");
		GridLayout gl = new GridLayout();
		gl.numColumns = 2;
		m_Group.setLayout(gl);
		
		
		GridData gd;

		m_AgentButtons = new Composite(m_Group, SWT.NONE);
		m_AgentButtons.setLayout(new FillLayout());
		gd = new GridData();
		gd.horizontalAlignment = GridData.BEGINNING;
		gd.horizontalSpan = 2;
		m_AgentButtons.setLayoutData(gd);
		
		m_NewAgentButton = new Button(m_AgentButtons, SWT.PUSH);
		m_NewAgentButton.setText("New");
		m_NewAgentButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				new CreateEaterDialog(parent.getShell(), m_Simulation).open();
			}
		});
		
		m_CloneAgentButton = new Button(m_AgentButtons, SWT.PUSH);
		m_CloneAgentButton.setText("Clone");
		m_CloneAgentButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				String color = null;
				// TODO: this probably isn't the most efficient way of doing this, but this is not a bottleneck point
				for (int i = 0; i < EatersWindowManager.kColors.length; ++i) {
					boolean notTaken = true;
					for (int j = 0; j < m_Eaters.length; ++j) {
						if (m_Eaters[j].getColorString().equalsIgnoreCase(EatersWindowManager.kColors[i])) {
							notTaken = false;
							break;
						}
					}
					if (notTaken) {
						color = EatersWindowManager.kColors[i];
						break;
					}
				}
				
				// Risking null exception here, but that should not be possible ;)
				m_Simulation.createEater(color, m_SelectedEater.getProductions(), color);
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
				
		m_AgentTable = new Table(m_Group, SWT.BORDER | SWT.FULL_SELECTION);
		gd = new GridData();
		gd.heightHint = kTableHeight;
		m_AgentTable.setLayoutData(gd);
		TableColumn tc1 = new TableColumn(m_AgentTable, SWT.CENTER);
		TableColumn tc2 = new TableColumn(m_AgentTable, SWT.CENTER);
		tc1.setText("Name");
		tc1.setWidth(kNameWidth);
		tc2.setText("Score");
		tc2.setWidth(kScoreWidth);
		m_AgentTable.setHeaderVisible(true);
		m_AgentTable.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				if (m_Eaters == null) {
					return;
				}
				for (int i = 0; i < m_Eaters.length; ++i) {
					m_SelectedEater = m_Eaters[m_AgentTable.getSelectionIndex()];
					m_AgentWorld.setAgentLocation(m_SelectedEater.getLocation());
					m_AgentWorld.enable();
					m_AgentWorld.redraw();
				}
				updateButtons();
			}
		});
		updateEaterList();
		updateButtons();
		
		m_AgentWorld = new VisualWorld(m_Group, SWT.BORDER, m_Simulation, kAgentMapCellSize);
		gd = new GridData();
		gd.heightHint = m_AgentWorld.getMiniHeight() + 4;
		gd.widthHint = m_AgentWorld.getMiniWidth() + 4;		
		m_AgentWorld.setLayoutData(gd);
		m_AgentWorld.disable();
	}
	
	void agentEvent() {
		deselect();
		updateEaterList();
		updateButtons();
	}

	void worldChangeEvent() {
		if (m_SelectedEater != null) {
			m_AgentWorld.setAgentLocation(m_SelectedEater.getLocation());
			m_AgentWorld.redraw();
		}
		
		if (m_Items != null) {
			for (int i = 0; i < m_Items.length; ++i) {
				m_Items[i].setText(1, Integer.toString(m_Eaters[i].getScore()));
			}
		}
	}
	
	void deselect() {
		m_AgentTable.deselectAll();
		m_SelectedEater = null;
		m_AgentWorld.disable();
		m_AgentWorld.redraw();
	}
	
	void updateEaterList() {
		m_Eaters = m_Simulation.getWorld().getEaters();
		m_AgentTable.removeAll();
		if (m_Eaters == null) {
			m_Items = null;
			return;
		}
		m_Items = new TableItem[m_Eaters.length];
		for (int i = 0; i < m_Eaters.length; ++i) {
			m_Items[i] = new TableItem(m_AgentTable, SWT.NONE);
			m_Items[i].setText(new String[] {m_Eaters[i].getName(), Integer.toString(m_Eaters[i].getScore())});
		}
		//this.layout();
	}
	
	void updateItemList() {
		
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
		m_CloneAgentButton.setEnabled(!running && !agentsFull && selectedEater);
		m_DestroyAgentButton.setEnabled(!running && !noAgents && selectedEater);
 	}
}
