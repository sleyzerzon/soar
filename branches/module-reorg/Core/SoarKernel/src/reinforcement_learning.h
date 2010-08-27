/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*************************************************************************
 *
 *  file:  reinforcement_learning.h
 *
 * =======================================================================
 */

#ifndef REINFORCEMENT_LEARNING_H
#define REINFORCEMENT_LEARNING_H

#include <map>
#include <string>
#include <list>

#include "soar_module.h"

#include "chunk.h"
#include "production.h"

//////////////////////////////////////////////////////////
// RL Parameters
//////////////////////////////////////////////////////////

class rl_learning_param;

class rl_param_container: public soar_module::param_container
{
	public:
		enum learning_choices { sarsa, q };
		
		rl_learning_param *learning;
		soar_module::decimal_param *discount_rate;
		soar_module::decimal_param *learning_rate;
		soar_module::constant_param<learning_choices> *learning_policy;
		soar_module::decimal_param *et_decay_rate;
		soar_module::decimal_param *et_tolerance;
		soar_module::boolean_param *temporal_extension;
		soar_module::boolean_param *hrl_discount;
		soar_module::boolean_param *temporal_discount;

		rl_param_container( agent *new_agent );
};

class rl_learning_param: public soar_module::boolean_param
{
	private:
		bool first_switch;

	protected:
		agent *my_agent;

	public:
		rl_learning_param( const char *new_name, soar_module::boolean new_value, soar_module::predicate<soar_module::boolean> *new_prot_pred, agent *new_agent );
		void set_value( soar_module::boolean new_value );
};


//////////////////////////////////////////////////////////
// RL Statistics
//////////////////////////////////////////////////////////

class rl_stat_container: public soar_module::stat_container
{
	public:	
		soar_module::decimal_stat *update_error;
		soar_module::decimal_stat *total_reward;
		soar_module::decimal_stat *global_reward;
				
		rl_stat_container( agent *new_agent );
};


//////////////////////////////////////////////////////////
// RL Types
//////////////////////////////////////////////////////////

// map of eligibility traces
typedef std::map< production*, double > rl_et_map;

// list of rules associated with the last operator
typedef std::list< production* > rl_rule_list;

// rl data associated with each state
typedef struct rl_state_data_struct {
	Symbol* reward_header;					// reward-link identifier
	
	rl_et_map eligibility_traces;			// traces associated with productions
	rl_rule_list prev_op_rl_rules;			// rl rules associated with the previous operator
	
	double previous_q;						// q-value of the previous state
	double reward;							// accumulated discounted reward

	unsigned int gap_age;					// the number of steps since a cycle containing rl rules
	unsigned int hrl_age;					// the number of steps in a subgoal
} rl_state_data;

typedef std::map< Symbol*, Symbol* > rl_symbol_map;
typedef std::set< rl_symbol_map > rl_symbol_map_set;

typedef std::map< Symbol*, rl_state_data* > rl_state_map;


//////////////////////////////////////////////////////////
// RL Module
//////////////////////////////////////////////////////////

class rl_module: public soar_module::module
{
public:
	rl_module(agent* my_agent);
	
	// run events
	void on_create_soar_agent();
	void on_init_soar_agent();
	void on_reinitialize_soar();

	void on_create_state( Symbol* goal, goal_stack_level level, bool is_top );
	void on_retract_state( Symbol* goal );

	// publicly available information
	rl_param_container params;
	rl_stat_container stats;

	// shortcut to the learning parameter
	bool enabled();

	// maintenance on excise
	void remove_refs_for_prod( production *prod );

	// rule format validation
	static bool valid_template( production *prod );
	static bool valid_rule( production *prod );

	// templates
	void initialize_template_tracking();
	void update_template_tracking( const char *rule_name );	
	Symbol* build_template_instantiation( instantiation *my_template_instance, struct token_struct *tok, wme *w );

	// reward
	void tabulate_reward_values();
	void tabulate_reward_value_for_goal( Symbol *goal );

	// updates
	void store_data( Symbol *goal, preference *cand );
	void perform_update( double op_value, bool op_rl, Symbol *goal, bool update_efr = true );
	void watkins_clear( Symbol *goal );

private:
	
	// templates
	int get_template_id( const char *prod_name );
	int next_template_id();
	void revert_template_id();
	void get_template_constants( condition* p_conds, condition* i_conds, rl_symbol_map* constants );
	void add_goal_or_impasse_tests_to_conds( condition *all_conds );
	action* make_simple_action( Symbol *id_sym, Symbol *attr_sym, Symbol *val_sym, Symbol *ref_sym );
	void get_test_constant( test* p_test, test* i_test, rl_symbol_map* constants );
	void get_symbol_constant( Symbol* p_sym, Symbol* i_sym, rl_symbol_map* constants );

	// symbol constants
	Symbol* sc_reward_link;
	Symbol* sc_reward;
	Symbol* sc_value;

	// state data
	rl_state_data* get_state_data( Symbol* goal );
	rl_state_map state_info;
	
	int template_count;
};

#endif
