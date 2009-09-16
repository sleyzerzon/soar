package edu.umich.soar.gridmap2d;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import javax.swing.SwingUtilities;
import javax.swing.table.AbstractTableModel;

import edu.umich.soar.gridmap2d.core.Simulation;
import edu.umich.soar.gridmap2d.core.events.PlayerAddedEvent;
import edu.umich.soar.gridmap2d.core.events.PlayerRemovedEvent;
import edu.umich.soar.gridmap2d.events.SimEvent;
import edu.umich.soar.gridmap2d.events.SimEventListener;
import edu.umich.soar.gridmap2d.events.SimEventManager;
import edu.umich.soar.gridmap2d.map.Player;

public class PlayerTableModel extends AbstractTableModel {

	private static final long serialVersionUID = 8818379571567684870L;

	private final Simulation sim;
	private final Listener listener = new Listener();
	private final List<Player> players = Collections.synchronizedList(new ArrayList<Player>());
	
	public PlayerTableModel(Simulation sim) {
		this.sim = sim;
	}
	
	public void initialize() {
		final SimEventManager eventManager = sim.getEvents();
		eventManager.addListener(PlayerAddedEvent.class, listener);
		eventManager.addListener(PlayerRemovedEvent.class, listener);
		
		players.addAll(sim.getWorld().getPlayers());
	}
	
	@Override
	public int getColumnCount() {
		return 2;
	}

	@Override
	public int getRowCount() {
		return players.size();
	}

	@Override
	public Object getValueAt(int row, int column) {
		synchronized(players) {
			Player p = players.get(row);
			switch (column) {
			case 0: return p;
			case 1: return p.getPoints();
			}
		}
		return null;
	}
	
	private void handlePlayerRemoved(Player player) {
        int row = 0;
        synchronized (players)
        {
            row = players.indexOf(player);
            if(row == -1)
            {
                return;
            }
            players.remove(row);
        }
        fireTableRowsDeleted(row, row);
	}

	private void handlePlayerAdded(Player player) {
		int row = 0;
		synchronized (players) {
			row = players.size();
			players.add(player);
		}
		fireTableRowsInserted(row, row);
	}
	
    @Override
    public Class<?> getColumnClass(int c)
    {
    	switch(c) {
    	case 0: return Player.class;
    	case 1: return Integer.class;
    	}
    	return super.getColumnClass(c);
    }
    
    @Override
    public String getColumnName(int c)
    {
        switch(c)
        {
        case 0: return "Name";
        case 1: return "Score";
        }
        return super.getColumnName(c);
    }
    
	private class Listener implements SimEventListener {
		@Override
		public void onEvent(final SimEvent event) {
			{
				Runnable runnable = new Runnable() {
					public void run() {
						if (event instanceof PlayerAddedEvent) {
							handlePlayerAdded(((PlayerAddedEvent) event)
									.getPlayer());
						} else {
							handlePlayerRemoved(((PlayerRemovedEvent) event)
									.getPlayer());
						}
					}

				};
				if (SwingUtilities.isEventDispatchThread()) {
					runnable.run();
				} else {
					SwingUtilities.invokeLater(runnable);
				}
			}

		}
	}

	public List<Player> getPlayers() {
		return players;
	}
}
