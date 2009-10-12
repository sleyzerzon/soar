package edu.umich.soar.room;

import java.awt.GridBagLayout;
import javax.swing.JPanel;
import javax.swing.JRadioButton;
import java.awt.GridBagConstraints;
import java.util.prefs.Preferences;

import javax.swing.BorderFactory;
import javax.swing.ButtonGroup;
import javax.swing.JSlider;
import javax.swing.JLabel;
import javax.swing.JButton;
import javax.swing.JTextField;
import javax.swing.SwingConstants;

import edu.umich.soar.room.GoAction.GoProvider;
import edu.umich.soar.room.core.Simulation;

public class SimulationControlPanel extends JPanel implements GoProvider {

	private static final long serialVersionUID = 1L;
	private JRadioButton jSyncRadioButton = null;
	private JRadioButton jAsyncRadioButton = null;
	private JPanel jRunPanel = null;
	private JRadioButton jForeverRadioButton = null;
	private JRadioButton jTicksRadioButton = null;
	private JTextField jQuantityTextField = null;
	private JButton jRunButton = null;
	private JButton jStopButton = null;
	private JButton jResetButton = null;
	private JPanel jTypePanel = null;
	private JPanel jDelayPanel = null;
	private JSlider jDelaySlider = null;
	private JLabel jMsecPerTickLabel = null;
	private JPanel jInfoPanel = null;
	private JLabel jCountsLabel = null;

	private ButtonGroup typeButtonGroup = new ButtonGroup();  
	private ButtonGroup runButtonGroup = new ButtonGroup(); 
	
	private final String KEY_RUN_QTY = "runQty";
	private final String KEY_RUN_FOREVER = "runForever"; 
	private final Simulation sim;
	private final Preferences pref;
	private final ActionManager am;
	
	/**
	 * This is the default constructor
	 */
	public SimulationControlPanel(Adaptable app, Preferences pref) {
		super();
		this.sim = Adaptables.adapt(app, Simulation.class);
        this.am = Adaptables.adapt(app, ActionManager.class);
		this.pref = pref;
		
		initialize();

        int runQty = pref.getInt(KEY_RUN_QTY, 1);
    	boolean runForever = pref.getBoolean(KEY_RUN_FOREVER, true);

		jAsyncRadioButton.setSelected(this.sim.getCogArch().isAsync());
		jSyncRadioButton.setSelected(!this.sim.getCogArch().isAsync());
		
		jQuantityTextField.setText(Integer.toString(runQty));
		jForeverRadioButton.setSelected(runForever);
		jTicksRadioButton.setSelected(!runForever);
		
		jDelaySlider.setValue(100);

		foreverChanged();
		delayChanged();
		updateCounts();
	}

	/**
	 * This method initializes this
	 * 
	 * @return void
	 */
	private void initialize() {
		GridBagConstraints gridBagConstraints10 = new GridBagConstraints();
		gridBagConstraints10.gridx = 0;
		gridBagConstraints10.fill = GridBagConstraints.HORIZONTAL;
		gridBagConstraints10.anchor = GridBagConstraints.NORTHWEST;
		gridBagConstraints10.gridy = 3;
		GridBagConstraints gridBagConstraints51 = new GridBagConstraints();
		gridBagConstraints51.gridx = 0;
		gridBagConstraints51.fill = GridBagConstraints.HORIZONTAL;
		gridBagConstraints51.anchor = GridBagConstraints.NORTHWEST;
		gridBagConstraints51.gridy = 0;
		GridBagConstraints gridBagConstraints2 = new GridBagConstraints();
		gridBagConstraints2.gridx = 0;
		gridBagConstraints2.gridheight = 2;
		gridBagConstraints2.fill = GridBagConstraints.HORIZONTAL;
		gridBagConstraints2.anchor = GridBagConstraints.NORTHWEST;
		gridBagConstraints2.gridy = 1;
		GridBagConstraints gridBagConstraints52 = new GridBagConstraints();
		gridBagConstraints52.gridx = 0;
		gridBagConstraints52.gridheight = 2;
		gridBagConstraints52.fill = GridBagConstraints.HORIZONTAL;
		gridBagConstraints52.anchor = GridBagConstraints.NORTHWEST;
		gridBagConstraints52.gridy = 4;
		this.setSize(242, 208);
		this.setLayout(new GridBagLayout());
		this.add(getJRunPanel(), gridBagConstraints2);
		this.add(getJIntegrationPanel(), gridBagConstraints51);
		this.add(getJDelayPanel(), gridBagConstraints10);
		this.add(getJInfoPanel(), gridBagConstraints52);
	}

	/**
	 * This method initializes jSyncRadioButton	
	 * 	
	 * @return javax.swing.JRadioButton	
	 */
	private JRadioButton getJSyncRadioButton() {
		if (jSyncRadioButton == null) {
			jSyncRadioButton = new JRadioButton();
			jSyncRadioButton.setText("Synchronous");
			jSyncRadioButton.addChangeListener(new javax.swing.event.ChangeListener() {
				public void stateChanged(javax.swing.event.ChangeEvent e) {
					typeChanged();
				}
			});
			typeButtonGroup.add(jSyncRadioButton);
		}
		return jSyncRadioButton;
	}

	/**
	 * This method initializes jAsyncRadioButton	
	 * 	
	 * @return javax.swing.JRadioButton	
	 */
	private JRadioButton getJAsyncRadioButton() {
		if (jAsyncRadioButton == null) {
			jAsyncRadioButton = new JRadioButton();
			jAsyncRadioButton.setText("Asynchronous");
			jAsyncRadioButton.addChangeListener(new javax.swing.event.ChangeListener() {
				public void stateChanged(javax.swing.event.ChangeEvent e) {
					typeChanged();
				}
			});
			typeButtonGroup.add(jAsyncRadioButton);
		}
		return jAsyncRadioButton;
	}
	
	private void typeChanged() {
		sim.getCogArch().setAsync(jAsyncRadioButton.isSelected());
	}

	/**
	 * This method initializes jRunPanel	
	 * 	
	 * @return javax.swing.JPanel	
	 */
	private JPanel getJRunPanel() {
		if (jRunPanel == null) {
			GridBagConstraints gridBagConstraints9 = new GridBagConstraints();
			gridBagConstraints9.gridx = 3;
			gridBagConstraints9.fill = GridBagConstraints.HORIZONTAL;
			gridBagConstraints9.gridy = 2;
			GridBagConstraints gridBagConstraints8 = new GridBagConstraints();
			gridBagConstraints8.gridx = 3;
			gridBagConstraints8.fill = GridBagConstraints.BOTH;
			gridBagConstraints8.gridheight = 2;
			gridBagConstraints8.gridy = 0;
			GridBagConstraints gridBagConstraints7 = new GridBagConstraints();
			gridBagConstraints7.gridx = 2;
			gridBagConstraints7.gridheight = 3;
			gridBagConstraints7.fill = GridBagConstraints.BOTH;
			gridBagConstraints7.gridy = 0;
			GridBagConstraints gridBagConstraints6 = new GridBagConstraints();
			gridBagConstraints6.fill = GridBagConstraints.BOTH;
			gridBagConstraints6.gridy = 1;
			gridBagConstraints6.weightx = 1.0;
			gridBagConstraints6.gridheight = 2;
			gridBagConstraints6.gridx = 0;
			GridBagConstraints gridBagConstraints5 = new GridBagConstraints();
			gridBagConstraints5.gridx = 1;
			gridBagConstraints5.anchor = GridBagConstraints.WEST;
			gridBagConstraints5.gridy = 2;
			GridBagConstraints gridBagConstraints3 = new GridBagConstraints();
			gridBagConstraints3.gridx = 1;
			gridBagConstraints3.anchor = GridBagConstraints.WEST;
			gridBagConstraints3.gridy = 0;
			jRunPanel = new JPanel();
			jRunPanel.setLayout(new GridBagLayout());
			jRunPanel.setBorder(BorderFactory.createTitledBorder("Run"));
			jRunPanel.add(getJForeverRadioButton(), gridBagConstraints3);
			jRunPanel.add(getJTicksRadioButton(), gridBagConstraints5);
			jRunPanel.add(getJQuantityTextField(), gridBagConstraints6);
			jRunPanel.add(getJGoButton(), gridBagConstraints7);
			jRunPanel.add(getJStopButton(), gridBagConstraints8);
			jRunPanel.add(getJResetButton(), gridBagConstraints9);
		}
		return jRunPanel;
	}

	/**
	 * This method initializes jForeverRadioButton	
	 * 	
	 * @return javax.swing.JRadioButton	
	 */
	private JRadioButton getJForeverRadioButton() {
		if (jForeverRadioButton == null) {
			jForeverRadioButton = new JRadioButton();
			jForeverRadioButton.setText("Forever");
			jForeverRadioButton.setSelected(true);
			runButtonGroup.add(jForeverRadioButton);
			jForeverRadioButton.addChangeListener(new javax.swing.event.ChangeListener() {
				public void stateChanged(javax.swing.event.ChangeEvent e) {
					foreverChanged();
				}
			});
		}
		return jForeverRadioButton;
	}
	
	private void foreverChanged() {
		jQuantityTextField.setEnabled(!jForeverRadioButton.isSelected());
	}

	/**
	 * This method initializes jTicksRadioButton	
	 * 	
	 * @return javax.swing.JRadioButton	
	 */
	private JRadioButton getJTicksRadioButton() {
		if (jTicksRadioButton == null) {
			jTicksRadioButton = new JRadioButton();
			jTicksRadioButton.setText("Ticks");
			runButtonGroup.add(jTicksRadioButton);
			jTicksRadioButton.addChangeListener(new javax.swing.event.ChangeListener() {
				public void stateChanged(javax.swing.event.ChangeEvent e) {
					jQuantityTextField.setEnabled(true);
				}
			});
		}
		return jTicksRadioButton;
	}

	/**
	 * This method initializes jQuantityTextField	
	 * 	
	 * @return javax.swing.JTextField	
	 */
	private JTextField getJQuantityTextField() {
		if (jQuantityTextField == null) {
			jQuantityTextField = new JTextField();
			jQuantityTextField.setText("1");
			jQuantityTextField.setColumns(4);
			jQuantityTextField.addKeyListener(new java.awt.event.KeyAdapter() {
				public void keyTyped(java.awt.event.KeyEvent e) {
					if (!Character.isDigit(e.getKeyChar())) {
						e.consume();
					}
				}
			});
		}
		return jQuantityTextField;
	}

	/**
	 * This method initializes jGoButton	
	 * 	
	 * @return javax.swing.JButton	
	 */
	private JButton getJGoButton() {
		if (jRunButton == null) {
			jRunButton = new JButton();
			GoAction ga = am.getAction(GoAction.class);
			ga.setGoProvider(this);
			jRunButton.setAction(ga);
			//jRunButton.setText("Go");
		}
		return jRunButton;
	}

	/**
	 * This method initializes jStopButton	
	 * 	
	 * @return javax.swing.JButton	
	 */
	private JButton getJStopButton() {
		if (jStopButton == null) {
			jStopButton = new JButton();
			jStopButton.setAction(am.getAction(StopAction.class));
			//jStopButton.setText("Stop");
		}
		return jStopButton;
	}

	/**
	 * This method initializes jResetButton	
	 * 	
	 * @return javax.swing.JButton	
	 */
	private JButton getJResetButton() {
		if (jResetButton == null) {
			jResetButton = new JButton();
			jResetButton.setAction(am.getAction(ResetAction.class));
			//jResetButton.setText("Reset");
		}
		return jResetButton;
	}

	/**
	 * This method initializes jIntegrationPanel	
	 * 	
	 * @return javax.swing.JPanel	
	 */
	private JPanel getJIntegrationPanel() {
		if (jTypePanel == null) {
			GridBagConstraints gridBagConstraints1 = new GridBagConstraints();
			gridBagConstraints1.gridx = 0;
			gridBagConstraints1.gridy = 0;
			GridBagConstraints gridBagConstraints = new GridBagConstraints();
			gridBagConstraints.gridx = 1;
			gridBagConstraints.gridy = -1;
			jTypePanel = new JPanel();
			jTypePanel.setLayout(new GridBagLayout());
			jTypePanel.setBorder(BorderFactory.createTitledBorder("Type"));
			jTypePanel.add(getJSyncRadioButton(), gridBagConstraints);
			jTypePanel.add(getJAsyncRadioButton(), gridBagConstraints1);
		}
		return jTypePanel;
	}

	private JPanel getJInfoPanel() {
		if (jInfoPanel == null) {
			GridBagConstraints gridBagConstraints1 = new GridBagConstraints();
			gridBagConstraints1.gridx = 0;
			gridBagConstraints1.gridy = 0;
			jInfoPanel = new JPanel();
			jInfoPanel.setLayout(new GridBagLayout());
			jInfoPanel.setBorder(BorderFactory.createTitledBorder("Info"));
			jCountsLabel = new JLabel();
			jCountsLabel.setHorizontalAlignment(SwingConstants.LEFT);
			jCountsLabel.setVerticalAlignment(SwingConstants.TOP);
			jInfoPanel.add(jCountsLabel, gridBagConstraints1);
		}
		return jInfoPanel;
	}

	/**
	 * This method initializes jDelayPanel	
	 * 	
	 * @return javax.swing.JPanel	
	 */
	private JPanel getJDelayPanel() {
		if (jDelayPanel == null) {
			GridBagConstraints gridBagConstraints12 = new GridBagConstraints();
			gridBagConstraints12.gridx = 0;
			gridBagConstraints12.gridy = 1;
			jMsecPerTickLabel = new JLabel();
			GridBagConstraints gridBagConstraints11 = new GridBagConstraints();
			gridBagConstraints11.fill = GridBagConstraints.VERTICAL;
			gridBagConstraints11.gridy = 0;
			gridBagConstraints11.weightx = 1.0;
			gridBagConstraints11.gridx = 0;
			jDelayPanel = new JPanel();
			jDelayPanel.setLayout(new GridBagLayout());
			jDelayPanel.setBorder(BorderFactory.createTitledBorder("Simulation Speed"));

			jDelayPanel.add(getJDelaySlider(), gridBagConstraints11);
			jDelayPanel.add(jMsecPerTickLabel, gridBagConstraints12);
		}
		return jDelayPanel;
	}

	/**
	 * This method initializes jDelaySlider	
	 * 	
	 * @return javax.swing.JSlider	
	 */
	private JSlider getJDelaySlider() {
		if (jDelaySlider == null) {
			jDelaySlider = new JSlider();
			jDelaySlider.setValue(100);
			jDelaySlider.setExtent(25);
			jDelaySlider.setMaximum(500);
			jDelaySlider.setMinimum(0);
			jDelaySlider.addChangeListener(new javax.swing.event.ChangeListener() {
				public void stateChanged(javax.swing.event.ChangeEvent e) {
					delayChanged();
				}
			});
		}
		return jDelaySlider;
	}
	
	@Override
	public double getTimeScale() {
		return jDelaySlider.getValue() / 100.0;
	}
	
	private void delayChanged() {
		double timeScale = getTimeScale();
		jMsecPerTickLabel.setText(String.format("%1.1f", timeScale) + "x real");
	}

	public void dispose() {
		try {
			pref.putInt(KEY_RUN_QTY, Integer.parseInt(jQuantityTextField.getText()));
		} catch (NumberFormatException e) {
			pref.putInt(KEY_RUN_QTY, 0);
		}
		
    	pref.putBoolean(KEY_RUN_FOREVER, runButtonGroup.isSelected(jTicksRadioButton.getModel()));
	}

	@Override
	public boolean isRunForever() {
		if (runButtonGroup.isSelected(jTicksRadioButton.getModel())) {
			return false;
		}
		return true;
	}

	@Override
	public int getQuantity() {
		return Integer.valueOf(jQuantityTextField.getText());
	}

	public void updateCounts() {
		StringBuilder countsText = new StringBuilder();
		countsText.append("<html> <b>World&nbsp;Count:</b>&nbsp;");
		countsText.append(sim.getWorldCount());
		countsText.append("&nbsp;&nbsp;&nbsp;");
		countsText.append(" <br /><b>Soar&nbsp;Update&nbsp;Count:</b>&nbsp;");
		countsText.append(sim.getCogArch().getUpdateCount());
		countsText.append("&nbsp;&nbsp;&nbsp;");
		countsText.append(" </html>");
		jCountsLabel.setText(countsText.toString());
	}
} 

