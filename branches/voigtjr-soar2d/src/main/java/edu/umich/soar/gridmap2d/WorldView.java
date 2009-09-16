package edu.umich.soar.gridmap2d;


import org.flexdock.docking.DockingConstants;

import edu.umich.soar.gridmap2d.core.Simulation;

public class WorldView extends AbstractAdaptableView {
	
	private static final long serialVersionUID = 4612350578042898852L;
	
	private final GridMapPanel gridMapPanel;
	
    public WorldView(Adaptable app) {
        super("worldView", "World View");

		Simulation sim = Adaptables.adapt(app, Simulation.class);
		
        addAction(DockingConstants.PIN_ACTION);

        switch (sim.getGame()) {
        default:
        case EATERS:
            gridMapPanel = new EatersPanel(app);
            break;
        case TANKSOAR:
            gridMapPanel = new TankSoarPanel(app);
            break;
        case TAXI:
            gridMapPanel = new TaxiPanel(app);
            break;
        case ROOM:
            gridMapPanel = new RoomPanel(app);
        	break;
        }
        setContentPane(gridMapPanel);
    }
}
