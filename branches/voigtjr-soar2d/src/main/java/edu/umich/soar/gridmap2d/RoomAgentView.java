package edu.umich.soar.gridmap2d;

import java.awt.BorderLayout;
import java.util.Arrays;

import javax.swing.BorderFactory;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.SwingUtilities;
import javax.swing.event.ListSelectionEvent;

import org.flexdock.docking.DockingConstants;
import org.jdesktop.swingx.JXTable;
import org.jdesktop.swingx.decorator.HighlighterFactory;

import edu.umich.soar.gridmap2d.core.Simulation;
import edu.umich.soar.gridmap2d.map.Robot;
import edu.umich.soar.gridmap2d.selection.SelectionListener;
import edu.umich.soar.gridmap2d.selection.SelectionManager;
import edu.umich.soar.gridmap2d.selection.SelectionProvider;
import edu.umich.soar.gridmap2d.selection.TableSelectionProvider;

public class RoomAgentView extends AbstractAgentView implements SelectionListener {
	
	private static final long serialVersionUID = 88497381529040370L;
	
	private final Simulation sim;
	private final RobotTableModel model;
    private final JXTable table;
	private final JLabel properties = new JLabel();
	private final TableSelectionProvider selectionProvider;
	
    public RoomAgentView(Adaptable app) {
        super("roomAgentView", "Agent View");
        addAction(DockingConstants.PIN_ACTION);

        this.sim = Adaptables.adapt(app, Simulation.class);
        Adaptables.adapt(app, Application.class).getSelectionManager().addListener(this);
        
        this.addAction(DockingConstants.PIN_ACTION);

        this.model = new RobotTableModel(this.sim);
        this.model.initialize();
        this.table = new JXTable(this.model);
            
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
        p.add(new JScrollPane(table), BorderLayout.NORTH);

        properties.setBorder(BorderFactory.createTitledBorder("Robot Properties"));
        p.add(properties, BorderLayout.CENTER);
        
        setContentPane(p);
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
