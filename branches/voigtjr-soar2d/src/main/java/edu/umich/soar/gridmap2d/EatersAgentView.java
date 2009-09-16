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
import edu.umich.soar.gridmap2d.map.Eater;
import edu.umich.soar.gridmap2d.selection.SelectionListener;
import edu.umich.soar.gridmap2d.selection.SelectionManager;
import edu.umich.soar.gridmap2d.selection.SelectionProvider;
import edu.umich.soar.gridmap2d.selection.TableSelectionProvider;

public class EatersAgentView extends AbstractAdaptableView implements Refreshable, SelectionListener {
	
	private static final long serialVersionUID = -2209482143387576390L;
	
	private final Simulation sim;
	private final PlayerTableModel model;
    private final JXTable table;
	private final JLabel properties = new JLabel();
	private final TableSelectionProvider selectionProvider;
	
    public EatersAgentView(Adaptable app) {
        super("eatersAgentView", "Agent View");
        addAction(DockingConstants.PIN_ACTION);

        this.sim = Adaptables.adapt(app, Simulation.class);
        Adaptables.adapt(app, Application.class).getSelectionManager().addListener(this);
        
        this.addAction(DockingConstants.PIN_ACTION);

        this.model = new PlayerTableModel(this.sim);
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
                updateEaterProperties();
            }};
        
        final JPanel p = new JPanel(new BorderLayout());
        p.add(new JScrollPane(table), BorderLayout.NORTH);

        properties.setBorder(BorderFactory.createTitledBorder("Eater Properties"));
        p.add(properties, BorderLayout.CENTER);
        
        //GridMapPanel gridMapPanel = new EatersPanel(app);
        //p.add(gridMapPanel, BorderLayout.SOUTH);

        setContentPane(p);
    }
    
    private void updateEaterProperties() {
    	final Eater eater = (Eater)selectionProvider.getSelectedObject();
    	if (eater != null) {
	    	SwingUtilities.invokeLater(new Runnable() {
	    		@Override
	    		public void run() {
	    			//final String spaces = "&nbsp;&nbsp;&nbsp;";
	    			StringBuilder sb = new StringBuilder("<html>");
	    			sb.append("<b>Location:</b>&nbsp;");
	    			sb.append(Arrays.toString(eater.getLocation()));
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

        updateEaterProperties();
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
        else if(PlayerTableModel.class.equals(klass))
        {
            return model;
        }
        return super.getAdapter(klass);
    }

	public void selectEater(Eater eater) {
		int index = model.getPlayers().indexOf(eater);
        if(index != -1) {
            table.getSelectionModel().setSelectionInterval(index, index);
            table.scrollRowToVisible(index);
        }
	}

	@Override
	public void selectionChanged(SelectionManager manager) {
    	final Eater eater = (Eater)manager.getSelectedObject();
    	if (eater != null) {
    		selectEater(eater);
    		updateEaterProperties();
    	}
	}
}
