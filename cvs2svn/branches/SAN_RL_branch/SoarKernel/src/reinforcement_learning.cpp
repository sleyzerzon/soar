#include <math.h>
#include "reinforcement_learning.h"
#include "print.h"
#include "production.h"
#include "rhsfun.h"
#include "instantiations.h"
#include "prefmem.h"
#include "wmem.h"


/**************************************************
	Tabulate reward value for goal

Given a goal id ID, sum together any rewards found under that goal's
reward link - more precisely look for numeric values R at ID.reward-link.attr.attr R.
'data->step' should hold the number of steps the current operator has 
been installed at the goal, so if there is an op no-change, data->step > 1
and the new reward is discounted and added to reward accumulated on 
previous decision cycles.

This function is called for a goal right before it is destroyed and is called
for the top state right before a 'halt'. It is also called once for each goal 
in the goal stack at the beginning of the decision phase.

**************************************************/
void tabulate_reward_value_for_goal(agent *thisAgent, Symbol *goal){
	RL_data *data = goal->id.RL_data;
	if (!data->counting_rewards) return; /* Only count rewards at top state or for op no-change impasses. */
	slot *s = goal->id.reward_header->id.slots;
	float reward = 0.0;
	if (s){
		for ( ; s ; s = s->next){
			for (wme *w = s->wmes ; w ; w = w->next){
				 if (w->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE){
					for (slot *t = w->value->id.slots ; t ; t = t->next){
						for (wme *x = t->wmes ; x ; x = x->next){
							if (x->value->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE){
								reward = reward + x->value->fc.value;
							} else if (x->value->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE){
								reward = reward + x->value->ic.value;
							}
						}
					}
				}
			}
		}
	data->reward += (reward*pow(thisAgent->gamma, data->step));
	}
	data->step++;
}

/************************************************
        Tabulate reward values
Checks for reward at all goals in the goal stack.

Called at the beginning of the decision phase.
************************************************/
void tabulate_reward_values(agent *thisAgent){
	Symbol *goal = thisAgent->top_goal;

	while(goal){
		tabulate_reward_value_for_goal(thisAgent, goal);
	    goal = goal->id.lower_goal;
	}
}

/**************************************************
		Compute temp diff

Computes a TD update. Takes as input the current reward (r_t+1),
max Q at the current time step (Q_t+1) , and the estimate of the
Q-value of the last operator selected (Q_t). 
Discounting uses the gamma parameter and a count of the
time steps since the last operator was selected.
Returns r_t+1 + gamma*Q_t+1 - Q_t.
**************************************************/

float compute_temp_diff(agent *thisAgent, RL_data* r, float best_op_value){
	
	float delta_Q = r->reward;
	delta_Q += pow(thisAgent->gamma, r->step)*best_op_value;
	delta_Q -= r->previous_Q;
	return delta_Q;
}

/***************************************************
		Perform Bellman update

If we have RL rules to update for the last operator
selected, then compute a TD update r, and change the
numeric preference for each rule by 
(r/(number of RL rules to be updated))*learning rate.

If any of the rules changed are currently matching
and asserting their numeric preference, we need to
update the value stored in the current preference
as well as the value in the rule. In this case, this
function returns current_pref_changed=TRUE.

Finally, we reset all the RL_data in preparation for
the next operator.
***************************************************/

bool perform_Bellman_update(agent *thisAgent, float best_op_value, Symbol *goal){
		
	RL_data *data = goal->id.RL_data;
 	bool current_pref_changed = FALSE;
	
	int num_prods = 0;
	for (cons *c = data->productions_to_be_updated; c ; c = c->rest){
		if (c->first)
			num_prods++;
	}
 			
	if (num_prods > 0){  // if there are productions to update
		float update = compute_temp_diff(thisAgent, data, best_op_value);
		float increment = thisAgent->alpha*(update / num_prods);
		cons *c = data->productions_to_be_updated;
			while(c){
		 
				production *prod = (production *) c->first;
				c = c->rest;
	 
				if (!prod) continue;
	  		 		
				float temp;
				if (rhs_value_to_symbol(prod->action_list->referent)->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE){
					temp = rhs_value_to_symbol(prod->action_list->referent)->ic.value;
				} else if (rhs_value_to_symbol(prod->action_list->referent)->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE){
					temp = rhs_value_to_symbol(prod->action_list->referent)->fc.value;
				} else {
				 // We should never get here. Need to return an error here.
				}
			 
				temp += increment;

				symbol_remove_ref(thisAgent, rhs_value_to_symbol(prod->action_list->referent));
				prod->action_list->referent = symbol_to_rhs_value(make_float_constant(thisAgent, temp));
			 
				if (prod->instantiations){
					current_pref_changed = TRUE;
					for (instantiation *inst = prod->instantiations ; inst ; inst = inst->next){
						for (preference *pref = inst->preferences_generated ; pref ; pref = pref->inst_next){
							symbol_remove_ref(thisAgent, pref->referent);
							pref->referent = make_float_constant(thisAgent, temp);
						}
					}
				}	 
		}
	}
	data->reward = 0.0;
	data->step = 0;
	data->previous_Q = 0;
	data->counting_rewards = TRUE;
	free_list(thisAgent, data->productions_to_be_updated);
	data->productions_to_be_updated = NIL;
	
	return current_pref_changed;
}

/*************************************************************
       RL update symbolically chosen

When an operator is chosen probabilistically, we perform a TD
update with max Q (since we do Q-learning). But when the operator
is chosen with symbolic preferences, we always update with the
value of the selected operator. That value is computed here and
used in a call to perform_Bellman_update.
*************************************************************/

void RL_update_symbolically_chosen(agent *thisAgent, slot *s, preference *candidates){
	if (!candidates) return;
	float temp_Q = 0;   // DEFAULT_INDIFFERENT_VALUE;
	
	for (preference *temp=s->preferences[NUMERIC_INDIFFERENT_PREFERENCE_TYPE]; temp!=NIL; temp=temp->next){
		if (candidates->value == temp->value){
			if (temp->referent->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE)
				temp_Q += temp->referent->ic.value;
			if (temp->referent->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE)
				temp_Q += temp->referent->fc.value;
		}
	}
	candidates->sum_of_probability = temp_Q; // store estimate of Q-value to be used to update this operator on the next decision cycle
	perform_Bellman_update(thisAgent, temp_Q, s->id);
}

/***************************************************************
		Store RL data

Store data that will be needed later to perform a Bellman update
for the current operator. This includes the current estimate of
the operator's Q-value, and a list of pointers to RL rules that
fired proposing numeric preferences for the operator.

This is called after running the decision procedure and only if
an operator has been selected.

***************************************************************/

void store_RL_data(agent *thisAgent, Symbol *goal, preference *cand)
{
	RL_data *data = goal->id.RL_data;
	Symbol *op = cand->value;
    data->previous_Q = cand->sum_of_probability;

	for (preference *pref = goal->id.operator_slot->preferences[NUMERIC_INDIFFERENT_PREFERENCE_TYPE]; pref ; pref = pref->next){
			  if (op == pref->value){
				  production *prod = pref->inst->prod;
				  push(thisAgent, prod, data->productions_to_be_updated);
				 }
	}
}

/*****************************************************************
	Reset RL

Called when reinforcement learning is turned off.
*****************************************************************/

void reset_RL(agent *thisAgent){
	Symbol *goal = thisAgent->top_goal;
	while(goal){
		RL_data *data = goal->id.RL_data;
		  free_list(thisAgent, data->productions_to_be_updated);
		  data->previous_Q = 0;
		  data->productions_to_be_updated = NIL;
		  data->reward = 0;
		  data->step = 0;
		  data->counting_rewards = TRUE;
		  goal = goal->id.lower_goal;
	}
}