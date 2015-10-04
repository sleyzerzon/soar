# Eaters Advanced Move Agent #
An eaters agent that implements a generalized move operator that proposes moves to any adjacent position that is empty, has food or contains another eater.  It prefers operators that move towards bonus food and avoids operators that move to empy spots or ones with an eater in them.  Unlike the basic move agent, this one will avoid repeating the previous move.

### Soar capabilities ###
  * Basic PSCM functions: State Elaboration, Operator Proposal, Operator Evaluation, Internal Operator Application
  * Demonstrates creating persistent structures in memory

### Download Links ###
  * This agent is packaged with the Eaters environment.

### External Environment ###
  * [Eaters](Domains_Eaters.md)

### Default Rules ###
  * None.

### Associated Publications ###
  * [The Soar Tutorial: Chapter 2](http://web.eecs.umich.edu/~soar/downloads/Documentation/SoarTutorial/Soar%20Tutorial%20Part%202.pdf)

### Developer ###
  * John Laird

### Soar Versions ###
  * Soar 8,9

### Project Type ###
  * Plain text