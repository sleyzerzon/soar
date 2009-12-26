#ifndef __RTDP_H__
#define __RTDP_H__

#include <map>
#include <list>
#include <set>
#include <utility>

#include "rli.h"

struct production_struct;

class rtdp {
public:
	rtdp();
	
	/*
	 * Productions have to be registered with the RTDP object before
	 * they are referred to in actions
	 */
	void register_rule(production_struct *p, double q);
	
	/*
	 * Starts a new trace and returns the ID for the trace.
	 */
	int new_trace();
	
	/*
	 * Tell the object that the trace finished (the subgoal retracts)
	 */
	void end_trace(int trace);
	
	/* Get the current state that is recorded for the trace. */
	rli_state_id get_state(int trace);
	
	/*
	 * This tells the RTDP object that the agent has entered a new state
	 * after taking the action specified by the last call to "set_action"
	 * and has received a reward of r, for a particular trace.
	 * Returns the state id.
	 */
	rli_state_id update_state(int trace, const std::list<rli_action*> &state, double r);
	
	/*
	 * This tells the RTDP object that the agent has entered the
	 * terminal state after taking the action specified by the last call
	 * to "set_action" and has received a reward of r, for a particular
	 * trace.
	 */	
	void update_state_terminal(int trace, double r);
	
	/* 
	 * Tells the RTDP object what action is about to be taken in a
	 * particular trace 
	 */
	void set_action(int trace, rli_action* action);
	
	/* 
	 * Tells the RTDP object that the null action was taken
	 */	
	void set_null_action(int trace);
	
	/* perform a DP update */
	void dp_update(rli_state_id state, double discount, std::map<production_struct*, double> &changes);
	
	void random_dp_update(double discount, std::map<production_struct*, double> &changes);
	
	rli_transmodel &get_model();
	rli_metastate &get_metastate();
	
	void print_svals(std::ostream &os);
	void print_avals(std::ostream &os);
	void print_model(std::ostream &os);
	
private:
	void update_vals(production_struct *r);
	double update_sval(rli_state_id sid);
	double update_aval(rli_action_id aid);
	
	rli_metastate   metastate;
	rli_transmodel  model;
	
	int tracecounter;
	std::map<int, std::pair<rli_state_id, rli_action_id> > traces;
	
	std::map<production_struct*, double> qvals;
	
	// value for a rli action is the sum of the Q values of rl rules that contribute
	std::map<rli_action_id, double> avals;
	
	// value for a rli state is the max of the actions available in that state
	std::map<rli_state_id, double> svals;
};

#endif