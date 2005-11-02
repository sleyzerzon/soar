#ifndef REINFORCEMENT_H
#define REINFORCEMENT_H

#include "mem.h"
#include "gdatastructs.h"
#include "agent.h"
#include "symtab.h"

// #ifdef NUMERIC_INDIFFERENCE
/*-------------------------------------------------------------------------
                          Reinforcement learning data

Stores data between decision phases that is needed to update numeric preference values. RL_data_structs
are used on goal identifiers.

Fields in an RL_data_struct:
	productions_to_be_updated: A list of pointers to the RL rules that fired for the selected operator
							   on a previous decision cycle, and that will have their numeric
							   values updated on a subsequent decision cycle.
    previous_Q: The Q-value computed for the previously selected operator.
 	reward: Accumulates reward for an extended operator.
	step: The number of decision cycles an operator has been active.
--------------------------------------------------------------------------*/
typedef struct RL_data_struct {
list *productions_to_be_updated;
float previous_Q;
float reward;
int step;
} RL_data;

// #endif

extern float compute_temp_diff(agent *, RL_data *, float);
extern bool perform_Bellman_update(agent *, float , Symbol *);
extern void RL_update_symbolically_chosen(agent *, slot *, preference *);
extern void tabulate_reward_values(agent *);
extern void store_RL_data(agent *, Symbol *, preference *);
extern void tabulate_reward_value_for_goal(agent *thisAgent, Symbol *goal);

#endif