/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  episodic_memory.h
 *
 * =======================================================================
 */

#ifndef EPISODIC_MEMORY_H
#define EPISODIC_MEMORY_H

#include <portability.h>

#include <map>
#include <list>
#include <stack>
#include <set>
#include <queue>

#include "soar_module.h"
#include "soar_db.h"

//////////////////////////////////////////////////////////
// EpMem Experimentation
//
// If defined, we hijack the main EpMem function
// for tight-loop experimentation/timing.
//
//////////////////////////////////////////////////////////

//#define EPMEM_EXPERIMENT


//////////////////////////////////////////////////////////
// EpMem Constants
//////////////////////////////////////////////////////////

enum epmem_variable_key
{
	var_rit_offset_1, var_rit_leftroot_1, var_rit_rightroot_1, var_rit_minstep_1,
	var_rit_offset_2, var_rit_leftroot_2, var_rit_rightroot_2, var_rit_minstep_2,
	var_mode, var_next_id
};

// algorithm constants
#define EPMEM_MEMID_NONE							0
#define EPMEM_NODEID_ROOT							0

#define EPMEM_NODE_POS								0
#define EPMEM_NODE_NEG								1
#define EPMEM_RANGE_START							0
#define EPMEM_RANGE_END								1
#define EPMEM_RANGE_EP								0
#define EPMEM_RANGE_NOW								1
#define EPMEM_RANGE_POINT							2

#define EPMEM_RIT_ROOT								0
#define EPMEM_RIT_OFFSET_INIT						-1
#define EPMEM_LN_2									0.693147180559945

#define EPMEM_DNF									2

#define EPMEM_RIT_STATE_NODE						0
#define EPMEM_RIT_STATE_EDGE						1


//////////////////////////////////////////////////////////
// EpMem Typedefs
//////////////////////////////////////////////////////////

// represents a unique node identifier in the episodic store
typedef intptr_t epmem_node_id;

// represents a unique temporal hash in the episodic store
typedef uintptr_t epmem_hash_id;

// represents a unique episode identifier in the episodic store
typedef uintptr_t epmem_time_id;


//////////////////////////////////////////////////////////
// EpMem Parameters
//////////////////////////////////////////////////////////

class epmem_path_param;
class epmem_graph_match_param;
class epmem_mode_param;

class epmem_param_container: public soar_module::param_container
{
	public:
		enum db_choices { memory, file };
		enum mode_choices { tree, graph };
		enum phase_choices { phase_output, phase_selection };
		enum trigger_choices { none, output, dc };
		enum force_choices { remember, ignore, force_off };

		enum cache_choices { cache_S, cache_M, cache_L };
		enum opt_choices { opt_safety, opt_speed };

		soar_module::boolean_param *learning;
		soar_module::constant_param<db_choices> *database;
		epmem_path_param *path;
		soar_module::integer_param *commit;

		epmem_mode_param *mode;
		epmem_graph_match_param *graph_match;

		soar_module::constant_param<phase_choices> *phase;
		soar_module::constant_param<trigger_choices> *trigger;
		soar_module::constant_param<force_choices> *force;
		soar_module::decimal_param *balance;
		soar_module::set_param *exclusions;
		soar_module::constant_param<soar_module::timer::timer_level> *timers;

		soar_module::constant_param<cache_choices> *cache;
		soar_module::constant_param<opt_choices> *opt;

		epmem_param_container( agent *new_agent );
};

class epmem_path_param: public soar_module::string_param
{
	protected:
		agent *my_agent;

	public:
		epmem_path_param( const char *new_name, const char *new_value, soar_module::predicate<const char *> *new_val_pred, soar_module::predicate<const char *> *new_prot_pred, agent *new_agent );
		virtual void set_value( const char *new_value );
};

class epmem_graph_match_param: public soar_module::boolean_param
{
	protected:
		agent *my_agent;

	public:
		epmem_graph_match_param( const char *new_name, soar_module::boolean new_value, soar_module::predicate<soar_module::boolean> *new_prot_pred, agent *new_agent );
		virtual bool validate_string( const char *new_string );
};

class epmem_mode_param: public soar_module::constant_param<epmem_param_container::mode_choices>
{
	protected:
		agent *my_agent;

	public:
		epmem_mode_param( const char *new_name, epmem_param_container::mode_choices new_value, soar_module::predicate<epmem_param_container::mode_choices> *new_prot_pred, agent *new_agent );
		virtual void set_value( epmem_param_container::mode_choices new_value );
};

template <typename T>
class epmem_db_predicate: public soar_module::agent_predicate<T>
{
	public:
		epmem_db_predicate( agent *new_agent );
		bool operator() ( T val );
};


//////////////////////////////////////////////////////////
// EpMem Statistics
//////////////////////////////////////////////////////////

typedef soar_module::primitive_stat<epmem_time_id> epmem_time_id_stat;
typedef soar_module::primitive_stat<epmem_node_id> epmem_node_id_stat;

class epmem_mem_usage_stat;
class epmem_mem_high_stat;

class epmem_stat_container: public soar_module::stat_container
{
	public:
		epmem_time_id_stat *time;
		epmem_mem_usage_stat *mem_usage;
		epmem_mem_high_stat *mem_high;
		soar_module::integer_stat *ncb_wmes;

		soar_module::integer_stat *qry_pos;
		soar_module::integer_stat *qry_neg;
		epmem_time_id_stat *qry_ret;
		soar_module::integer_stat *qry_card;
		soar_module::integer_stat *qry_lits;

		epmem_node_id_stat *next_id;

		soar_module::intptr_stat *rit_offset_1;
		soar_module::intptr_stat *rit_left_root_1;
		soar_module::intptr_stat *rit_right_root_1;
		soar_module::intptr_stat *rit_min_step_1;

		soar_module::intptr_stat *rit_offset_2;
		soar_module::intptr_stat *rit_left_root_2;
		soar_module::intptr_stat *rit_right_root_2;
		soar_module::intptr_stat *rit_min_step_2;

		epmem_stat_container( agent *my_agent );
};

class epmem_mem_usage_stat: public soar_module::intptr_stat
{
	protected:
		agent *my_agent;

	public:
		epmem_mem_usage_stat( agent *new_agent, const char *new_name, intptr_t new_value, soar_module::predicate<intptr_t> *new_prot_pred );
		intptr_t get_value();
};

//

class epmem_mem_high_stat: public soar_module::intptr_stat
{
	protected:
		agent *my_agent;

	public:
		epmem_mem_high_stat( agent *new_agent, const char *new_name, intptr_t new_value, soar_module::predicate<intptr_t> *new_prot_pred );
		intptr_t get_value();
};


//////////////////////////////////////////////////////////
// EpMem Timers
//////////////////////////////////////////////////////////

class epmem_timer_container: public soar_module::timer_container
{
	public:
		soar_module::timer *total;
		soar_module::timer *storage;
		soar_module::timer *ncb_retrieval;
		soar_module::timer *query;
		soar_module::timer *api;
		soar_module::timer *trigger;
		soar_module::timer *init;
		soar_module::timer *next;
		soar_module::timer *prev;
		soar_module::timer *hash;

		soar_module::timer *ncb_edge;
		soar_module::timer *ncb_edge_rit;
		soar_module::timer *ncb_node;
		soar_module::timer *ncb_node_rit;

		soar_module::timer *query_dnf;
		soar_module::timer *query_graph_match;
		soar_module::timer *query_pos_start_ep;
		soar_module::timer *query_pos_start_now;
		soar_module::timer *query_pos_start_point;
		soar_module::timer *query_pos_end_ep;
		soar_module::timer *query_pos_end_now;
		soar_module::timer *query_pos_end_point;
		soar_module::timer *query_neg_start_ep;
		soar_module::timer *query_neg_start_now;
		soar_module::timer *query_neg_start_point;
		soar_module::timer *query_neg_end_ep;
		soar_module::timer *query_neg_end_now;
		soar_module::timer *query_neg_end_point;

		epmem_timer_container( agent *my_agent );
};

class epmem_timer_level_predicate: public soar_module::agent_predicate<soar_module::timer::timer_level>
{
	public:
		epmem_timer_level_predicate( agent *new_agent );
		bool operator() ( soar_module::timer::timer_level val );
};

class epmem_timer: public soar_module::timer
{
	public:
		epmem_timer( const char *new_name, agent *new_agent, timer_level new_level );
};


//////////////////////////////////////////////////////////
// EpMem Statements
//////////////////////////////////////////////////////////

class epmem_common_statement_container: public soar_module::sqlite_statement_container
{
	public:
		soar_module::sqlite_statement *begin;
		soar_module::sqlite_statement *commit;
		soar_module::sqlite_statement *rollback;

		soar_module::sqlite_statement *var_get;
		soar_module::sqlite_statement *var_set;

		soar_module::sqlite_statement *rit_add_left;
		soar_module::sqlite_statement *rit_truncate_left;
		soar_module::sqlite_statement *rit_add_right;
		soar_module::sqlite_statement *rit_truncate_right;

		soar_module::sqlite_statement *hash_get;
		soar_module::sqlite_statement *hash_add;

		epmem_common_statement_container( agent *new_agent );
};

class epmem_tree_statement_container: public soar_module::sqlite_statement_container
{
	public:
		soar_module::sqlite_statement *add_time;

		//
		
		soar_module::sqlite_statement *add_node_now;
		soar_module::sqlite_statement *delete_node_now;
		soar_module::sqlite_statement *add_node_point;
		soar_module::sqlite_statement *add_node_range;

		//

		soar_module::sqlite_statement *add_node_unique;
		soar_module::sqlite_statement *find_node_unique;
		soar_module::sqlite_statement *find_identifier;

		//

		soar_module::sqlite_statement *valid_episode;
		soar_module::sqlite_statement *next_episode;
		soar_module::sqlite_statement *prev_episode;

		soar_module::sqlite_statement *get_episode;

		//

		epmem_tree_statement_container( agent *new_agent );
};

class epmem_graph_statement_container: public soar_module::sqlite_statement_container
{
	public:
		soar_module::sqlite_statement *add_time;

		//

		soar_module::sqlite_statement *add_node_now;
		soar_module::sqlite_statement *delete_node_now;
		soar_module::sqlite_statement *add_node_point;
		soar_module::sqlite_statement *add_node_range;

		soar_module::sqlite_statement *add_node_unique;
		soar_module::sqlite_statement *find_node_unique;

		//

		soar_module::sqlite_statement *add_edge_now;
		soar_module::sqlite_statement *delete_edge_now;
		soar_module::sqlite_statement *add_edge_point;
		soar_module::sqlite_statement *add_edge_range;

		soar_module::sqlite_statement *add_edge_unique;
		soar_module::sqlite_statement *find_edge_unique;
		soar_module::sqlite_statement *find_edge_unique_shared;

		//

		soar_module::sqlite_statement *valid_episode;
		soar_module::sqlite_statement *next_episode;
		soar_module::sqlite_statement *prev_episode;

		soar_module::sqlite_statement *get_nodes;
		soar_module::sqlite_statement *get_edges;

		//		
		
		epmem_graph_statement_container( agent *new_agent );
};


//////////////////////////////////////////////////////////
// Common Types
//////////////////////////////////////////////////////////

// represents a vector of times
typedef std::vector<epmem_time_id> epmem_time_list;

// represents a list of wmes
typedef std::list<wme *> epmem_wme_list;

// keeping state for multiple RIT's
typedef struct epmem_rit_state_param_struct
{
	soar_module::intptr_stat *stat;
	epmem_variable_key var_key;
} epmem_rit_state_param;

typedef struct epmem_rit_state_struct
{
	epmem_rit_state_param offset;
	epmem_rit_state_param leftroot;
	epmem_rit_state_param rightroot;
	epmem_rit_state_param minstep;

	soar_module::timer *timer;
	soar_module::sqlite_statement *add_query;	
} epmem_rit_state;

//////////////////////////////////////////////////////////
// Soar Integration Types
//////////////////////////////////////////////////////////

// data associated with each state
typedef struct epmem_data_struct
{
	unsigned long last_ol_time;		// last update to output-link
	unsigned long last_ol_count;	// last count of output-link

	unsigned long last_cmd_time;	// last update to epmem.command
	unsigned long last_cmd_count;	// last update to epmem.command

	epmem_time_id last_memory;		// last retrieved memory

	std::set<wme *> *cue_wmes;		// wmes in last cue
	std::stack<wme *> *epmem_wmes;	// wmes in last epmem
} epmem_data;


//////////////////////////////////////////////////////////
// Mode "one" Types (i.e. Working Memory Tree)
//////////////////////////////////////////////////////////

// represents a leaf wme
typedef struct epmem_leaf_node_struct
{
	double leaf_weight;						// wma value
	epmem_node_id leaf_id;					// node id
} epmem_leaf_node;

// maintains state within sqlite b-trees
typedef struct epmem_range_query_struct
{
	soar_module::sqlite_statement *stmt;	// query
	epmem_time_id val;						// current b-tree leaf value

	double weight;							// wma value
	long ct;								// cardinality w.r.t. positive/negative query
} epmem_range_query;

// functor to maintain a priority cue of b-tree pointers
// based upon their current value
struct epmem_compare_range_queries
{
	bool operator() ( const epmem_range_query *a, const epmem_range_query *b ) const
	{
		return ( a->val < b->val );
	}
};
typedef std::priority_queue<epmem_range_query *, std::vector<epmem_range_query *>, epmem_compare_range_queries> epmem_range_query_list;


//////////////////////////////////////////////////////////
// Mode "three" Types (i.e. Working Memory Graph)
//////////////////////////////////////////////////////////

// see below
typedef struct epmem_shared_literal_struct epmem_shared_literal;
typedef struct epmem_shared_literal_pair_struct epmem_shared_literal_pair;
typedef std::vector<epmem_shared_literal *> epmem_shared_literal_list;
typedef std::vector<epmem_shared_literal_pair *> epmem_shared_literal_pair_list;
typedef std::list<epmem_shared_literal_list::size_type> epmem_shared_wme_index;
typedef std::vector<wme *> epmem_shared_wme_list;
typedef struct epmem_shared_wme_counter_struct epmem_shared_wme_counter;
typedef std::map<epmem_node_id, unsigned long> epmem_shared_literal_counter;
typedef std::map<wme *, epmem_shared_wme_counter *> epmem_shared_wme_book;

// lookup tables to facilitate shared identifiers
typedef std::map<epmem_node_id, Symbol *> epmem_id_mapping;
typedef std::map<epmem_node_id, epmem_shared_literal *> epmem_literal_mapping;

// lookup table to propagate constrained identifiers during
// full graph-match
typedef std::map<Symbol *, epmem_node_id> epmem_constraint_list;
typedef std::map<epmem_node_id, Symbol *> epmem_reverse_constraint_list;

// types/structures to facilitate re-use of identifiers
typedef std::map<epmem_node_id, epmem_node_id> epmem_id_pool;
typedef std::map<epmem_node_id, epmem_id_pool *> epmem_hashed_id_pool;
typedef std::map<epmem_node_id, epmem_hashed_id_pool *> epmem_parent_id_pool;
typedef std::map<epmem_node_id, epmem_id_pool *> epmem_return_id_pool;
typedef std::map<epmem_node_id, uintptr_t> epmem_id_ref_counter;
typedef struct epmem_id_reservation_struct
{
	epmem_node_id my_id;
	epmem_hash_id my_hash;
	epmem_id_pool *my_pool;
} epmem_id_reservation;

// represents a graph edge (i.e. identifier)
// follows cs theory notation of finite automata: q1 = d( q0, w )
typedef struct epmem_edge_struct
{
	epmem_node_id q0;							// id
	Symbol *w;									// attr
	epmem_node_id q1;							// value
} epmem_edge;

// represents cached children of an identifier in working memory
typedef struct epmem_wme_cache_element_struct
{
	epmem_wme_list *wmes;						// child wmes	
	unsigned long parents;						// number of parents

	epmem_literal_mapping *lits;				// child literals
} epmem_wme_cache_element;

// represents state of a leaf wme
// at a particular episode
typedef struct epmem_shared_match_struct
{
	double value_weight;						// wma value
	long value_ct;								// cardinality w.r.t. positive/negative query

	unsigned long ct;							// number of contributing literals that are "on"
} epmem_shared_match;

// represents a list of literals grouped
// sequentially by cue wme
typedef struct epmem_shared_literal_group_struct
{
	epmem_shared_literal_pair_list *literals;	// vector of sequentially grouped literals
	epmem_shared_wme_index *wme_index;			// list of indexes to the start of wmes

	wme *c_wme;									// current wme (used during building and using groups)
} epmem_shared_literal_group;

struct epmem_shared_wme_counter_struct
{
	unsigned long wme_ct;
	epmem_shared_literal_counter *lit_ct;
};

// represents state of one historical
// identity of a cue wme at a particular
// episode
struct epmem_shared_literal_struct
{
	epmem_node_id shared_id;					// shared q1, if identifier

	unsigned long ct;							// number of contributing literals that are "on"
	unsigned long max;							// number of contributing literals that *need* to be on	
	epmem_shared_wme_book *wme_ct;				// bookkeeping to ensure literal count

	struct wme_struct *wme;						// associated cue wme
	bool wme_kids;								// does the cue wme have children (indicative of leaf wme status)	

	epmem_shared_match *match;					// associated match, if leaf wme
	epmem_shared_literal_group *children;		// grouped child literals, if not leaf wme
};

struct epmem_shared_literal_pair_struct
{
	epmem_node_id unique_id;
	epmem_node_id q0;
	epmem_node_id q1;

	struct wme_struct *wme;

	epmem_shared_literal *lit;
};

// maintains state within sqlite b-trees
typedef struct epmem_shared_query_struct
{
	soar_module::sqlite_statement *stmt;		// associated query
	epmem_time_id val;							// current b-tree leaf value

	epmem_node_id unique_id;					// id searched by this statement

	epmem_shared_literal_pair_list *triggers;	// literals to update when stepping this b-tree
} epmem_shared_query;

// functor to maintain a priority cue of b-tree pointers
// based upon their current value
struct epmem_compare_shared_queries
{
	bool operator() ( const epmem_shared_query *a, const epmem_shared_query *b ) const
	{
		return ( a->val < b->val );
	}
};
typedef std::priority_queue<epmem_shared_query *, std::vector<epmem_shared_query *>, epmem_compare_shared_queries> epmem_shared_query_list;

//
// These must go below types
//

#include "stl_support.h"

//////////////////////////////////////////////////////////
// Parameter Functions (see cpp for comments)
//////////////////////////////////////////////////////////

// shortcut for determining if EpMem is enabled
inline extern bool epmem_enabled( agent *my_agent );


//////////////////////////////////////////////////////////
// Soar Functions (see cpp for comments)
//////////////////////////////////////////////////////////

// init, end
extern void epmem_reset( agent *my_agent, Symbol *state = NULL );
extern void epmem_close( agent *my_agent );

// perform epmem actions
extern void epmem_go( agent *my_agent );

#endif