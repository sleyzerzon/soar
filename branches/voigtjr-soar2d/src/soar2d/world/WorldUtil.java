package soar2d.world;

import java.util.Iterator;

import org.apache.log4j.Logger;

import soar2d.Soar2D;
import soar2d.players.Player;

class WorldUtil {
	private static Logger logger = Logger.getLogger(WorldUtil.class);

	static void dumpStats(PlayersManager players, boolean stop, String message) {
		int[] scores = players.getSortedScores();
		System.out.println(message);
		if (stop) {
			Soar2D.control.infoPopUp(message);
			Soar2D.control.stopSimulation();
		}
		boolean draw = false;
		if (scores.length > 1) {
			if (scores[scores.length - 1] ==  scores[scores.length - 2]) {
				logger.debug("Draw detected.");
				draw = true;
			}
		}
		
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			String status = null;
			Player player = iter.next();
			if (player.getPoints() == scores[scores.length - 1]) {
				status = draw ? "draw" : "winner";
			} else {
				status = "loser";
			}
			String statline = player.getName() + ": " + player.getPoints() + " (" + status + ")";
			logger.info(statline);
			System.out.println(statline);
		}
	}
}
