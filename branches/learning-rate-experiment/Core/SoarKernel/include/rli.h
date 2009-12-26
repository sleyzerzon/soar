/*
 * Data structures for working in the RL rule-induced state space.
 */
 
#ifndef __RLI_H__
#define __RLI_H__

#include <set>
#include <map>
#include <list>
#include <ostream>

struct production_struct;

/* 
 * An action in the induced space is a set of RL rules that contribute
 * to the same operator
 */
typedef std::set<production_struct*> rli_action;
typedef int rli_action_id;

/*
 * Each element in the powerset of actions corresponds to one distinct
 * state in the RL rule induced state space.
 */
typedef std::set<rli_action_id> rli_state;
typedef int rli_state_id;

typedef std::pair<rli_state_id, rli_action_id> rli_sa_pair;

struct rli_state_ptr_lt {
	bool operator()(const rli_state* a, const rli_state* b) const {
		return (*a) < (*b);
	}
};

struct rli_action_ptr_lt {
	bool operator()(const rli_action* a, const rli_action* b) const {
		return (*a) < (*b);
	}
};

typedef std::map<rli_action*, rli_action_id, rli_action_ptr_lt> rli_aid_map;
typedef std::map<rli_state*, rli_state_id, rli_state_ptr_lt> rli_sid_map;

/*
 * This class is responsible for keeping track of all state information
 * related to the RL rule induced state space. This includes naming
 * states and actions with indexes and keeping track of the value of
 * states.
 */
class rli_metastate {
public:
	rli_metastate();
	~rli_metastate();
	
	rli_action_id get_action_id(rli_action *action, bool *isnew = NULL);
	rli_state_id get_state_id(rli_state *state, bool *isnew = NULL);
	
	rli_action *get_action(rli_action_id id);
	rli_state *get_state(rli_state_id id);

	/* 
     * Get all actions that r contributes to. Returns the number of such
     * actions.
	 */
	int get_relevant_actions(production_struct *r, std::set<rli_action_id> &actions);
	
	/* 
     * Get all states that an action is available in. Returns the number
     * of such states.
	 */
	int get_relevant_states(rli_action_id aid, std::set<rli_state_id> &states);
	
	rli_state_id random_state();
	
private:
	void assoc_rule_action(production_struct *r, rli_action_id a);
	void assoc_action_state(rli_action_id a, rli_state_id s);
	
	int idcounter;
	
	// maps for looking up actions and action ids in both directions
	rli_aid_map aidmap;
	std::map<rli_action_id, rli_action*> idamap;
	
	// maps for looking up states and state ids in both directions
	rli_sid_map sidmap;
	std::map<rli_state_id, rli_state*> idsmap;
	
	// map from rl rules to all actions that they contribute to
	std::map<production_struct*, std::list<rli_action_id> > ramap;
	
	// map from actions to all states they are available in
	std::map<rli_action_id, std::list<rli_state_id> > asmap;
};

/* statistics that have to be maintained for each state-action pair */
typedef struct rli_model_stats_struct {

	/* 
	 * Maps next states to the number of times transitions landed in them
	 * and the sum of all rewards for those transitions. Note that
	 *
	 * times / nexps = P(s,a,s')
	 * 
	 * and 
	 *
	 * (sum of rewards) / nexps = avg reward
	 */
	std::map<rli_state_id, int> *nstotals;
	
	// total number of experiences of this s, a pair
	int nexps;
	
	// total reward
	double reward;
	
} rli_model_stats;

/*
 * Maintains a maximum likelihood estimate of the transition function
 * and reward function in the RL rule induced state space.
 */
class rli_transmodel {
public:
	/* some special states */
	enum { TERMSTATE = -1, INITSTATE = 0 };
	
	/* 
	 * special action indicating agent chose an operator that does not
	 * have a Q-value
	 */
	enum { NULLACTION = -1 };

	rli_transmodel();
	~rli_transmodel();

	/* improve the model with a transition experience */
	void addexp(rli_state_id s1, rli_action_id a, rli_state_id s2, double r);
	
	/* returns sum(s', T(s,a,s') * [ R(s,a,s') + gamma * V(s') ]) */
	double TDsignal(rli_state_id s, 
	                rli_action_id a, 
	                double discount,
	                const std::map<rli_state_id, double> &svals,
	                bool &nostats);

	void print(std::ostream &os);
	void print(std::ostream &os, rli_metastate &ms);
	
private:
	std::map<rli_sa_pair, rli_model_stats*> stats;
};

#endif