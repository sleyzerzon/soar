package soar2d.world;

import org.apache.log4j.Logger;

import soar2d.Soar2D;
import soar2d.players.Player;

class WorldUtil {
	private static Logger logger = Logger.getLogger(WorldUtil.class);

	static void dumpStats(int[] sortedScores, Player[] players, boolean stop, String message) {
		System.out.println(message);
		if (stop) {
			Soar2D.control.infoPopUp(message);
			Soar2D.control.stopSimulation();
		}
		boolean draw = false;
		if (sortedScores.length > 1) {
			if (sortedScores[sortedScores.length - 1] ==  sortedScores[sortedScores.length - 2]) {
				logger.debug("Draw detected.");
				draw = true;
			}
		}
		
		for (Player player : players) {
			String status = null;
			if (player.getPoints() == sortedScores[sortedScores.length - 1]) {
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
