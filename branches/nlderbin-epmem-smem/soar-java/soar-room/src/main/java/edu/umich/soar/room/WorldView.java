package edu.umich.soar.room;


import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.util.ArrayList;
import java.util.List;

import org.flexdock.docking.DockingConstants;

import edu.umich.soar.room.core.Simulation;
import edu.umich.soar.room.map.Robot;
import edu.umich.soar.room.selection.SelectionManager;
import edu.umich.soar.room.selection.SelectionProvider;

public class WorldView extends AbstractAdaptableView implements SelectionProvider, Refreshable {
	
	private static final long serialVersionUID = 4612350578042898852L;
	
	private final GridMapPanel gridMapPanel;
	private SelectionManager manager;
	private final Simulation sim;
	private Robot selectedPlayer;

    public WorldView(Adaptable app) {
        super("worldView", "World View");

		sim = Adaptables.adapt(app, Simulation.class);
		
        addAction(DockingConstants.PIN_ACTION);

        gridMapPanel = new RoomPanel(app);
        setContentPane(gridMapPanel);
        
        gridMapPanel.addMouseListener(new MouseAdapter() {
        	@Override
        	public void mousePressed(MouseEvent e) {
        		selectPlayer(getPlayerAtPixel(e.getX(), e.getY()));
        	}
        });
    }

	private Robot getPlayerAtPixel(int x, int y) {
		int[] xy = gridMapPanel.getCellAtPixel(x, y);
		if (sim.getMap().isInBounds(xy)) {
			return this.sim.getMap().getCell(xy).getFirstPlayer();
		}
		return null;
	}
	
	private void selectPlayer(Robot player) {
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

	@Override
	public void refresh() {
		this.gridMapPanel.repaint();
	}
}
