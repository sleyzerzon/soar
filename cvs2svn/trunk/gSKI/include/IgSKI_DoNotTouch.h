/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_donottouch.h
*********************************************************************
* created:	   6/27/2002   10:44
*
* purpose: 
*********************************************************************/
#ifndef IGSKI_DONOTTOUCH_H
#define IGSKI_DONOTTOUCH_H

typedef struct production_struct production;
typedef unsigned char wme_trace_type;
typedef struct rete_node_struct rete_node;
typedef unsigned char ms_trace_type;
typedef struct agent_struct agent;

typedef void * soar_callback_agent;

namespace gSKI
{
   class IAgent;
   class IKernel;
   namespace EvilBackDoor 
   {
      class ITgDWorkArounds
      {
      public:
         
      /**
          * @brief
          */
         virtual ~ITgDWorkArounds(){}
         
         /**
          * @brief
          */
         ITgDWorkArounds(){}

         /**
          * @brief
          */
         virtual void SetSysparam (IAgent* agent, int param_number, long new_value) = 0;
         
         /**
          * @brief
          */
         virtual long GetSysparam(IAgent* agent, int param_number) = 0;

         /**
          * @brief
          */
         virtual const long* GetSysparams(IAgent* agnet) = 0;

         /**
          * @brief
          */
         virtual rete_node* NameToProduction(IAgent* agent, char* string_to_test) = 0;

         /**
         * @brief
         */
         virtual void PrintPartialMatchInformation(IAgent* thisAgent, 
                                                   struct rete_node_struct *p_node,
                                                   wme_trace_type wtt) = 0;
         /**
         * @brief
         */
        virtual void PrintMatchSet(IAgent* thisAgent, wme_trace_type wtt, ms_trace_type  mst) = 0;

        /**
         * @brief 
         */
        virtual void PrintStackTrace(IAgent* thisAgent, bool print_states, bool print_operators) = 0;

        /**
         * @brief 
         */
        virtual void PrintSymbol(IAgent*     thisAgent, 
                                 char*       arg, 
                                 bool        name_only, 
                                 bool        print_filename, 
                                 bool        internal,
                                 bool        full_prod,
                                 int         depth) = 0;

        /**
         * @brief 
         */
        virtual void PrintUser(IAgent*       thisAgent,
                               char*         arg,
                               bool          internal,
                               bool          print_filename,
                               bool          full_prod,
                               unsigned int  prodType) = 0;

        /**
         * @brief 
         */
        virtual bool Preferences(IAgent*     thisAgent,
                                 int         argc,
                                 char*       argv[])=0;

        /**
         * @brief 
         */
        virtual bool ProductionFind(IAgent*     thisAgent,
                                    agent*      agnt,
                                    IKernel*    kernel,
                                    bool        lhs,
                                    bool        rhs,
                                    char*       arg,
                                    bool        show_bindings,
                                    bool        just_chunks,
                                    bool        no_chunks)=0;
        /**
         * @brief 
         */
        virtual bool GDSPrint(IAgent* thisAgent) = 0;

      };
   }
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
////                                                                      ////
////  These defines are a total HACK!!  They are copied from the          ////
////  gsysparams.h file.                                                  //// 
////                                                                      ////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
/* -------------------------------
      Types of Productions
------------------------------- */

#define USER_PRODUCTION_TYPE 0
#define DEFAULT_PRODUCTION_TYPE 1
#define CHUNK_PRODUCTION_TYPE 2
#define JUSTIFICATION_PRODUCTION_TYPE 3

#define NUM_PRODUCTION_TYPES 4

/* ---------------------------------------
    Match Set print parameters
--------------------------------------- */

#define MS_ASSERT_RETRACT 0      /* print both retractions and assertions */
#define MS_ASSERT         1      /* print just assertions */
#define MS_RETRACT        2      /* print just retractions */

typedef unsigned char byte;
typedef byte ms_trace_type;   /* must be one of the above constants */

/* ---------------------------------------
    How much information to print about
    the wmes matching an instantiation
--------------------------------------- */

#define NONE_WME_TRACE    1      /* don't print anything */
#define TIMETAG_WME_TRACE 2      /* print just timetag */
#define FULL_WME_TRACE    3      /* print whole wme */
#define NO_WME_TRACE_SET  4

typedef byte wme_trace_type;   /* must be one of the above constants */


/* ---------------------------
   And now, the sysparam's
--------------------------- */

/* ====== Sysparams for what to trace === */

#define TRACE_CONTEXT_DECISIONS_SYSPARAM          1
#define TRACE_PHASES_SYSPARAM                     2

/* --- Warning: these next four MUST be consecutive and in the order of the
   production types defined above --- */
#define TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM      3
#define TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM   4
#define TRACE_FIRINGS_OF_CHUNKS_SYSPARAM          5
#define TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM  6

#define TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM     7
#define TRACE_FIRINGS_PREFERENCES_SYSPARAM        8
#define TRACE_WM_CHANGES_SYSPARAM                 9
#define TRACE_CHUNK_NAMES_SYSPARAM               10
#define TRACE_JUSTIFICATION_NAMES_SYSPARAM       11
#define TRACE_CHUNKS_SYSPARAM                    12
#define TRACE_JUSTIFICATIONS_SYSPARAM            13
#define TRACE_BACKTRACING_SYSPARAM               14
/* ===== watch loading flag =====  KJC 7/96 */
#define TRACE_LOADING_SYSPARAM                   15

/* ====== Max Elaborations === */
#define MAX_ELABORATIONS_SYSPARAM                16

/* ====== Max Chunks === */
#define MAX_CHUNKS_SYSPARAM                      17

#define RESPOND_TO_LOAD_ERRORS_SYSPARAM          18

/* ====== Sysparams for control of learning === */
#define LEARNING_ON_SYSPARAM                     19
#define LEARNING_ONLY_SYSPARAM                   20
#define LEARNING_EXCEPT_SYSPARAM                 21
#define LEARNING_ALL_GOALS_SYSPARAM              22

/* ====== User Select === */
#define USER_SELECT_MODE_SYSPARAM                23

/* ====== Print Warnings === */
#define PRINT_WARNINGS_SYSPARAM                  24

/* AGR 627 begin */
/* ====== Whether to print out aliases as they're defined === */
#define PRINT_ALIAS_SYSPARAM                     25
/* AGR 627 end */

/* ===== explain_flag =====  KJC 7/96 */
#define EXPLAIN_SYSPARAM                         26

/* kjh(B14) */
#define USE_LONG_CHUNK_NAMES                     27

/* REW:  10.22.97 */
#define TRACE_OPERAND2_REMOVALS_SYSPARAM         28

/* RMJ */
#define REAL_TIME_SYSPARAM         		          29

/* RMJ */
#define ATTENTION_LAPSE_ON_SYSPARAM              30

/* KJC 3/01 limit number of cycles in run_til_output */
#define MAX_NIL_OUTPUT_CYCLES_SYSPARAM           31

/* SAN (??) */
//NUMERIC_INDIFFERENCE
#define TRACE_INDIFFERENT_SYSPARAM               32

/* rmarinie 11/04 */
#define TIMERS_ENABLED                           33

/* --- Warning: if you add sysparams, be sure to update the next line! --- */
#define HIGHEST_SYSPARAM_NUMBER                  33

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024   /* AGR 536  - from sys/param.h */
#endif

#define kChunkNamePrefixMaxLength  64  /* kjh (B14) */



//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
////                                                                      ////
////  These defines are a total HACK!!  They are copied from the          ////
////  lexer.h file.                                                       //// 
////                                                                      ////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32x
// Consider just including lexer.h hear (it's just a temporary hack)
enum lexer_token_type {
  EOF_LEXEME,                        /* end-of-file */
  IDENTIFIER_LEXEME,                 /* identifier */
  VARIABLE_LEXEME,                   /* variable */
  SYM_CONSTANT_LEXEME,               /* symbolic constant */
  INT_CONSTANT_LEXEME,               /* integer constant */
  FLOAT_CONSTANT_LEXEME,             /* floating point constant */
  L_PAREN_LEXEME,                    /* "(" */
  R_PAREN_LEXEME,                    /* ")" */
  L_BRACE_LEXEME,                    /* "{" */
  R_BRACE_LEXEME,                    /* "}" */
  PLUS_LEXEME,                       /* "+" */
  MINUS_LEXEME,                      /* "-" */
  RIGHT_ARROW_LEXEME,                /* "-->" */
  GREATER_LEXEME,                    /* ">" */
  LESS_LEXEME,                       /* "<" */
  EQUAL_LEXEME,                      /* "=" */
  LESS_EQUAL_LEXEME,                 /* "<=" */
  GREATER_EQUAL_LEXEME,              /* ">=" */
  NOT_EQUAL_LEXEME,                  /* "<>" */
  LESS_EQUAL_GREATER_LEXEME,         /* "<=>" */
  LESS_LESS_LEXEME,                  /* "<<" */
  GREATER_GREATER_LEXEME,            /* ">>" */
  AMPERSAND_LEXEME,                  /* "&" */
  AT_LEXEME,                         /* "@" */
  TILDE_LEXEME,                      /* "~" */
  UP_ARROW_LEXEME,                   /* "^" */
  EXCLAMATION_POINT_LEXEME,          /* "!" */
  COMMA_LEXEME,                      /* "," */
  PERIOD_LEXEME,                     /* "." */
  QUOTED_STRING_LEXEME,              /* string in double quotes */
  DOLLAR_STRING_LEXEME };            /* string for shell escape */
#endif

#endif

