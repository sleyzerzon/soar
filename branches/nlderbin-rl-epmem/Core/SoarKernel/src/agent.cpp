#include <portability.h>

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*************************************************************************
 *
 *  file:  agent.cpp
 *
 * =======================================================================
 *  Initialization for the agent structure.  Also the cleanup routine
 *  when an agent is destroyed.  These routines are usually replaced
 *  by the same-named routines in the Tcl interface file soarAgent.c
 *  The versions in this file are used only when not linking in Tcl.
 *  HOWEVER, this code should be maintained, and the agent structure
 *  must be kept up to date.
 * =======================================================================
 */

#include <stdlib.h>
#include <map>

#include "agent.h"
#include "kernel.h"
#include "mem.h"
#include "lexer.h"
#include "symtab.h"
#include "gdatastructs.h"
#include "rhsfun.h"
#include "instantiations.h"
#include "production.h"
#include "gsysparam.h"
#include "init_soar.h"
#include "decide.h"
#include "print.h"
#include "recmem.h"
#include "backtrace.h"
#include "chunk.h"
#include "explain.h"
#include "rete.h"
#include "trace.h"
#include "callback.h"
#include "io_soar.h"
#include "kernel_struct.h"
#include "exploration.h"
#include "reinforcement_learning.h"
#include "decision_manipulation.h"
#include "episodic_memory.h"
#include "sqlite3.h"

/* JC ADDED: Need to initialize gski callbacks */
#include "gski_event_system_functions.h"


/* ================================================================== */

char * soar_version_string;

/* ===================================================================
   
                           Initialization Function

=================================================================== */

void init_soar_agent(Kernel* thisKernel, agent* thisAgent) {

  /* Updated this from soar_agent to thisAgent. -AJC (8/8/02) */
  /* JC ADDED: Initialize the gski callbacks. 
     This is mildly frightening.  I hope soar_agent is set correctly
      by this point
  */
  gSKI_InitializeAgentCallbacks(thisAgent);

  /* JC ADDED: link the agent to its kernel */
  thisAgent->kernel = thisKernel;

  /* JC ADDED: initialize the rhs function linked list */
  thisAgent->rhs_functions = NIL;

   /* --- initialize everything --- */
  init_symbol_tables(thisAgent);
  create_predefined_symbols(thisAgent);
  init_production_utilities(thisAgent);
  init_built_in_rhs_functions (thisAgent);
  init_rete (thisAgent);
  init_lexer (thisAgent);
  init_firer (thisAgent);
  init_decider (thisAgent);
  init_soar_io (thisAgent);
  init_chunker (thisAgent);
  init_tracing (thisAgent);
  init_explain(thisAgent);  /* AGR 564 */
  init_select(thisAgent);
  init_predict(thisAgent);

#ifdef REAL_TIME_BEHAVIOR
  /* RMJ */
  init_real_time(thisAgent);
#endif


  /* --- add default object trace formats --- */
  add_trace_format (thisAgent, FALSE, FOR_ANYTHING_TF, NIL,
                    "%id %ifdef[(%v[name])]");
  add_trace_format (thisAgent, FALSE, FOR_STATES_TF, NIL,
                    "%id %ifdef[(%v[attribute] %v[impasse])]");
  { Symbol *evaluate_object_sym;
    evaluate_object_sym = make_sym_constant (thisAgent, "evaluate-object");
    add_trace_format (thisAgent, FALSE, FOR_OPERATORS_TF, evaluate_object_sym,
                      "%id (evaluate-object %o[object])");
    symbol_remove_ref (thisAgent, evaluate_object_sym);
  }
  /* --- add default stack trace formats --- */
  add_trace_format (thisAgent, TRUE, FOR_STATES_TF, NIL,
                    "%right[6,%dc]: %rsd[   ]==>S: %cs");
  add_trace_format (thisAgent, TRUE, FOR_OPERATORS_TF, NIL,
                    "%right[6,%dc]: %rsd[   ]   O: %co");

  reset_statistics (thisAgent);
   
  /* RDF: For gSKI */
  init_agent_memory(thisAgent);
  /* END */

}


/*
===============================

===============================
*/
agent * create_soar_agent (Kernel * thisKernel, char * agent_name) {                                          /* loop index */
  char cur_path[MAXPATHLEN];   /* AGR 536 */

  agent* newAgent = (agent *) malloc(sizeof(agent));

  newAgent->current_tc_number = 0;

  thisKernel->agent_counter++;
  thisKernel->agent_count++;

  newAgent->name                               = savestring(agent_name);

  /* mvp 5-17-94 */
  newAgent->variables_set                      = NIL;

//#ifdef _WINDOWS
//  newAgent->current_line[0]                    = 0;
//  newAgent->current_line_index                 = 0;
//#endif /* _WINDOWS */
  /* String redirection */
  newAgent->using_output_string                = FALSE;
  newAgent->using_input_string                 = FALSE;
  newAgent->output_string                      = NIL;
  newAgent->input_string                       = NIL;

  newAgent->alias_list                         = NIL;  /* AGR 568 */
  newAgent->all_wmes_in_rete                   = NIL;
  newAgent->alpha_mem_id_counter               = 0;
  newAgent->alternate_input_string             = NIL;
  newAgent->alternate_input_suffix             = NIL;
  newAgent->alternate_input_exit               = FALSE;/* Soar-Bugs #54 */
  newAgent->backtrace_number                   = 0;
  newAgent->beta_node_id_counter               = 0;
  newAgent->bottom_goal                        = NIL;
  newAgent->changed_slots                      = NIL;
  newAgent->chunk_count                        = 1;
  newAgent->chunk_free_problem_spaces          = NIL;
  newAgent->chunky_problem_spaces              = NIL;  /* AGR MVL1 */
  strcpy(newAgent->chunk_name_prefix,"chunk");	/* ajc (5/14/02) */
  newAgent->context_slots_with_changed_acceptable_preferences = NIL;
  newAgent->current_file                       = NIL;
  newAgent->current_phase                      = INPUT_PHASE;
  newAgent->applyPhase                         = FALSE;
  newAgent->current_symbol_hash_id             = 0;
  newAgent->current_variable_gensym_number     = 0;
  newAgent->current_wme_timetag                = 1;
  newAgent->default_wme_depth                  = 1;  /* AGR 646 */
  newAgent->disconnected_ids                   = NIL;
  newAgent->existing_output_links              = NIL;
  newAgent->output_link_changed                = FALSE;  /* KJC 11/9/98 */
  /* newAgent->explain_flag                       = FALSE; */
  newAgent->go_number                          = 1;
  newAgent->go_type                            = GO_DECISION;
  newAgent->grounds_tc                         = 0;
  newAgent->highest_goal_whose_context_changed = NIL;
  newAgent->ids_with_unknown_level             = NIL;
  newAgent->input_period                       = 0;     /* AGR REW1 */
  newAgent->input_cycle_flag                   = TRUE;  /* AGR REW1 */
  newAgent->justification_count                = 1;
  newAgent->lex_alias                          = NIL;  /* AGR 568 */
  newAgent->link_update_mode                   = UPDATE_LINKS_NORMALLY;
  newAgent->locals_tc                          = 0;
  newAgent->logging_to_file                    = FALSE;
  newAgent->max_chunks_reached                 = FALSE; /* MVP 6-24-94 */
  newAgent->mcs_counter                        = 1;
  newAgent->memory_pools_in_use                = NIL;
  newAgent->ms_assertions                      = NIL;
  newAgent->ms_retractions                     = NIL;
  newAgent->num_existing_wmes                  = 0;
  newAgent->num_wmes_in_rete                   = 0;
  newAgent->potentials_tc                      = 0;
  newAgent->prev_top_state                     = NIL;
  newAgent->print_prompt_flag                  = TRUE;
  newAgent->printer_output_column              = 1;
  newAgent->production_being_fired             = NIL;
  newAgent->productions_being_traced           = NIL; 
  newAgent->promoted_ids                       = NIL;
  newAgent->reason_for_stopping                = "Startup";
  newAgent->redirecting_to_file                = FALSE;
  newAgent->replay_input_data                  = FALSE;
  newAgent->slots_for_possible_removal         = NIL;
  newAgent->stop_soar                          = TRUE;           
  newAgent->system_halted                      = FALSE;
  newAgent->token_additions                    = 0;
  newAgent->top_dir_stack                      = NIL;   /* AGR 568 */
  newAgent->top_goal                           = NIL;
  newAgent->top_state                          = NIL;
  newAgent->wmes_to_add                        = NIL;
  newAgent->wmes_to_remove                     = NIL;
  newAgent->wme_filter_list                    = NIL;   /* Added this to avoid
															    access violation
																-AJC (5/13/02) */
  newAgent->multi_attributes                   = NIL;

  /* REW: begin 09.15.96 */

  newAgent->did_PE                             = FALSE;
  newAgent->operand2_mode                      = TRUE;
  newAgent->soar_verbose_flag                  = FALSE;
  newAgent->FIRING_TYPE                        = IE_PRODS;
  newAgent->ms_o_assertions                    = NIL;
  newAgent->ms_i_assertions                    = NIL;

  /* REW: end   09.15.96 */

  /* REW: begin 08.20.97 */
  newAgent->active_goal                        = NIL;
  newAgent->active_level                       = 0;
  newAgent->previous_active_level              = 0;

  /* Initialize Waterfall-specific lists */
  newAgent->nil_goal_retractions               = NIL;
  /* REW: end   08.20.97 */

  /* REW: begin 10.24.97 */
  newAgent->waitsnc                            = FALSE;
  newAgent->waitsnc_detect                     = FALSE;
  /* REW: end   10.24.97 */

  /* Initializing rete stuff */
  for (int i=0; i < 256; i++) {
     newAgent->actual[i]=0;
     newAgent->if_no_merging[i]=0;
     newAgent->if_no_sharing[i]=0;
  }

  /* Initializing lexeme */
  newAgent->lexeme.type = NULL_LEXEME;
  newAgent->lexeme.string[0] = 0;
  newAgent->lexeme.length = 0;
  newAgent->lexeme.int_val = 0;
  newAgent->lexeme.float_val = 0.0;
  newAgent->lexeme.id_letter = 'A';
  newAgent->lexeme.id_number = 0;

  /* Initializing all the timer structures */
  reset_timer(&(newAgent->start_total_tv));
  reset_timer(&(newAgent->total_cpu_time));
  reset_timer(&(newAgent->start_kernel_tv));
  reset_timer(&(newAgent->start_phase_tv));
  reset_timer(&(newAgent->total_kernel_time));

  reset_timer(&(newAgent->input_function_cpu_time));
  reset_timer(&(newAgent->output_function_cpu_time));
  reset_timer(&(newAgent->start_gds_tv));
  reset_timer(&(newAgent->total_gds_time));

  for (int ii=0;ii < NUM_PHASE_TYPES; ii++) {
     reset_timer(&(newAgent->decision_cycle_phase_timers[ii]));
     reset_timer(&(newAgent->monitors_cpu_time[ii]));
     reset_timer(&(newAgent->ownership_cpu_time[ii]));
     reset_timer(&(newAgent->chunking_cpu_time[ii]));
     reset_timer(&(newAgent->match_cpu_time[ii]));
     reset_timer(&(newAgent->gds_cpu_time[ii]));
  }

  newAgent->real_time_tracker = 0;
  newAgent->attention_lapse_tracker = 0;


  if(!getcwd(cur_path, MAXPATHLEN))
    print(newAgent, "Unable to set current directory while initializing agent.\n");
  newAgent->top_dir_stack = (dir_stack_struct *) malloc(sizeof(dir_stack_struct));   /* AGR 568 */
  newAgent->top_dir_stack->directory = (char *) malloc(MAXPATHLEN*sizeof(char));   /* AGR 568 */
  newAgent->top_dir_stack->next = NIL;   /* AGR 568 */
  strcpy(newAgent->top_dir_stack->directory, cur_path);   /* AGR 568 */

  /* changed all references of 'i', a var belonging to a previous for loop, to 'productionTypeCounter' to be unique 
    stokesd Sept 10 2004*/
  for (int productionTypeCounter=0; productionTypeCounter<NUM_PRODUCTION_TYPES; productionTypeCounter++) {  
    newAgent->all_productions_of_type[productionTypeCounter] = NIL;
    newAgent->num_productions_of_type[productionTypeCounter] = 0;
  }

  newAgent->o_support_calculation_type = 4; /* KJC 7/00 */ // changed from 3 to 4 by voigtjr  (/* bugzilla bug 339 */)
//#ifdef NUMERIC_INDIFFERENCE
  newAgent->numeric_indifferent_mode = NUMERIC_INDIFFERENT_MODE_SUM;
//#endif
  newAgent->attribute_preferences_mode = 0; /* RBD 4/17/95 */

   /* JC ADDED: Make sure that the RHS functions get initialized correctly */
   newAgent->rhs_functions = NIL;

  soar_init_callbacks((soar_callback_agent) newAgent);

  //
  // This call is needed to set up callbacks.
  init_memory_utilities(newAgent);

  //
  // This was moved here so that system parameters could
  // be set before the agent was initialized.
  init_sysparams (newAgent);

  
  // exploration initialization
  newAgent->exploration_params[ EXPLORATION_PARAM_EPSILON ] = add_exploration_parameter( 0.1, &validate_epsilon, "epsilon" );
  newAgent->exploration_params[ EXPLORATION_PARAM_TEMPERATURE ] = add_exploration_parameter( 25, &validate_temperature, "temperature" );
  
  // rl initialization
  newAgent->rl_params[ RL_PARAM_LEARNING ] = add_rl_parameter( "learning", RL_LEARNING_ON, &validate_rl_learning, &convert_rl_learning, &convert_rl_learning );    
  newAgent->rl_params[ RL_PARAM_DISCOUNT_RATE ] = add_rl_parameter( "discount-rate", 0.9, &validate_rl_discount );
  newAgent->rl_params[ RL_PARAM_LEARNING_RATE ] = add_rl_parameter( "learning-rate", 0.3, &validate_rl_learning_rate );
  newAgent->rl_params[ RL_PARAM_LEARNING_POLICY ] = add_rl_parameter( "learning-policy", RL_LEARNING_SARSA, &validate_rl_learning_policy, &convert_rl_learning_policy, &convert_rl_learning_policy );
  newAgent->rl_params[ RL_PARAM_ET_DECAY_RATE ] = add_rl_parameter( "eligibility-trace-decay-rate", 0, &validate_rl_decay_rate );
  newAgent->rl_params[ RL_PARAM_ET_TOLERANCE ] = add_rl_parameter( "eligibility-trace-tolerance", 0.001, &validate_rl_trace_tolerance );
  newAgent->rl_params[ RL_PARAM_TEMPORAL_EXTENSION ] = add_rl_parameter( "temporal-extension", RL_TE_ON, &validate_te_enabled, &convert_te_enabled, &convert_te_enabled );

  newAgent->rl_stats[ RL_STAT_UPDATE_ERROR ] = add_rl_stat( "update-error" );
  newAgent->rl_stats[ RL_STAT_TOTAL_REWARD ] = add_rl_stat( "total-reward" );
  newAgent->rl_stats[ RL_STAT_GLOBAL_REWARD ] = add_rl_stat( "global-reward" );

  initialize_template_tracking( newAgent );
  
  // select initialization
  newAgent->select = new select_info;
  init_select( newAgent );

  // predict initialization
  newAgent->prediction = new std::string();
  init_predict( newAgent );
  
  // epmem initialization
  newAgent->epmem_params[ EPMEM_PARAM_LEARNING ] = epmem_add_parameter( "learning", EPMEM_LEARNING_ON, &epmem_validate_learning, &epmem_convert_learning, &epmem_convert_learning );
  newAgent->epmem_params[ EPMEM_PARAM_DB ] = epmem_add_parameter( "database", EPMEM_DB_FILE, &epmem_validate_database, &epmem_convert_database, &epmem_convert_database );
  newAgent->epmem_params[ EPMEM_PARAM_PATH ] = epmem_add_parameter( "path", "", &epmem_validate_path );
    
  newAgent->epmem_params[ EPMEM_PARAM_INDEXING ] = epmem_add_parameter( "indexing", EPMEM_INDEXING_BIGTREE_RANGE, &epmem_validate_indexing, &epmem_convert_indexing, &epmem_convert_indexing );
  newAgent->epmem_params[ EPMEM_PARAM_PROVENANCE ] = epmem_add_parameter( "provenance", EPMEM_PROVENANCE_OFF, &epmem_validate_provenance, &epmem_convert_provenance, &epmem_convert_provenance );
    
  newAgent->epmem_params[ EPMEM_PARAM_TRIGGER ] = epmem_add_parameter( "trigger", EPMEM_TRIGGER_OUTPUT, &epmem_validate_trigger, &epmem_convert_trigger, &epmem_convert_trigger );
  newAgent->epmem_params[ EPMEM_PARAM_BALANCE ] = epmem_add_parameter( "balance", 0.5, &epmem_validate_balance );

  newAgent->epmem_stats[ EPMEM_STAT_TIME ] = epmem_add_stat( "time" );
  newAgent->epmem_stats[ EPMEM_STAT_MEM_USAGE ] = epmem_add_stat( "mem_usage" );
  newAgent->epmem_stats[ EPMEM_STAT_MEM_HIGH ] = epmem_add_stat( "mem_high" );

  newAgent->epmem_stats[ EPMEM_STAT_RIT_OFFSET ] = epmem_add_stat( "rit_offset" );
  newAgent->epmem_stats[ EPMEM_STAT_RIT_LEFTROOT ] = epmem_add_stat( "rit_left_root" );
  newAgent->epmem_stats[ EPMEM_STAT_RIT_RIGHTROOT ] = epmem_add_stat( "rit_right_root" );
  newAgent->epmem_stats[ EPMEM_STAT_RIT_MINSTEP ] = epmem_add_stat( "rit_min_step" );
    
  newAgent->epmem_db = NULL;
  newAgent->epmem_db_status = -1;
  for ( int i=0; i<EPMEM_MAX_STATEMENTS; i++ )
  	newAgent->epmem_statements[ i ] = NULL;

  newAgent->epmem_range_removals = new std::map<unsigned long, bool>();
  newAgent->epmem_range_mins = new std::vector<long>();
  newAgent->epmem_range_maxes = new std::vector<long>();

  return newAgent;
}

/*
===============================

===============================
*/
void initialize_soar_agent(Kernel *thisKernel, agent* thisAgent) {

  init_soar_agent(thisKernel, thisAgent);
                                         /* Add agent to global list   */
                                         /* of all agents.             */
  push(thisAgent, thisAgent, thisKernel->all_soar_agents);

  soar_invoke_callbacks(thisAgent, thisAgent, 
			AFTER_INIT_AGENT_CALLBACK,
			(soar_call_data) NULL);
}

/*
===============================

===============================
*/
void destroy_soar_agent (Kernel * thisKernel, agent * delete_agent)
{
  cons  * c;
  cons  * prev = NULL;   /* Initialized to placate gcc -Wall */
  agent * the_agent;
 
  //print(delete_agent, "\nDestroying agent %s.\n", delete_agent->name);  /* AGR 532 */

//#ifdef USE_X_DISPLAY
//
//  /* Destroy X window associated with agent */
//  destroy_agent_window (delete_agent);
//#endif /* USE_X_DISPLAY */

  remove_built_in_rhs_functions(delete_agent);

  /* Splice agent structure out of global list of agents. */
  for (c = thisKernel->all_soar_agents; c != NIL; c = c->rest) {  
	  the_agent = (agent *) c->first;
	  if (the_agent == delete_agent) {
		  if (c == thisKernel->all_soar_agents) {
			  thisKernel->all_soar_agents = c->rest;
		  } else {
			  prev->rest = c->rest;
		  }
		  free_cons(the_agent, c);  // RPM 11/06
		  break;
	  }
	  prev = c;
  }

  /* Free structures stored in agent structure */
  free(delete_agent->name);
  free(delete_agent->top_dir_stack->directory);
  free(delete_agent->top_dir_stack);

  /* Freeing the agent's multi attributes structure */
  multi_attribute* lastmattr = 0;
  for ( multi_attribute* curmattr = delete_agent->multi_attributes;
        curmattr != 0;
        curmattr = curmattr->next ) {
     
     symbol_remove_ref(delete_agent, curmattr->symbol);
     
     free_memory(delete_agent, (void*) lastmattr, MISCELLANEOUS_MEM_USAGE);
     lastmattr = curmattr;
  }
  free_memory(delete_agent, lastmattr, MISCELLANEOUS_MEM_USAGE);

  /* Freeing all the productions owned by this agent */
  excise_all_productions(delete_agent, false);

  /* Releasing all the predefined symbols */
  release_predefined_symbols(delete_agent);

  /* Releasing rete stuff RPM 11/06 */
  free_with_pool(&delete_agent->rete_node_pool, delete_agent->dummy_top_node);
  free_with_pool(&delete_agent->token_pool, delete_agent->dummy_top_token);

  /* Cleaning up the various callbacks 
     TODO: Not clear why callbacks need to take the agent pointer essentially twice.
  */
  soar_remove_all_monitorable_callbacks(delete_agent, (void*) delete_agent);

  /* RPM 9/06 begin */

  free_memory(delete_agent, delete_agent->left_ht, HASH_TABLE_MEM_USAGE);
  free_memory(delete_agent, delete_agent->right_ht, HASH_TABLE_MEM_USAGE);
  free_memory(delete_agent, delete_agent->rhs_variable_bindings, MISCELLANEOUS_MEM_USAGE);

  /* Releasing memory allocated in inital call to start_lex_from_file from init_lexer */
  free_memory_block_for_string(delete_agent, delete_agent->current_file->filename);
  free_memory (delete_agent, delete_agent->current_file, MISCELLANEOUS_MEM_USAGE);

  /* Releasing trace formats (needs to happen before tracing hashtables are released) */
  remove_trace_format (delete_agent, FALSE, FOR_ANYTHING_TF, NIL);
  remove_trace_format (delete_agent, FALSE, FOR_STATES_TF, NIL);
  Symbol *evaluate_object_sym = find_sym_constant (delete_agent, "evaluate-object");
  remove_trace_format (delete_agent, FALSE, FOR_OPERATORS_TF, evaluate_object_sym);
  remove_trace_format (delete_agent, TRUE, FOR_STATES_TF, NIL);
  remove_trace_format (delete_agent, TRUE, FOR_OPERATORS_TF, NIL);

  /* Releasing hashtables allocated in init_tracing */
  for (int i=0; i<3; i++) {
	free_hash_table(delete_agent, delete_agent->object_tr_ht[i]);
	free_hash_table(delete_agent, delete_agent->stack_tr_ht[i]);
  }

  /* Releasing memory allocated in init_rete */
  for (int i=0; i<16; i++) {
	  free_hash_table(delete_agent, delete_agent->alpha_hash_tables[i]);
  }

  /* Releasing other hashtables */
  free_hash_table(delete_agent, delete_agent->variable_hash_table);
  free_hash_table(delete_agent, delete_agent->identifier_hash_table);
  free_hash_table(delete_agent, delete_agent->sym_constant_hash_table);
  free_hash_table(delete_agent, delete_agent->int_constant_hash_table);
  free_hash_table(delete_agent, delete_agent->float_constant_hash_table);

  /* Releasing memory pools */
  memory_pool* cur_pool = delete_agent->memory_pools_in_use;
  memory_pool* next_pool;
  while(cur_pool != NIL) {
	  next_pool = cur_pool->next;
	  free_memory_pool(delete_agent, cur_pool);
	  cur_pool = next_pool;
  }

  /* RPM 9/06 end */

  // cleanup exploration
  for ( int i=0; i<EXPLORATION_PARAMS; i++ )
	  delete delete_agent->exploration_params[ i ];

  // cleanup Soar-RL
  clean_parameters( delete_agent );
  clean_stats( delete_agent );
  
  // cleanup select
  init_select( delete_agent );
  delete delete_agent->select;

  // cleanup predict
  delete delete_agent->prediction;
  
  // cleanup EpMem
  delete delete_agent->epmem_range_removals;
  delete delete_agent->epmem_range_mins;
  delete delete_agent->epmem_range_maxes;
  epmem_end( delete_agent );
  epmem_clean_parameters( delete_agent );

  /* Free soar agent structure */
  free((void *) delete_agent);
 
  thisKernel->agent_count--;
}
