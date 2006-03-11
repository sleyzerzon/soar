#ifndef REINFORCEMENT_H
#define REINFORCEMENT_H


#include "agent.h"
#include "mem.h"
#include "gdatastructs.h"
#include "symtab.h"
#include "instantiations.h"


/*-------------------------------------------------------------------------
                          Reinforcement learning data

Stores data between decision phases that is needed to update numeric preference values. RL_data_structs
are used on goal identifiers.

Fields in an RL_data_struct:
	productions_to_be_updated: A list of pointers to the RL rules that fired for the selected operator
							   on a previous decision cycle, and that will have their numeric
							   values updated on a subsequent decision cycle.
    previous_Q: The Q-value computed for the previously selected operator.
 	reward: Accumulates reward for an extended operator. (ie, op no-change)
	step: The number of decision cycles an operator has been active. (will be > 1 only for op no-change)
	counting_rewards: indicates whether this state is impassed and if so, what kind of impasse
--------------------------------------------------------------------------*/
typedef struct RL_data_struct {
list *productions_to_be_updated;
float previous_Q;
float reward;
int step;
byte impasse_type;
} RL_data;

// #endif

extern float compute_temp_diff(agent *, RL_data *, float);
extern bool perform_Bellman_update(agent *, float , Symbol *);
extern void RL_update_symbolically_chosen(agent *, slot *, preference *);
extern void tabulate_reward_values(agent *);
extern void store_RL_data(agent *, Symbol *, preference *);
extern void tabulate_reward_value_for_goal(agent *thisAgent, Symbol *goal);
extern void reset_RL(agent *thisAgent);
extern production *build_production(agent *thisAgent, condition *top_cond, not_struct *nots, preference *pref);
extern void check_prefs_for_RL(production *prod);
extern Bool check_template_for_RL(production *prod);


#endif