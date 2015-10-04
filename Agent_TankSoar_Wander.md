# TankSoar Wander Agent #
This is a very simple agent that wanders the map and adjusts its radar power.

Wandering consists of moving around the map, using sensors to avoid bumping into obstacles and to detect other objects. To best utilize a tank's radar, which works from the front of the tank this agent prefers to move forward and turn only to avoid obstacles. The radar uses up energy, so it attempts to use it sparingly. The simplest thing to do is to turn on the radar when the tank turns, and turn it off if there is nothing interesting to see (such as tanks, missiles, the energy recharger, or the health recharger).

This agent uses four operators and one search control rule:
  * Move: move forward if not blocked.
  * Turn: if front is blocked, rotate and turn on the radar with power 13.
  * Turn-backward: if at the end of an alley, turn left
  * Radar-off: if the radar is on and there are no objects visible, turn off the radar.
  * If radar-off is proposed, then prefer it to move.

### Soar capabilities ###
  * Basic PSCM functions: operator proposal, operator evaluation, internal operator application

### Download Links ###
  * This agent is packaged with the TankSoar environment.

### External Environment ###
  * [TankSoar](Domains_TankSoar.md)

### Default Rules ###
  * None.

### Associated Publications ###
  * [The Soar Tutorial: Chapter 3](http://web.eecs.umich.edu/~soar/downloads/Documentation/SoarTutorial/Soar%20Tutorial%20Part%203.pdf)

### Developer ###
  * John Laird

### Soar Versions ###
  * Soar 8,9

### Project Type ###
  * Plain text file