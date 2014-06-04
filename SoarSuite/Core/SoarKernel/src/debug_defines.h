/* -----------------------------------------------------------------------------
 *                                debug_defines.h
 * --------------- Compiler directives for debugging messages ------------------
 *
 *  Created on: Jul 17, 2013
 *      Author: Mazin
 * ----------------------------------------------------------------------------- */

#ifndef DEBUG_DEFINES_H_
#define DEBUG_DEFINES_H_

/* -- The schema version used by the output manager's debug database -- */
#define DEBUG_SCHEMA_VERSION "0.1"

//#define DEBUG_FREE_SETTINGS
//#define DEBUG_MINIMAL_SETTINGS
//#define DEBUG_REFCOUNT_SETTINGS
#define DEBUG_UNITTEST_SETTINGS

#ifdef DEBUG_FREE_SETTINGS
/* -- Which trace messages should be printed -- */

#define TRACE_Init_No_Mode                      true
#define TRACE_Init_TM_EPMEM                     true
#define TRACE_Init_TM_SMEM                      true
#define TRACE_Init_TM_LEARNING                  true
#define TRACE_Init_TM_CHUNKING                  true
#define TRACE_Init_TM_RL                        true
#define TRACE_Init_TM_WMA                       true

/* -- Which debug messages should be printed -- */
#define TRACE_Init_DT_No_Mode                     true
#define TRACE_Init_DT_DEBUG                       true
//--
#define TRACE_Init_DT_REFCOUNT_ADDS               false
#define TRACE_Init_DT_REFCOUNT_REMS               false
#define TRACE_Init_DT_DEALLOCATES                 false
#define TRACE_Init_DT_DEALLOCATE_SYMBOLS          false
#define TRACE_Init_DT_ID_LEAKING                  false
//--
#define TRACE_Init_DT_GDS                         false
#define TRACE_Init_DT_PARSER                      false
#define TRACE_Init_DT_FUNC_PRODUCTIONS            false
#define TRACE_Init_DT_PRINT_INSTANTIATIONS        false
//--
#define TRACE_Init_DT_ADD_TEST_TO_TEST            false
#define TRACE_Init_DT_ADD_CONSTRAINTS_ORIG_TESTS  false
#define TRACE_Init_DT_SAVEDVARS                   false
#define TRACE_Init_DT_UNIQUE_VARIABLIZATION       false
#define TRACE_Init_DT_REORDERER                   false
//--
#define TRACE_Init_DT_BACKTRACE                   false
#define TRACE_Init_DT_LHS_VARIABLIZATION          false
#define TRACE_Init_DT_RHS_VARIABLIZATION          false
#define TRACE_Init_DT_RL_VARIABLIZATION           false
#define TRACE_Init_DT_VARIABLIZATION_MANAGER      false
#define TRACE_Init_DT_VARIABLIZATION_REV          false
#define TRACE_Init_DT_NCC_VARIABLIZATION          false
#define TRACE_Init_DT_IDENTITY_PROP               false
#define TRACE_Init_DT_SOAR_INSTANCE               true
#define TRACE_Init_DT_CLI_LIBRARIES               true

/* -- Output can be sent to the following listeners: debug database,
 *    XML, print callback, stdout, and/or a file. -- */

/* -- Which output listeners should be initially turned on -- */
#define OM_Init_db_mode           off
#define OM_Init_XML_mode          off
#define OM_Init_callback_mode     on
#define OM_Init_stdout_mode       off
#define OM_Init_file_mode         off

/* -- Which output debug listeners should be initially turned on -- */
#define OM_Init_db_dbg_mode       off
#define OM_Init_XML_dbg_mode      off
#define OM_Init_callback_dbg_mode off
#define OM_Init_stdout_dbg_mode   on
#define OM_Init_file_dbg_mode     off
#endif

#ifdef DEBUG_MINIMAL_SETTINGS
/* ------------------------------------------------------------------------------------
 * -- MToDo | Remove these. Just a quick set of init settings for minimal debug msgs --
 * ------------------------------------------------------------------------------------ */


/* -- Which trace messages should be printed -- */

#define TRACE_Init_No_Mode                      true
#define TRACE_Init_TM_EPMEM                     true
#define TRACE_Init_TM_SMEM                      true
#define TRACE_Init_TM_LEARNING                  true
#define TRACE_Init_TM_CHUNKING                  true
#define TRACE_Init_TM_RL                        true
#define TRACE_Init_TM_WMA                       true

/* -- Which debug messages should be printed -- */
#define TRACE_Init_DT_No_Mode                     true
#define TRACE_Init_DT_DEBUG                       true
//--
#define TRACE_Init_DT_REFCOUNT_ADDS               false
#define TRACE_Init_DT_REFCOUNT_REMS               false
#define TRACE_Init_DT_DEALLOCATES                 false
#define TRACE_Init_DT_DEALLOCATE_SYMBOLS          false
#define TRACE_Init_DT_ID_LEAKING                  false
//--
#define TRACE_Init_DT_GDS                         false
#define TRACE_Init_DT_PARSER                      false
#define TRACE_Init_DT_FUNC_PRODUCTIONS            false
#define TRACE_Init_DT_PRINT_INSTANTIATIONS        false
//--
#define TRACE_Init_DT_ADD_TEST_TO_TEST            false
#define TRACE_Init_DT_ADD_CONSTRAINTS_ORIG_TESTS  false
#define TRACE_Init_DT_SAVEDVARS                   false
#define TRACE_Init_DT_UNIQUE_VARIABLIZATION       false
#define TRACE_Init_DT_REORDERER                   false
//--
#define TRACE_Init_DT_BACKTRACE                   false
#define TRACE_Init_DT_LHS_VARIABLIZATION          false
#define TRACE_Init_DT_RHS_VARIABLIZATION          false
#define TRACE_Init_DT_RL_VARIABLIZATION           false
#define TRACE_Init_DT_VARIABLIZATION_MANAGER      false
#define TRACE_Init_DT_VARIABLIZATION_REV          false
#define TRACE_Init_DT_NCC_VARIABLIZATION          false
#define TRACE_Init_DT_IDENTITY_PROP               false
#define TRACE_Init_DT_SOAR_INSTANCE               false
#define TRACE_Init_DT_CLI_LIBRARIES               false

/* -- Output can be sent to the following listeners: debug database,
 *    XML, print callback, stdout, and/or a file. -- */

/* -- Which output listeners should be initially turned on -- */
#define OM_Init_db_mode           off
#define OM_Init_XML_mode          off
#define OM_Init_callback_mode     off
#define OM_Init_stdout_mode       on
#define OM_Init_file_mode         off

/* -- Which output debug listeners should be initially turned on -- */
#define OM_Init_db_dbg_mode       off
#define OM_Init_XML_dbg_mode      off
#define OM_Init_callback_dbg_mode off
#define OM_Init_stdout_dbg_mode   on
#define OM_Init_file_dbg_mode     off
#endif
#ifdef DEBUG_REFCOUNT_SETTINGS
/* ----------------------------------------------------------------------------------
 * -- MToDo | Remove these. Just a quick set of init settings for refcount tracing --
 * ---------------------------------------------------------------------------------- */

/* -- Which trace messages should be printed -- */

#define TRACE_Init_No_Mode                      true
#define TRACE_Init_TM_EPMEM                     true
#define TRACE_Init_TM_SMEM                      true
#define TRACE_Init_TM_LEARNING                  true
#define TRACE_Init_TM_CHUNKING                  true
#define TRACE_Init_TM_RL                        true
#define TRACE_Init_TM_WMA                       true

/* -- Which debug messages should be printed -- */
#define TRACE_Init_DT_No_Mode                     true
#define TRACE_Init_DT_DEBUG                       true
//--
#define TRACE_Init_DT_REFCOUNT_ADDS               false
#define TRACE_Init_DT_REFCOUNT_REMS               false
#define TRACE_Init_DT_DEALLOCATES                 true
#define TRACE_Init_DT_DEALLOCATE_SYMBOLS          true
#define TRACE_Init_DT_ID_LEAKING                  true
//--
#define TRACE_Init_DT_GDS                         true
#define TRACE_Init_DT_PARSER                      false
#define TRACE_Init_DT_FUNC_PRODUCTIONS            true
#define TRACE_Init_DT_PRINT_INSTANTIATIONS        true
//--
#define TRACE_Init_DT_ADD_TEST_TO_TEST            true
#define TRACE_Init_DT_ADD_CONSTRAINTS_ORIG_TESTS  true
#define TRACE_Init_DT_SAVEDVARS                   true
#define TRACE_Init_DT_UNIQUE_VARIABLIZATION       true
#define TRACE_Init_DT_REORDERER                   true
//--
#define TRACE_Init_DT_BACKTRACE                   true
#define TRACE_Init_DT_LHS_VARIABLIZATION          true
#define TRACE_Init_DT_RHS_VARIABLIZATION          true
#define TRACE_Init_DT_RL_VARIABLIZATION           true
#define TRACE_Init_DT_VARIABLIZATION_MANAGER      true
#define TRACE_Init_DT_VARIABLIZATION_REV          true
#define TRACE_Init_DT_NCC_VARIABLIZATION          true
#define TRACE_Init_DT_IDENTITY_PROP               true
#define TRACE_Init_DT_SOAR_INSTANCE               true
#define TRACE_Init_DT_CLI_LIBRARIES               true

/* -- Output can be sent to the following listeners: debug database,
 *    XML, print callback, stdout, and/or a file. -- */

/* -- Which output listeners should be initially turned on -- */
#define OM_Init_db_mode           on
#define OM_Init_XML_mode          off
#define OM_Init_callback_mode     off
#define OM_Init_stdout_mode       on
#define OM_Init_file_mode         off

/* -- Which output debug listeners should be initially turned on -- */
#define OM_Init_db_dbg_mode       on
#define OM_Init_XML_dbg_mode      off
#define OM_Init_callback_dbg_mode off
#define OM_Init_stdout_dbg_mode   off
#define OM_Init_file_dbg_mode     off
#endif
#ifdef DEBUG_UNITTEST_SETTINGS
/* ------------------------------------------------------------------------------------
 * -- MToDo | Remove these. Just a quick set of init settings for unit tests and TSP --
 * ------------------------------------------------------------------------------------ */

/* -- Which trace messages should be printed -- */

#define TRACE_Init_No_Mode                      true
#define TRACE_Init_TM_EPMEM                     true
#define TRACE_Init_TM_SMEM                      true
#define TRACE_Init_TM_LEARNING                  true
#define TRACE_Init_TM_CHUNKING                  true
#define TRACE_Init_TM_RL                        true
#define TRACE_Init_TM_WMA                       true

/* -- Which debug messages should be printed -- */
#define TRACE_Init_DT_No_Mode                     false
#define TRACE_Init_DT_DEBUG                       false
//--
#define TRACE_Init_DT_REFCOUNT_ADDS               false
#define TRACE_Init_DT_REFCOUNT_REMS               false
#define TRACE_Init_DT_DEALLOCATES                 false
#define TRACE_Init_DT_DEALLOCATE_SYMBOLS          false
#define TRACE_Init_DT_ID_LEAKING                  false
//--
#define TRACE_Init_DT_GDS                         false
#define TRACE_Init_DT_PARSER                      false
#define TRACE_Init_DT_FUNC_PRODUCTIONS            false
#define TRACE_Init_DT_PRINT_INSTANTIATIONS        false
//--
#define TRACE_Init_DT_ADD_TEST_TO_TEST            false
#define TRACE_Init_DT_ADD_CONSTRAINTS_ORIG_TESTS  false
#define TRACE_Init_DT_SAVEDVARS                   false
#define TRACE_Init_DT_UNIQUE_VARIABLIZATION       false
#define TRACE_Init_DT_REORDERER                   false
//--
#define TRACE_Init_DT_BACKTRACE                   false
#define TRACE_Init_DT_LHS_VARIABLIZATION          false
#define TRACE_Init_DT_RHS_VARIABLIZATION          false
#define TRACE_Init_DT_RL_VARIABLIZATION           false
#define TRACE_Init_DT_VARIABLIZATION_MANAGER      false
#define TRACE_Init_DT_VARIABLIZATION_REV          false
#define TRACE_Init_DT_NCC_VARIABLIZATION          false
#define TRACE_Init_DT_IDENTITY_PROP               false
#define TRACE_Init_DT_SOAR_INSTANCE               false
#define TRACE_Init_DT_CLI_LIBRARIES               false

/* -- Output can be sent to the following listeners: debug database,
 *    XML, print callback, stdout, and/or a file. -- */

/* -- Which output listeners should be initially turned on -- */
#define OM_Init_db_mode           off
#define OM_Init_XML_mode          off
#define OM_Init_callback_mode     on
#define OM_Init_stdout_mode       off
#define OM_Init_file_mode         off

/* -- Which output debug listeners should be initially turned on -- */
#define OM_Init_db_dbg_mode       off
#define OM_Init_XML_dbg_mode      off
#define OM_Init_callback_dbg_mode off
#define OM_Init_stdout_dbg_mode   off
#define OM_Init_file_dbg_mode     off
#endif

#endif /* DEBUG_DEFINES_H_ */
