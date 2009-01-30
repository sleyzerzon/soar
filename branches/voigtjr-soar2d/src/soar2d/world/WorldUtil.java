package soar2d.world;

import java.util.List;

import org.apache.log4j.Logger;

import soar2d.Soar2D;
import soar2d.players.CommandInfo;
import soar2d.players.Player;

class WorldUtil {
	private static Logger logger = Logger.getLogger(WorldUtil.class);

	static void dumpStats(int[] sortedScores, Player[] players, boolean stopping, List<String> messages) {
		StringBuilder bigMessage = new StringBuilder(); 
		for (String message : messages) {
			System.out.println(message);
			logger.info(message);
			if (stopping) {
				bigMessage.append(message);
				bigMessage.append("\n");
			}
		}
		if (stopping) {
			Soar2D.control.infoPopUp(bigMessage.toString());
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

	static void checkMaxUpdates(List<String> stopMessages, int worldCount) {
		if (Soar2D.config.terminalsConfig().max_updates > 0) {
			if (worldCount >= Soar2D.config.terminalsConfig().max_updates) {
				stopMessages.add("Reached maximum updates, stopping.");
			}
		}
	}
	
	static void checkStopSim(List<String> stopMessages, CommandInfo command, Player player) {
		if (command.stopSim) {
			if (Soar2D.config.terminalsConfig().agent_command) {
				stopMessages.add(player.getName() + " issued simulation stop command.");
			} else {
				logger.warn(player.getName() + " issued illegal simulation stop command.");
			}
		}
	}

	static void checkWinningScore(List<String> stopMessages, int[] scores) {
		if (Soar2D.config.terminalsConfig().winning_score > 0) {
			if (scores[scores.length - 1] >= Soar2D.config.terminalsConfig().winning_score) {
				stopMessages.add("At least one player has achieved at least " + Soar2D.config.terminalsConfig().winning_score + " points.");
			}
		}
	}
	
	static void checkNumPlayers(int numPlayers) throws Exception {
		if (numPlayers == 0) {
			Soar2D.control.stopSimulation();
			throw new Exception("Update called with no players.");
		}
	}
}
