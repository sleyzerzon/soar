package edu.umich.soar.gridmap2d;


import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.util.ArrayList;
import java.util.List;

import org.flexdock.docking.DockingConstants;

import edu.umich.soar.gridmap2d.core.Simulation;
import edu.umich.soar.gridmap2d.map.Player;
import edu.umich.soar.gridmap2d.selection.SelectionManager;
import edu.umich.soar.gridmap2d.selection.SelectionProvider;

public class WorldView extends AbstractAdaptableView implements SelectionProvider {
	
	private static final long serialVersionUID = 4612350578042898852L;
	
	private final GridMapPanel gridMapPanel;
	private SelectionManager manager;
	private final Simulation sim;
	private Player selectedPlayer;

    public WorldView(Adaptable app) {
        super("worldView", "World View");

		sim = Adaptables.adapt(app, Simulation.class);
		
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
        
        gridMapPanel.addMouseListener(new MouseAdapter() {
        	@Override
        	public void mousePressed(MouseEvent e) {
        		selectPlayer(getPlayerAtPixel(e.getX(), e.getY()));
        	}
        });
    }

	private int [] getCellAtPixel(int x, int y) {
		int [] loc = new int [] {x, y};
		loc[0] /= gridMapPanel.getCellSize();
		loc[1] /= gridMapPanel.getCellSize();
		if (sim.getMap().isInBounds(loc)) {
			return loc;
		}
		return null;
	}
	
	private Player getPlayerAtPixel(int x, int y) {
		int[] xy = getCellAtPixel(x, y);
		if (xy == null) {
			return null;
		}
		return this.sim.getMap().getCell(xy).getFirstPlayer();
	}
	
	private void selectPlayer(Player player) {
		selectedPlayer = player;
		if (this.manager != null) {
			manager.fireSelectionChanged();
		}
	}

	@Override
	public void activate(SelectionManager manager) {
		this.manager = manager;
	}

	@Override
	public void deactivate() {
		this.manager = null;
	}

	@Override
	public Object getSelectedObject() {
		return selectedPlayer;
	}

	@Override
	public List<Object> getSelection() {
		List<Object> selection = new ArrayList<Object>(1);
		selection.add(selectedPlayer);
		return selection;
	}
}
