package edu.umich.soar.gridmap2d;


import org.flexdock.docking.DockingConstants;

public class WorldView extends AbstractAdaptableView {
	
	private static final long serialVersionUID = 4612350578042898852L;
	private final Adaptable app;
	
	private final GridMapPanel gridMapPanel;
	
    public WorldView(Adaptable app)
    {
        super("worldView", "World View");

        this.app = app;
        
        addAction(DockingConstants.PIN_ACTION);
        
        gridMapPanel = new GridMapPanel(this.app);
        setContentPane(gridMapPanel);
        
    }
    
}
