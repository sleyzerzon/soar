package eaters.visuals;

import org.eclipse.swt.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;

import eaters.Eater;
import eaters.EatersSimulation;
import eaters.SimulationListener;

import utilities.*;

public class CreateEaterDialog extends Dialog {

	EatersSimulation m_Simulation;
	String m_Productions;
	Label m_ProductionsLabel;
	Text m_Name;
	Combo m_Color;
	Button m_CreateEater;
	
	public CreateEaterDialog(Shell parent, EatersSimulation simulation, Eater selected) {
		super(parent);
		m_Simulation = simulation;
		if (selected != null) {
			m_Productions = selected.getProductions();
		}
	}
	
	public void open() {
		Shell parent = getParent();
		final Shell dialog = new Shell(parent, SWT.DIALOG_TRIM | SWT.APPLICATION_MODAL);
		dialog.setSize(300,160);
		dialog.setText("Create Eater");
		
		GridLayout gl = new GridLayout();
		gl.numColumns = 3;
		dialog.setLayout(gl);

		GridData gd;
		
		final Label label2 = new Label(dialog, SWT.NONE);
		gd = new GridData();
		gd.horizontalAlignment = GridData.BEGINNING;
		label2.setText("Productions:");
		
		m_ProductionsLabel = new Label(dialog, SWT.NONE);
		gd = new GridData();
		gd.grabExcessHorizontalSpace = true;
		gd.widthHint = 150;
		m_ProductionsLabel.setLayoutData(gd);
		if (m_Productions == null) {
			m_ProductionsLabel.setText("<choose productions>");
		} else {
			m_ProductionsLabel.setText(m_Productions.substring(m_Productions.lastIndexOf(System.getProperty("file.separator")) + 1));
		}

		final Button productionsBrowse = new Button(dialog, SWT.PUSH);
		gd = new GridData();
		gd.horizontalAlignment = GridData.BEGINNING;
		productionsBrowse.setLayoutData(gd);
		productionsBrowse.setText("...");
		productionsBrowse.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				FileDialog fd = new FileDialog(dialog, SWT.OPEN);
				fd.setText("Open");
				fd.setFilterPath(m_Simulation.getAgentPath());
				String productions = fd.open();
				if (productions != null) {
					m_Productions = productions;
					m_ProductionsLabel.setText(m_Productions.substring(m_Productions.lastIndexOf(System.getProperty("file.separator")) + 1));
				}
				updateButtons();
			}
		});
		
		final Label label4 = new Label(dialog, SWT.NONE);
		gd = new GridData();
		gd.horizontalAlignment = GridData.BEGINNING;
		label4.setLayoutData(gd);
		label4.setText("Color:");
		
		m_Color = new Combo(dialog, SWT.NONE);
		gd = new GridData();
		gd.horizontalSpan = 2;
		m_Color.setLayoutData(gd);
		m_Color.setItems(EatersWindowManager.kColors);
		// remove taken colors
		Eater[] eaters = m_Simulation.getWorld().getEaters();
		if (eaters != null) {
			for (int i = 0; i < eaters.length; ++i) {
				m_Color.remove(eaters[i].getColorString());
			}
		}
		m_Color.select(0);
		m_Color.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				m_Name.setText(m_Color.getText());
			}
		});
		
		final Label label1 = new Label(dialog, SWT.NONE);
		gd = new GridData();
		gd.horizontalAlignment = GridData.BEGINNING;
		label1.setLayoutData(gd);
		label1.setText("Name:");
		
		m_Name = new Text(dialog, SWT.SINGLE | SWT.BORDER);
		gd = new GridData();
		gd.horizontalSpan = 2;
		gd.widthHint = 150;
		gd.grabExcessHorizontalSpace = true;
		m_Name.setLayoutData(gd);
		m_Name.setText(m_Color.getText());
		m_Name.addKeyListener(new KeyAdapter() {
			public void keyReleased(KeyEvent e) {
				updateButtons();
				if (Character.isWhitespace(e.character)) {
					e.doit = false;
				}
			}
		});
		
		m_CreateEater = new Button(dialog, SWT.PUSH);
		gd = new GridData();
		gd.horizontalAlignment = GridData.END;
		gd.horizontalSpan = 3;
		m_CreateEater.setLayoutData(gd);
		m_CreateEater.setText("Create Eater");
		m_CreateEater.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				m_CreateEater.setEnabled(false);
				m_Simulation.createEater(m_Name.getText(), m_Productions, m_Color.getText());
				dialog.dispose();
			}
		});
		
		updateButtons();
		
		dialog.open();
		Display display = parent.getDisplay();
		while (!dialog.isDisposed()) {
			if (!display.readAndDispatch()) {
				display.sleep();
			}
		}
	}
	
	void updateButtons() {
		boolean productions = (m_Productions != null);
		boolean name = false;
	
		String nameText = m_Name.getText();
		if (nameText != null) {
			if (nameText.length() > 0) {
				name = true;
				for (int i = 0; i < nameText.length(); ++i) {
					if (Character.isWhitespace(nameText.charAt(i))) {
						name = false;
					}
				}			
			}
		}
		
		m_CreateEater.setEnabled(productions && name);
	}
}
