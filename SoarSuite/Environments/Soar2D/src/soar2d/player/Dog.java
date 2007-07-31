package soar2d.player;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;

import soar2d.PlayersManager;
import soar2d.Soar2D;
import soar2d.World;
import soar2d.world.CellObject;
import soar2d.world.GridMap;

public class Dog extends Player {

	public Dog(PlayerConfig playerConfig) {
		super(playerConfig);
	}

	Player target;
	double targetAngleOff;

	public void update(java.awt.Point location) {
		World world = Soar2D.simulation.world;
		
		super.update(location);
		
		PlayersManager players = world.getPlayers();
		if (players.numberOfPlayers() > 1) {
			Iterator<Player> playersIter = players.iterator();
			target = null;
			while (playersIter.hasNext()) {
				Player player = playersIter.next();
				if (this.equals(player) || this.getName().equals("mouse")) {
					continue;
				}
				if (player.getLocationId() == this.getLocationId()) {
					double angleOff = players.angleOff(this, player);
					double maxAngleOff = Soar2D.config.getVisionCone() / 2;
					if (Math.abs(angleOff) <= maxAngleOff) {
						target = player;
						targetAngleOff = angleOff;
						break;
					}
				}
			}
		}
	}
	
	public MoveInfo getMove() {
		MoveInfo move = new MoveInfo();
		if (target == null) {
			// todo: wander
			move.forward = true;
			move.backward = true;
			move.rotate = true;
			move.rotateDirection = "left"; 
			return move;
		}
		
		move.rotateRelative = true;
		move.rotateRelativeYaw = this.targetAngleOff;
		move.forward = true;
		
		return move;
	}
	public boolean getHumanMove() {
		return true;
	}

	public void reset() {
		super.reset();
		target = null;
	}
}
