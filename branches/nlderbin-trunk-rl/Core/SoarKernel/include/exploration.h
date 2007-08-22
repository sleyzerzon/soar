/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*************************************************************************
 *
 *  file:  exploration.h
 *
 * =======================================================================
 */

#ifndef EXPLORATION_H
#define EXPLORATION_H

#include <map>
#include <vector>

#include "gdatastructs.h"

//////////////////////////////////////////////////////////
// Exploration constants
//////////////////////////////////////////////////////////
#define EXPLORATION_EXPONENTIAL 1
#define EXPLORATION_LINEAR 2

//////////////////////////////////////////////////////////
// Exploration Types
//////////////////////////////////////////////////////////
typedef struct exploration_parameter_struct  
{
	double value;
	long reduction_policy;
	bool (*val_func)( double );
	std::map<long, double> rates;
} exploration_parameter;

//////////////////////////////////////////////////////////
// Exploration Policies
//////////////////////////////////////////////////////////

// policy <=> name conversion
extern const long convert_exploration_policy( const char *policy_name );
extern const char *convert_exploration_policy( const long policy );

// sets exploration policy name
extern bool set_exploration_policy( agent *my_agent, const char *policy_name );
extern bool set_exploration_policy( agent *my_agent, const long policy );

// get exploration policy
extern const long get_exploration_policy( agent *my_agent );

//////////////////////////////////////////////////////////
// Exploration Policy Parameters
//////////////////////////////////////////////////////////

// add parameter
extern exploration_parameter *add_exploration_parameter( double value, bool (*val_func)( double ) );

// validate parameter name
extern bool valid_parameter( agent *my_agent, const char *name );

// get parameter value
extern double get_parameter_value( agent *my_agent, const char *parameter );

// validate parameter value
extern bool validate_epsilon( double value );
extern bool validate_temperature( double value );

// validate parameter value
extern bool valid_parameter_value( agent *my_agent, const char *name, double value );

// set parameter value
extern bool set_parameter_value( agent *my_agent, const char *name, double value );

// list of parameter names
extern std::vector<std::string> *get_parameter_names( agent *my_agent );

// update parameters according to their reduction policies/rates
extern void update_exploration_parameters( agent *my_agent );

//////////////////////////////////////////////////////////
// Reduction Policies
//////////////////////////////////////////////////////////

// policy <=> name conversion
extern const long convert_reduction_policy( const char *policy_name );
extern const char *convert_reduction_policy( const long policy );

// get parameter reduction policy
extern const long get_reduction_policy( agent *my_agent, const char *parameter );

// validate reduction policy per parameter
extern bool valid_reduction_policy( agent *my_agent, const char *parameter, const char *policy_name );
extern bool valid_reduction_policy( agent *my_agent, const char *parameter, const long policy );

// set parameter reduction policy
extern bool set_reduction_policy( agent *my_agent, const char *parameter, const char *policy_name );

// list of reduction policies
extern std::vector<const char *> *get_reduction_policies( agent *my_agent, const char *parameter );

//////////////////////////////////////////////////////////
// Reduction Rates
//////////////////////////////////////////////////////////

// validate reduction rate
extern bool valid_reduction_rate( agent *my_agent, const char *parameter, const char *policy_name, double reduction_rate );
extern bool valid_reduction_rate( agent *my_agent, const char *parameter, const long policy, double reduction_rate );
extern bool valid_exponential( double reduction_rate );
extern bool valid_linear( double reduction_rate );

// get reduction rate
extern double get_reduction_rate( agent *my_agent, const char *parameter, const char *policy_name );
extern double get_reduction_rate( agent *my_agent, const char *parameter, const long policy );

// set reduction rate
extern bool set_reduction_rate( agent *my_agent, const char *parameter, const char *policy_name, double reduction_rate );
extern bool set_reduction_rate( agent *my_agent, const char *parameter, const long policy, double reduction_rate );

//////////////////////////////////////////////////////////
// Decision Procedures
//////////////////////////////////////////////////////////

// selects a candidate based upon the current exploration mode
extern preference *choose_according_to_exploration_mode( agent *my_agent, slot *s, preference *candidates );

// selects a candidate in a random fashion
extern preference *randomly_select( preference *candidates );

// selects a candidate in a softmax fashion
extern preference *probabilistically_select( preference *candidates );

// selects a candidate based on a boltzmann distribution
extern preference *boltzmann_select( agent *my_agent, preference *candidates );

// selects a candidate based upon an epsilon-greedy distribution
extern preference *epsilon_greedy_select( agent *my_agent, preference *candidates );

// returns candidate with highest q-value (random amongst ties), assumes computed values
extern preference *get_highest_q_value_pref( preference *candidates );

// computes total contribution for a candidate from each preference, as well as number of contributions
extern void compute_value_of_candidate( agent *my_agent, preference *cand, slot *s, float default_value = 0 );

#endif