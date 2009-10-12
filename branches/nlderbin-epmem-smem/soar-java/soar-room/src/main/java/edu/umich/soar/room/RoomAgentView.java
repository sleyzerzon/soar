package edu.umich.soar.room;

import java.awt.BorderLayout;
import java.util.Arrays;
import java.util.List;

import javax.swing.BorderFactory;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextField;
import javax.swing.SwingUtilities;
import javax.swing.event.ListSelectionEvent;

import org.flexdock.docking.DockingConstants;
import org.jdesktop.swingx.JXTable;
import org.jdesktop.swingx.decorator.HighlighterFactory;

import edu.umich.soar.robot.SendMessages;
import edu.umich.soar.robot.SendMessagesInterface;
import edu.umich.soar.room.core.Simulation;
import edu.umich.soar.room.map.Robot;
import edu.umich.soar.room.selection.SelectionListener;
import edu.umich.soar.room.selection.SelectionManager;
import edu.umich.soar.room.selection.SelectionProvider;
import edu.umich.soar.room.selection.TableSelectionProvider;

public class RoomAgentView extends AbstractAgentView implements SelectionListener {
	
	private static final long serialVersionUID = 88497381529040370L;
	
	private final Simulation sim;
	private final RobotTableModel model;
    private final JXTable table;
	private final JLabel properties = new JLabel();
	private final TableSelectionProvider selectionProvider;
	private final JTextField commInput = new JTextField();
	private final SendMessagesInterface sendMessages;
	
    public RoomAgentView(Adaptable app) {
        super("roomAgentView", "Agent View");
        addAction(DockingConstants.PIN_ACTION);

        this.sim = Adaptables.adapt(app, Simulation.class);
        Adaptables.adapt(app, Application.class).getSelectionManager().addListener(this);

        this.sendMessages = Adaptables.adapt(app, SendMessagesInterface.class);
        
        this.model = new RobotTableModel(this.sim);
        this.model.initialize();
        this.table = new JXTable(this.model);
        this.table.setVisibleRowCount(2);
            
        this.table.setShowGrid(false);
        this.table.setHighlighters(HighlighterFactory.createAlternateStriping());
        this.table.setColumnControlVisible(true);

        this.selectionProvider = new TableSelectionProvider(this.table) {

            @Override
            public void valueChanged(ListSelectionEvent e)
            {
                super.valueChanged(e);
                updateRobotProperties();
            }};
        
        final JPanel p = new JPanel(new BorderLayout());
        final JScrollPane pane = new JScrollPane(table);
        p.add(pane, BorderLayout.NORTH);

        properties.setBorder(BorderFactory.createTitledBorder("Robot Properties"));
        properties.setHorizontalAlignment(LEFT);
        properties.setVerticalAlignment(TOP);
        p.add(properties, BorderLayout.CENTER);

        final JPanel commPanel = new JPanel(new BorderLayout());
        commPanel.setBorder(BorderFactory.createTitledBorder("Direct Communication"));
        commInput.addKeyListener(new java.awt.event.KeyAdapter() {
			public void keyTyped(java.awt.event.KeyEvent e) {
				if (e.getKeyChar() == '\n') {
					e.consume();
					sendMessage();
				}
			}
		});

        commPanel.add(commInput, BorderLayout.CENTER);
        final JButton commSend = new JButton("Send");
        commSend.addActionListener(new java.awt.event.ActionListener() {
			public void actionPerformed(java.awt.event.ActionEvent e) {
				sendMessage();
			}
        });
        commPanel.add(commSend, BorderLayout.EAST);

        p.add(commPanel, BorderLayout.SOUTH);
        setContentPane(p);
    }
    
    private void sendMessage() {
    	List<String> tokens = SendMessages.toTokens(commInput.getText());
    	if (!tokens.isEmpty()) {
    		sendMessages.sendMessage("operator", null, tokens);
    		commInput.setText("");
    	}
    }
    
    private void updateRobotProperties() {
    	final Robot robot = (Robot)selectionProvider.getSelectedObject();
    	if (robot != null) {
	    	SwingUtilities.invokeLater(new Runnable() {
	    		@Override
	    		public void run() {
	    			final String BR = "&nbsp;&nbsp;&nbsp;";
	    			StringBuilder sb = new StringBuilder("<html>");
	    			sb.append(" <b>Cell:</b>&nbsp;" + Arrays.toString(robot.getState().getLocation()) + BR);
	    			sb.append(" <b>Area:</b>&nbsp;" + robot.getState().getLocationId() + BR);
	    			sb.append(" <b>Location:</b>&nbsp;" + String.format("[%2.1f,%2.1f]", robot.getState().getPose().pos[0], robot.getState().getPose().pos[1]) + BR);
	    			sb.append(" <b>Yaw:</b>&nbsp;" + String.format("%2.1f", robot.getState().getYaw()) + BR);
	    			sb.append(" <b>X&nbsp;Collision:</b>&nbsp;" + robot.getState().isCollisionX() + BR);
	    			sb.append(" <b>Y&nbsp;Collision:</b>&nbsp;" + robot.getState().isCollisionY() + BR);
	    			sb.append(" <b>Carried:</b>&nbsp;" + (robot.getState().hasObject() ? robot.getState().getRoomObject() : "-") + BR);
	    			sb.append("</html>");
	    			properties.setText(sb.toString());
	    		}
	    	});
    	}
    }

	@Override
	public void refresh() {
        this.table.repaint();
        this.table.packAll();

        updateRobotProperties();
	}
	
    /* (non-Javadoc)
     * @see org.jsoar.debugger.AbstractAdaptableView#getAdapter(java.lang.Class)
     */
    @Override
    public Object getAdapter(Class<?> klass)
    {
        if(SelectionProvider.class.equals(klass))
        {
            return selectionProvider;
        }
        else if(RobotTableModel.class.equals(klass))
        {
            return model;
        }
        return super.getAdapter(klass);
    }

	public void selectRobot(Robot robot) {
		int index = model.getPlayers().indexOf(robot);
        if(index != -1) {
            table.getSelectionModel().setSelectionInterval(index, index);
            table.scrollRowToVisible(index);
        }
	}

	@Override
	public void selectionChanged(SelectionManager manager) {
		if (this.selectionProvider.isActive()) {
			return;
		}
		
    	final Robot robot = (Robot)manager.getSelectedObject();
    	if (robot != null) {
    		selectRobot(robot);
    		updateRobotProperties();
    	}
	}
}
