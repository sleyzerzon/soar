# TankSoar Simple Agent #
This agent implements a tank that wanders around the board looking for objects. It is also able to chase and attack other agents, as well as retreat.

The agent uses abstract operators that it decomposes into complex combinations of low-level actions. The tank uses its knowledge to select between these activities based on the current situation, just as it selects between different operators. Both of these problems are handled in Soar by allowing abstract high-level operators to be implemented in subgoals where a new state is created that allows low-level operators to be selected in pursuit of the high-level operators.

### Soar capabilities ###
  * Hierarchical task decomposition

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
  * VisualSoar