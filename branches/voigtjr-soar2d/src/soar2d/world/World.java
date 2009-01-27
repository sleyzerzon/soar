package soar2d.world;

import java.io.File;
import java.lang.Math;
import java.util.Arrays;
import java.util.Iterator;

import org.apache.log4j.Logger;

import soar2d.Game;
import soar2d.Soar2D;
import soar2d.config.SimConfig;
import soar2d.map.EatersMap;
import soar2d.map.GridMap;
import soar2d.map.TaxiMap;
import soar2d.players.MoveInfo;
import soar2d.players.Player;

public class World {
	
	public int getWorldCount() {
		return worldCount;
	}
	
	public boolean isTerminal() {
		return printedStats;
	}

	public boolean recentlyMovedOrRotated(Player targetPlayer) {
		MoveInfo move = players.getMove(targetPlayer);
		if (move == null) {
			return false;
		}
		return move.move || move.rotate;
	}

	public void interruped(String name) {
		if (Soar2D.wm.using()) {
			return;
		}
		if (players.numberOfPlayers() <= 1) {
			return;
		}
		Iterator<Player> iter = players.iterator();
		Player thePlayer = null;
		Integer lowestScore = null;
		while (iter.hasNext()) {
			Player player = iter.next();
			if (player.getName().equals(name)) {
				thePlayer = player;
			} else {
				if (lowestScore == null) {
					lowestScore = new Integer(player.getPoints());
				} else {
					lowestScore = Math.min(lowestScore, player.getPoints());
				}
			}
			
		}
		if ((thePlayer == null) || (lowestScore == null)) {
			// shouldn't happen if name is valid
			return;
		}
		
		lowestScore -= 1;
		thePlayer.setPoints(lowestScore, "interrupted");
		this.dumpStats(true, "interrupted");
	}
	
	public PlayersManager getPlayers() {
		return players;
	}
}
