// Update the value on RL productions from last cycle
bool perform_Bellman_update(agent *thisAgent, float best_op_value, Symbol *goal){

	RL_data *data = goal->id.RL_data;
 	int num_prods = list_length(data->productions_to_be_updated);
	float update = compute_temp_diff(thisAgent, data, best_op_value);
	bool current_pref_changed = FALSE;
	
	
	print("Update %f\n", update); // temporary
 			
	if (num_prods > 0){  // if there is a production to update
		float increment = current_agent(alpha)*(update / num_prods);
		cons *c = data->productions_to_be_updated;
			while(c){
		 
				production *prod = (production *) c->first;
				c = c->rest;
	 
				if (!prod) continue;
	  		 					
				float temp = rhs_value_to_symbol(prod->action_list->referent)->fc.value;
				temp += increment;

				symbol_remove_ref(rhs_value_to_symbol(prod->action_list->referent));
				prod->action_list->referent = symbol_to_rhs_value(make_float_constant(temp));
			 
				if (prod->instantiations){
					current_pref_changed = TRUE;
					for (inst = prod->instantiations ; inst ; inst = inst->next){
						for (pref = inst->preferences_generated ; pref ; pref = pref->inst_next){
							symbol_remove_ref(pref->referent);
							pref->referent = symbol_to_rhs_value(make_float_constant(temp));
						}
					}
				}
  
				print_with_symbols("\n%y  ", prod->name); // temporary
 	 			
	    		// print("Prediction %f ", record->previous_Q);
				// print_with_symbols("value %y ", rhs_value_to_symbol(prod->action_list->referent));
		}
		
 		 	data->reward = 0.0;
			data->step = 0;
			data->previous_Q = 0;
			free_list(data->productions_to_be_updated);
			data->productions_to_be_updated = NIL;
	}
	return current_pref_changed;
}

/* Computes the TD update for Q-learning. Takes as input the current reward, max Q at the next step, and the previous estimate of the Q-value. */ 
float compute_temp_diff(agent *thisAgent, RL_data* r, float best_op_value){
	
	// print_with_symbols("\n Q value for %y\n", r->op);

	float Q = r->reward;

    // print("\n Q after reward is %f\n" , Q);
	Q += pow(current_agent(gamma), r->step)*best_op_value;
	//Q += pow(current_agent(gamma), r->step)*(r->next_Q);
	// print("Q after next_Q update is %f\n", Q);
	// print("\n alpha is %f\n", current_agent(alpha));
	Q -= r->previous_Q;
	// print("Q after previous %f\n", Q);
	// Q *= current_agent(alpha);
	// print("Q after alpha %f\n", Q);

    /*if (r->num_prod > 0)
		Q = Q / r->num_prod;*/

	return Q;

}

void RL_update_symbolically_chosen(agent *thisAgent, slot *s, preference *candidates){ /* SAN - compute Q-value when winner decided by symbolic preferences */
	if (!candidates) return;
	double temp_Q = 0;   // DEFAULT_INDIFFERENT_VALUE;
	// for (rec = current_agent(records) ; rec->level > goal_level; rec = rec->next)
	// rec->next_Q = 0;
	
	for (preference *temp=s->preferences[NUMERIC_INDIFFERENT_PREFERENCE_TYPE]; temp!=NIL; temp=temp->next){
		if (candidates->value == temp->value){
			if (temp->referent->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE)
				temp_Q += temp->referent->ic.value;
			if (temp->referent->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE)
				temp_Q += temp->referent->fc.value;
		}
	}
	//   	rec->next_Q = temp_Q;
	//	candidates->sum_of_probability = temp_Q;
	learn_RL_productions(thisAgent, temp_Q, s->id);
}

/* Called at the beginning of the decision phase. Also called immediately before "halting" to find rewards associated with the end of an episodic task.
Finds numeric rewards one and two levels under the reward link and adds them together. */
float tabulate_reward_values(agent *thisAgent){
	Symbol *goal = thisAgent->top_goal;

	while(goal){
	    RL_data *data = goal->id.RL_data;
		slot *s = goal->id.reward_header;
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
		data->step++;
		}
		goal = goal->id.lower_goal;
	}
}
