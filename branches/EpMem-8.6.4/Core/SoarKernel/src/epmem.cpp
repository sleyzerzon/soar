/*************************************************************************
 *
 * file:  epmem.cpp
 *
 * Routines for Soar's episodic memory module (added in 2002 by :AMN:)
 *
 *
 * Copyright (c) 1995-2007 Carnegie Mellon University,
 *                         The Regents of the University of Michigan,
 *                         University of Southern California/Information
 *                         Sciences Institute.  All rights reserved.
 *
 * The Soar consortium proclaims this software is in the public domain, and
 * is made available AS IS.  Carnegie Mellon University, The University of 
 * Michigan, and The University of Southern California/Information Sciences 
 * Institute make no warranties about the software or its performance,
 * implied or otherwise.
 * =======================================================================
 */

/*
 *  FUTURE WORK:
 *  1.  I'm currently allocating all my memory with the MISCELLANEOUS_MEM_USAGE
 *      constant.  Should I add a separate constant specifically for
 *      episodic memory allocation (which can grow quite large)?
 *  2.  The "epmem" command (see cli_epmem.cpp) could be modified to
 *      allow the user to configure various settings at runtime.
 *      Particularly, the epmem constants (below).
 *  3.  Interval-based matching (once implemented in an older version)
 *      could be reimplemented here and in such a way that the user
 *      could select between them at compile time.
 *  4.  Multi-valued attributes are currently handled poorly,
 *      especially during cleanup (where they cause a memory leak).
 *      The real fix will probbaly require abandoning the shortcut of
 *      treating working memory as a tree rather than a graph.
 *  5.  Currently, as long as a cue exists the epmem system does a new
 *      retrieval every cycle.  Performance could be improved
 *      dramatically if there was some way to detect that the cue is
 *      unchanged since last cycle and do nothing.
 
 *  6.  The current epmem store is saved to a file at regular
 *      intervals.  The user can set the filename and the interval
 *      but it would be nice if there was a more meaningful criterion
 *      for saving.  At the very least, something should fire when the
 *      agent exits to save the current store.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H
#include "portability.h"

#include "kernel.h"

#ifdef EPISODIC_MEMORY

#include "agent.h"
#include "print.h"
#include "prefmem.h"
#include "io.h"

//defined in symtab.cpp but not in symtab.h
extern unsigned long compress(unsigned long h, short num_bits);
extern unsigned long hash_string(const char *s);

//Uncomment this to make epmem a no-op
//#define DISABLE_EPMEM

/*======================================================================

  EpMem Constants

  CAVEAT:  Some of these constants are used by code that is currently
           not active but must be enabled.
   
   num_active_wmes      - epmem sometimes uses the n most active wmes
                          to decide whether to record a new memory.
                          This is n.
   num_wmes_changed     - number of wmes in the current list that must
                          be different from the previous list to
                          trigger a new memory.
   memories_init_size   - starting size for the
                          thisAgent->epmem_memories array
   memory_match_wait    - How long to wait before memories can be recalled.  This
                          value is expressed in the number of newer memories that
                          must exist before this one is retrievable.
   ubiquitous_threshold - When a wmetree node's assoc_memories list has
                          a number of entries that exceeds this fraction
                          of the total number of episodic memories then it
                          has become too ubiquitous and is no longer used
                          in matching.
   ubiquitous_max       - There must be at least this many episodic
                          memories before any node can be considered
                          ubiquitous
   fraction_to_trim     - fraction of a new memory to trim before
                          recording it
   card_act_ratio       - Specifies the ratio of match score weighting
                          between match cardinality and match
                          activation.
   epmem_age_limit -      If a memory has not be created or retrieved after
                          this many other memories have been recorded
                          then pitch it.  Set this to 0 for no limit.
                          2100 = 99%; 600=95%
                          
  ======================================================================
*/

#define num_active_wmes 1
#define num_wmes_changed 1
#define memories_init_size 512
#define memory_match_wait 1     // %%%TODO: no longer needed?
#define ubiquitous_threshold 1.0
#define ubiquitous_max 25
#define fraction_to_trim 0.0
float g_card_act_ratio=3.0;
#define epmem_age_limit 2100

/*======================================================================
 * EpMem Data Structures
 *----------------------------------------------------------------------
 */

/*
 * arraylist
 *
 * A growable array.  This was written before the Soar kernel
 * supported C++ compiles (and thus we could have used a Vector
 * instead).  I'm not hurry to switch to Vectors since I hate the
 * STL.
 *
 * I use an arraylist to store each episodic memory (as a list of
 * pointers into thisAgent->epmem_wmetree_size).  I also use it in the
 * wmetree stuct to store a list of pointers to memories that use that
 * node.
 *
 * Members:
 * next     - used to create a linked list of these (also allows hashing)
 * array    - an array of void pointers
 * capactiy - the current size of the array
 * size     - the number of cells in the array currently being used
 *
 */
typedef struct arraylist_struct
{
    struct arraylist_struct *next;
    void **array;
    long capacity;
    long size;
} arraylist;

/*
 * 
 *   wmetree
 *
 * Used to build a tree representation of the structure and content of
 * all WMEs that the agent has encountered.  This in turn is used to
 * store and construct episodic memories.  This structure represents
 * one node in the tree.
 *
 *  Members:
 *   next           - dll of all nodes in the tree to allow hashing as
 *                    well as iterative traversal
 *   id             - every WME in the tree has a unique id.  I can't
 *                    use the timetag because multiple WMEs may be
 *                    created over time that have the same attr/value.
 *   attr           - the string representing the WME attribute associated
 *                    with this tree node
 *   val            - the string representing the WME value associated
 *                    with this tree node (*only* if this is a leaf node)
 *   val_type       - whether this WME's value is a string/int/float/other.
 *                    Uses the constants from symtab.h
 *   relation       - used in cues.  This specifies the relation that
 *                    the matched epmem's value for this WME must have
 *                    with the value in the cue (e.g., "greater than")
 *   children       - children of the current node (hash table)
 *   parent         - parent of the current node.  Currently only a
 *                    single parent is supported.  Thus episodic
 *                    memories are always a tree.
 *   depth          - depth in the tree (root node is depth=0)
 *   assoc_wmes     - a list of wmes that are currently in WM that
 *                    were created using this node as their template
 *                    (i.e., as part of a retrieval).  The index of
 *                    a particular WME in the list always equals
 *                    the index of the epmem_header for the memory.
 *   assoc_memories - this is a list of pointers to all the episodic
 *                    memories that use this WME.  As such it is
 *                    effectively an arraylist of arraylists.
 *   query_count    - How many times this WME has been in a cue and
 *                    therfore triggered its associated memories to be
 *                    examined for a match. (performance diagnostic use
 *                    only)
 *   ubiquitous     - this is a boolean flag that indicates whether
 *                    or not this node appears in "most" episodes.
 *                    "Most" is defined using ubiquitous_threshold and
 *                    ubiquitous_max
 *
 */

enum wmetree_relation { NONE, GREATER_THAN, LESS_THAN };

typedef struct wmetree_struct
{
    struct wmetree_struct *next; // used by the hash table
    int id;
    char *attr;
    union
    {
        char *strval;
        long intval;
        float floatval;
    } val;
    int val_type;
    enum wmetree_relation relation;
    
    hash_table *children;
    struct wmetree_struct *parent; // %%%TODO: make this an array later?
    int depth;
    arraylist *assoc_wmes;
    arraylist *assoc_memories;
    int query_count;
    int ubiquitous;
} wmetree;


/*
 * actwme - a wme paired with an activation value.  Activation values
 *          are stored when a memory is recorded.  They need to share
 *          a data structure together so an array of them can be sorted.
 *
 */
typedef struct actwme_struct
{
    wmetree *node;
    int activation;
} actwme;


/*
 * episodic_memory - This data structure contains a single episodic memory.
 *
 *   content     - an arraylist of actwme structs.
 *   index       - this memory's index in the
 *                 thisAgent->epmem_memories arraylist
 *   last_usage  - the number of the last retrieval that partially
 *                 matched this memory
 *   match_score - the total match score from the last partial match
 *   act_total   - the sum of the activation levels in the last match
 *   num_matches - the number of cue entries that matched in the last match
 *   last_ret    - the cycle when this memory was last created or retrieved
 *                 (a value of -1 indicates the memory has been forgotten)
 *
 */
typedef struct episodic_memory_struct
{
    arraylist *content;
    int index;
    int last_usage;
    float match_score;
    float act_total;
    int num_matches;
    int last_ret;
} episodic_memory;


/*
 * epmem_command
 *
 * This structure holds a query command given by the agent.  Some
 * commands can be handled directly but others require this structure.
 *
 *   cmd_wme  - the ID of the command
 *   name     - the name of the command
 *   arg      - the command argument*
 *   arg_type - the argument's type
 *
 * Commands that currently use this structure:  ^before, ^after
 *
 */
typedef struct epmem_command_struct
{
    wme *cmd_wme;
    char *name;
    union
    {
        char *strval;
        long intval;
        float floatval;
    } arg;
    int arg_type;
} epmem_command;

/*
 * epmem_header
 *
 * An arraylist of these is used to keep track of the ^epmem link
 * attached to each state in working memory
 *
 *  index           - the index of this header in thisAgent->epmem_header_stack.  This
 *                    index is used to reference the assoc_wmes arraylist
 *                    in a wmetree node
 *  state           - the state that ^epmem is attached to
 *  ss_wme          - a pointer to the state's ^superstate WME (used for
 *                    creating fake prefs)
 *  epmem           - The symbol that ^epmem has as an attribute
 *  command         - The symbol that ^command has as an attribute
 *  result          - The symbol that ^result has as an attribute
 *  query_cmd       - The root of the cue given via a "query" command 
 *  next_cmd        - If the agent has issued the "^command next" command
 *                    this flag is set to TRUE
 *  temporal_cmd    - This structure holds details of a "before" or "after"
 *                    command issued by the agent
 *  prohibit_list   - This arraylist stores ids the memories ^prohibit-ed
 *                    by the agent
 *  curr_memory     - Pointer to the memory currently in the ^retrieved link
 *  result_wmes     - a list of wme* that have been placed in WM as the result
 *                    of a retrieval
 *  retrieval_count - how many sequential memories have been retrieved via the
 *                    next command.  Initial value=1 for the first retrieval.
 *  last_cue_size   - size of the last cue given to this header
 *  last_match_size - number of items that matched the cue in the last match
 *  last_match_score- the raw match score of the last match
 *
 */
typedef struct epmem_header_struct
{
    int index;
    Symbol *state;
    wme *ss_wme;
    Symbol *epmem;
    wme *epmem_wme;
    Symbol *command;
    wme *command_wme;
    Symbol *result;
    wme *result_wme;
    Symbol *query_cmd;
    Symbol *negquery_cmd;
    int next_cmd;
    epmem_command *temporal_cmd;
    arraylist *prohibit_list;
    episodic_memory *curr_memory;
    arraylist *result_wmes;
    int retrieval_count;
    int last_cue_size;
    int last_match_size;
    float last_match_score;
} epmem_header;



/*======================================================================
  EpMem macros

   IS_LEAF_WME(w) - is w a leaf WME? (i.e., value is not an identifier)
   
 *----------------------------------------------------------------------
*/
#define IS_LEAF_WME(w) ((w)->value->common.symbol_type != IDENTIFIER_SYMBOL_TYPE)



#ifdef __cplusplus
extern "C"
{
#endif

/*======================================================================
  EpMem Forward Declarations

  These non-public functions are used before they are defined.
   
 *----------------------------------------------------------------------
*/
Symbol *update_wmetree(agent *thisAgent,
                       wmetree *node,
                       Symbol *sym,
                       arraylist *epmem,
                       tc_number tc);

episodic_memory *find_best_match(agent *thisAgent,
                                 epmem_header *h,
                                 arraylist *cue,
                                 arraylist *negcue);

void epmem_clear_curr_result(agent *thisAgent,
                             epmem_header *h);

wme *get_aug_of_id(agent *thisAgent,
                   Symbol *sym,
                   char *attr_name,
                   char *value_name);

void retrieve_all_child_wmes(agent *thisAgent,
                             Symbol *sym, 
                             arraylist *syms,
                             tc_number tc);
    
/* ===================================================================
   compare_ptr

   Compares two void * pointers.  (Used for qsort() calls)
   
   Created: 06 Nov 2002
   =================================================================== */
int compare_ptr( const void *arg1, const void *arg2 )
{
    return *((long *)arg1) - *((long *)arg2);
}//compare_ptr

/* ===================================================================
   compare_actwme

   Compares two actwmes.   (Used for qsort() calls)
   
   Created:  22 April 2004
   Modified: 26 Aug 2004 to use wmetree depth  (Happy Birthday to me!)
   =================================================================== */
int compare_actwme( const void *arg1, const void *arg2 )
{
    wmetree *w1 = (*((actwme **)arg1))->node;
    wmetree *w2 = (*((actwme **)arg2))->node;

    if (w1->depth != w2->depth)
    {
        return w1->depth - w2->depth;
    }

    return ((int)w1) - ((int)w2);
}//compare_actwme

/* ===================================================================
   compare_actwme_by_act

   Compares two actwmes based on activation.   (Used for qsort() calls)
   This comparison results in a descending sort:  higher activation
   is "smaller".
   
   Created:  13 Dec 2004
   =================================================================== */
int compare_actwme_by_act( const void *arg1, const void *arg2 )
{
    actwme *aw1 = (*(actwme **)arg1);
    actwme *aw2 = (*(actwme **)arg2);

    return aw2->activation - aw1->activation;
}//compare_actwme_by_act


/* ===================================================================
   hash_wmetree

   Creates a hash value for a wmetree node

   Created: 22 Apr 2004
   =================================================================== */
unsigned long hash_wmetree(void *item, short num_bits)
{
    wmetree *node = (wmetree *)item;
    unsigned long hash_value;

    //Generate a hash value for the node's attr and value
    hash_value = hash_string(node->attr);
    switch(node->val_type)
    {
        case SYM_CONSTANT_SYMBOL_TYPE:
            hash_value += hash_string(node->val.strval);
            break;
            
        case INT_CONSTANT_SYMBOL_TYPE:
            hash_value += node->val.intval;
            break;

        case FLOAT_CONSTANT_SYMBOL_TYPE:
            hash_value += (unsigned long)node->val.floatval;
            break;
    }//switch

    return compress(hash_value, num_bits);
}//hash_wmetree

/* ===================================================================
   hash_wme

   Creates a hash value for a WME.  This is used to find the
   corresponding wmetree node in a hash table.

   Created: 22 Apr 2004
   =================================================================== */
unsigned long hash_wme(wme *w, short num_bits)
{
    unsigned long hash_value;

    //Generate a hash value for the WME's attr and value
    hash_value = hash_string(w->attr->sc.name);
    switch(w->value->common.symbol_type)
    {
        case SYM_CONSTANT_SYMBOL_TYPE:
            hash_value += hash_string(w->value->sc.name);
            break;
            
        case INT_CONSTANT_SYMBOL_TYPE:
            hash_value += w->value->ic.value;
            break;

        case FLOAT_CONSTANT_SYMBOL_TYPE:
            hash_value += (unsigned long)w->value->fc.value;
            break;
    }//switch

    return compress(hash_value, num_bits);
    
}//hash_wme

/* ===================================================================
   make_arraylist

   Allocates and initializes a new, empty arraylist and returns a pointer
   to the caller.

   init_cap = initial capacity
   
   Created: 13 Jan 2004
   =================================================================== */
arraylist *make_arraylist(agent *thisAgent, int init_cap)
{
    arraylist *al;
    int i;

    if (init_cap <= 0) init_cap = 32; // default value
    
    al = (arraylist *)allocate_memory(thisAgent, sizeof(arraylist),
                                      MISCELLANEOUS_MEM_USAGE);
    al->array = (void **)allocate_memory(thisAgent, sizeof(void*) * init_cap,
                                         MISCELLANEOUS_MEM_USAGE);
    al->capacity = init_cap;
    al->size = 0;
    al->next = NULL;

    for(i = 0; i < init_cap; i++)
    {
        al->array[i] = NULL;
    }

    return al;
}//make_arraylist

/* ===================================================================
   copy_arraylist

   Allocates and initializes a new arraylist that is a copy of a given
   original arraylist.

   CAVEAT:  This is not a deep copy!
   CAVEAT:  User is responsible for deallocation
   
   Created: 06 Dec 2006
   =================================================================== */
arraylist *copy_arraylist(agent *thisAgent, arraylist *orig)
{
    arraylist *al;
    int i;

    if (orig == NULL) return NULL;
    if (orig->size == 0) return make_arraylist(thisAgent, orig->capacity);

    al = (arraylist *)allocate_memory(thisAgent,
                                      sizeof(arraylist),
                                      MISCELLANEOUS_MEM_USAGE);
    al->array = (void **)allocate_memory(thisAgent,
                                         sizeof(void*) * orig->capacity,
                                         MISCELLANEOUS_MEM_USAGE);
    al->capacity = orig->capacity;
    al->size = orig->size;
    al->next = NULL;

    for(i = 0; i < orig->capacity; i++)
    {
        al->array[i] = orig->array[i];
    }

    return al;
}//copy_arraylist

/* ===================================================================
   destroy_arraylist

   Deallocates a given arraylist.

   CAVEAT:  The caller is responsible for any memory pointed to by
            the entries in the arraylist.
   
   Created: 19 Jan 2004
   =================================================================== */
void destroy_arraylist(agent *thisAgent, arraylist *al)
{
    if (al == NULL) return;
    
    if ( (al->capacity > 0) && (al->array != NULL) )
    {
        free_memory(thisAgent, al->array, MISCELLANEOUS_MEM_USAGE);
    }
    free_memory(thisAgent, al, MISCELLANEOUS_MEM_USAGE);
}//destroy_arraylist

/* ===================================================================
   grow_arraylist

   This function increases the length of an arraylist to a minimum
   of a given capacity.
   
   Created: 06 Oct 2004
   =================================================================== */
void grow_arraylist(agent *thisAgent, arraylist *al, int desired_capacity)
{
    int i;
    void **new_array;
    int new_capacity;

    //Check to see if capacity is already correct
    if (desired_capacity <= al->capacity) return;

    //Determine the new capacity
    new_capacity = al->capacity;
    if (new_capacity == 0) new_capacity = 32;
    while (new_capacity < desired_capacity)
    {
        new_capacity *= 2;
    }
    
    //Grow the array (can't use realloc b/c of Soar's memory routines)
    new_array = (void **)allocate_memory_and_zerofill(thisAgent,
                                                      new_capacity * sizeof(void*),
                                                      MISCELLANEOUS_MEM_USAGE);
    for(i = 0; i < al->size; i++)
    {
        new_array[i] = al->array[i];
    }

    if (al->array != NULL)
    {
        free_memory(thisAgent, al->array, MISCELLANEOUS_MEM_USAGE);
    }

    al->array = new_array;
    al->capacity = new_capacity;

}//grow_arraylist


/* ===================================================================
   append_entry_to_arraylist

   This function adds a new entry to the end of an arraylist
   
   Created: 22 Apr 2004
   =================================================================== */
void append_entry_to_arraylist(agent *thisAgent, arraylist *al, void *new_entry)
{
    if (al->size == al->capacity)
    {
        grow_arraylist(thisAgent, al, al->size+1);
    }//if


    //Add the node to the array
    al->array[al->size] = new_entry;
    al->size++;
    
}//append_entry_to_arraylist

/* ===================================================================
   get_arraylist_entry

   Given an index, this function returns the entry in the arraylist
   at that point.
   
   Created: 06 Oct 2004
   =================================================================== */
void *get_arraylist_entry(agent *thisAgent, arraylist *al, int index)
{
    //Catch erroneous input values
    if (al == NULL) return NULL;
    if (index < 0) return NULL;
    if (index >= al->size)
    {
        grow_arraylist(thisAgent, al, index + 1);
    }

    return al->array[index];
}//get_arraylist_entry

/* ===================================================================
   set_arraylist_entry

   Given an index, this function sets the entry in the arraylist
   at that index to a given value.
   
   Created: 06 Oct 2004
   =================================================================== */
void set_arraylist_entry(agent *thisAgent, arraylist *al, int index, void *newval)
{
    //Catch erroneous input values
    if (al == NULL) return;
    if (index < 0) return;
    if (index >= al->size)
    {
        grow_arraylist(thisAgent, al, index + 1);
        al->size = index+1;
    }

    al->array[index] = newval;
}//set_arraylist_entry

/* ===================================================================
   remove_entry_from_arraylist

   Given an index, this function removes the entry at that index from
   an arraylist and moves down subsequent entries to fill in the gap.
   The caller is responsible for cleaning up the entry itself.
   
   CAVEAT: The caller is responsible for cleaning up the content
           of the entry itself.
   
   Created: 04 Oct 2004
   =================================================================== */
void *remove_entry_from_arraylist(arraylist *al, int index)
{
    int i;
    void *retval = NULL;
    
    //Catch erroneous input values
    if (al == NULL) return NULL;
    if (index >= al->size) return NULL;
    if (index < 0) return NULL;

    retval = al->array[index];
    al->array[index] = NULL;
    for(i = index + 1; i < al->size; i++)
    {
        al->array[i - 1] = al->array[i];
    }
    
    (al->size)--;

    return retval;
    
}//remove_entry_from_arraylist

/* ===================================================================
   remove_entry_from_arraylist_by_ptr

   Given a pointer, this function removes the entry from an arraylist
   that matches that pointer.

   CAVEAT: The caller is responsible for cleaning up the content
           of the entry itself.
   
   Created: 04 Apr 2007
   =================================================================== */
void remove_entry_from_arraylist_by_ptr(arraylist *al, void *ptr)
{
    int i;
    void *curr = NULL;
    int bFound = FALSE;
    
    //Loop over the associated epmems list to find this memory
    for(i = 1; i < al->size; i++)
    {
        //get the entry pointer
        curr = al->array[i];

        //If we've found it just shift everything down a notch
        if (bFound)
        {
            al->array[i - 1] = al->array[i];
        }
        //Otherwise see if we've found it
        else if (curr == ptr)
        {
            bFound = TRUE;
        }
    }//for

    if (bFound)
    {
        (al->size)--;
    }
    
}//remove_entry_from_arraylist_by_ptr


/* ===================================================================
   make_wmetree_node

   Creates a new wmetree node based upon a given wme.  If no WME is
   given (w == NULL) then an empty node is allocated.  The caller is
   responsible for setting the parent pointer and the id.
   
   Created: 09 Jan 2004
   =================================================================== */
wmetree *make_wmetree_node(agent *thisAgent, wme *w)
{
    wmetree *node;

    node = (wmetree *)allocate_memory(thisAgent, sizeof(wmetree), MISCELLANEOUS_MEM_USAGE);
    node->next = NULL;
    node->id = -1;
    node->attr = NULL;
    node->val.intval = 0;
    node->val_type = IDENTIFIER_SYMBOL_TYPE;
    node->relation = NONE;
    node->children = make_hash_table(thisAgent, 0, hash_wmetree);
    node->parent = NULL;
    node->depth = -1;
    node->assoc_wmes = make_arraylist(thisAgent, 20);
    node->assoc_memories = make_arraylist(thisAgent, 16);
    node->query_count = 0;
    node->ubiquitous = FALSE;

    if (w == NULL) return node;
    
    node->attr = (char *)allocate_memory(thisAgent, sizeof(char)*strlen(w->attr->sc.name) + 1,
                                MISCELLANEOUS_MEM_USAGE);
    strcpy(node->attr, w->attr->sc.name);
    
    switch(w->value->common.symbol_type)
    {
        case SYM_CONSTANT_SYMBOL_TYPE:
            node->val_type = SYM_CONSTANT_SYMBOL_TYPE;
            node->val.strval =
                (char *)allocate_memory(thisAgent, sizeof(char)*strlen(w->value->sc.name) + 1,
                                MISCELLANEOUS_MEM_USAGE);
            strcpy(node->val.strval, w->value->sc.name);
            break;

        case INT_CONSTANT_SYMBOL_TYPE:
            node->val_type = INT_CONSTANT_SYMBOL_TYPE;
            node->val.intval = w->value->ic.value;
            break;

        case FLOAT_CONSTANT_SYMBOL_TYPE:
            node->val_type = FLOAT_CONSTANT_SYMBOL_TYPE;
            node->val.floatval = w->value->fc.value;
            break;

        default:
            node->val_type = IDENTIFIER_SYMBOL_TYPE;
            node->val.intval = 0;
            break;
    }

    return node;
}//make_wmetree_node

/* ===================================================================
   destroy_wmetree (+ dw_helper)       *RECURSIVE*

   Deallocate an entire wmetree.

   CAVEAT:  All children of the given node are also deleted!
   CAVEAT:  Caller should make sure that no retreived epmems are in WM which
            are associated with this node or any of its children.
   CAVEAT:  asssoc_memories is dealloated but the epmems it points
            to are not.  The other arraylists are cleaned up.

   Created: 10 Nov 2002
   Updated: 31 Mar 2006 (cleanup for associated arraylists)
   =================================================================== */
void dw_helper(agent *thisAgent, wmetree *tree)
{
    unsigned long hash_value;
    wmetree *child, *next_child;
    int i;
    wme *w;

    if (tree->attr != NULL)
    {
        free_memory(thisAgent, tree->attr, MISCELLANEOUS_MEM_USAGE);
    }
    
    if (tree->val_type == SYM_CONSTANT_SYMBOL_TYPE)
    {
        free_memory(thisAgent, tree->val.strval, MISCELLANEOUS_MEM_USAGE);
    }
    
    if (tree->children->count == 0)
    {
        return;
    }


    //Recursively destroy all the children before the parent
    for (hash_value = 0; hash_value < tree->children->size; hash_value++)
    {
        child = (wmetree *) (*(tree->children->buckets + hash_value));
        while(child != NULL)
        {
            next_child = child->next;
            remove_from_hash_table(thisAgent, tree->children, child);
            dw_helper(thisAgent, child);
            free_memory(thisAgent, child, MISCELLANEOUS_MEM_USAGE);
            child = next_child;
        }//while
    }//for

    free_memory(thisAgent, tree->children, HASH_TABLE_MEM_USAGE);

    //Cleanup the arraylists

    //Sanity check
    for(i = 0; i < tree->assoc_wmes->size; i++)
    {
        w = (wme *)get_arraylist_entry(thisAgent, tree->assoc_wmes, i);
        if (w != NULL)
        {
            print(thisAgent, "ERROR!:  Removing wmetree node that still has associated WMEs.");
        }
    }
    destroy_arraylist(thisAgent, tree->assoc_wmes);
    destroy_arraylist(thisAgent, tree->assoc_memories);
    
}//dw_helper

void destroy_wmetree(agent *thisAgent, wmetree *tree)
{
    dw_helper(thisAgent, tree);

    free_memory(thisAgent, tree, MISCELLANEOUS_MEM_USAGE);
}//destroy_wmetree


/* ===================================================================
   make_epmem_command

   This routine creates an empty epmem_command.

   Created: 13 July 06
   =================================================================== */
epmem_command *make_epmem_command(agent *thisAgent)
{
    epmem_command *cmd =
        (epmem_command *)allocate_memory(thisAgent,
                                         sizeof(epmem_command),
                                         MISCELLANEOUS_MEM_USAGE);

    cmd->cmd_wme = NULL;
    cmd->name = NULL;
    cmd->arg.strval = NULL;
    cmd->arg_type = -1;

    return cmd;
}
/* ===================================================================
   cleanup_epmem_command

   This routine cleans out old data in an epmem command but does
   not deallocate it.

   Created: 13 July 06
   =================================================================== */
void cleanup_epmem_command(agent *thisAgent, epmem_command *cmd)
{
    if (cmd == NULL) return;

    if (cmd->name != NULL)
    {
        free_memory(thisAgent, cmd->name, MISCELLANEOUS_MEM_USAGE);
    }

    if ( (cmd->arg_type == SYM_CONSTANT_SYMBOL_TYPE)
         && (cmd->arg.strval != NULL) )
    {
        free_memory(thisAgent, cmd->arg.strval, MISCELLANEOUS_MEM_USAGE);
    }

    cmd->cmd_wme = NULL;
    cmd->name = NULL;
    cmd->arg.strval = NULL;
    cmd->arg_type = -1;

}//cleanup_epmem_command


/* ===================================================================
   destroy_epmem_command

   This routine cleans up and deallocates an epmem_command

   Created: 13 July 06
   =================================================================== */
void destroy_epmem_command(agent *thisAgent, epmem_command *cmd)
{
    if (cmd == NULL) return;

    cleanup_epmem_command(thisAgent, cmd);
    free_memory(thisAgent, cmd, MISCELLANEOUS_MEM_USAGE);

}//destroy_epmem_command

/* ===================================================================
   make_fake_preference_for_epmem_wme

   This function adds a fake preference to a WME so that it will not
   be added to the goal dependency set of the state it is attached
   to.  This is used to prevents the GDS from removing a state
   whenever a epmem is retrieved that is attached to it.  

   (The bulk of the content of this function is taken from
    make_fake_preference_for_goal_item() in decide.c)
   
   
   Created: 18 Oct 2004
   Changed  24 Jan 2005: to improve performance the ^superstate wme is now
                         cached in the epmem_header)
   Changed  12 Jan 2006: added support for chunking by using the cue WMEs
                         as the conditions of the instantiation.  In
                         a sense, this "fake preference" isn't really fake
                         anymore.
   =================================================================== */
preference *make_fake_preference_for_epmem_wme(agent *thisAgent, epmem_header *h, Symbol *goal, wme *w)
{
    instantiation *inst;
    preference *pref;
    condition *cond;
    condition *prev_cond = NULL;
    int i;

    /*
     * make the fake preference
     */
    pref = make_preference(thisAgent, ACCEPTABLE_PREFERENCE_TYPE, w->id, w->attr, w->value, NIL);
    pref->o_supported = TRUE;
    symbol_add_ref(pref->id);
    symbol_add_ref(pref->attr);
    symbol_add_ref(pref->value);

    //This may not be necessary??
    insert_at_head_of_dll(goal->id.preferences_from_goal, pref, all_of_goal_next, all_of_goal_prev);
    pref->on_goal_list = TRUE;

    preference_add_ref(pref);

        
    /*
     * make the fake instantiation
     */
    allocate_with_pool(thisAgent, &(thisAgent->instantiation_pool), &inst);
    pref->inst = inst;
    pref->inst_next = pref->inst_prev = NIL;
    inst->preferences_generated = pref;
    inst->prod = NIL;
    inst->next = inst->prev = NIL;
    inst->rete_token = NIL;
    inst->rete_wme = NIL;
    inst->match_goal = goal;
    inst->match_goal_level = goal->id.level;
    inst->okay_to_variablize = TRUE;
    inst->backtrace_number = 0;
    inst->in_ms = FALSE;

    /*
     * make the fake condition for all cue elements
     *
     * NOTE: This is effectively an instantiation whose condition is
     *       the cue WMEs and whose sole action is the retrieved epmem WME
     */
    //Retrieve all the cue WMEs
    arraylist *query = make_arraylist(thisAgent, 32);
    if (h->query_cmd != NULL)
    {
        retrieve_all_child_wmes(thisAgent, h->query_cmd, query, h->query_cmd->id.tc_num + 1);
    }
    
    //If there is no cue than just use ^superstate instead.  
    //(This should only happen with header WMEs).
    if (query->size ==0)
    {
        append_entry_to_arraylist(thisAgent, query, (void *)h->ss_wme);
    }
    
    //Create a condition for each cue WME
    for(i = 0; i < query->size; i++)
    {
        wme *cue_wme = (wme *)get_arraylist_entry(thisAgent, query, i);

        //Construct the condition
        allocate_with_pool(thisAgent, &(thisAgent->condition_pool), &cond);
        cond->type = POSITIVE_CONDITION;
        cond->prev = prev_cond;
        cond->next = NULL;
        if (prev_cond != NULL)
        {
            prev_cond->next = cond;
        }
        else
        {
            inst->top_of_instantiated_conditions = cond;
            inst->bottom_of_instantiated_conditions = cond;
            inst->nots = NIL;
        }
        cond->data.tests.id_test = make_equality_test(cue_wme->id);
        cond->data.tests.attr_test = make_equality_test(cue_wme->attr);
        cond->data.tests.value_test = make_equality_test(cue_wme->value);
        cond->test_for_acceptable_preference = TRUE;
        cond->bt.wme_= cue_wme;
        wme_add_ref(cue_wme);  //%%%Causes mem leak?
        cond->bt.level = cue_wme->id->id.level;
        cond->bt.trace = NIL;
        cond->bt.prohibits = NIL;

        prev_cond = cond;

    }//for

    destroy_arraylist(thisAgent, query);
    
    return pref;
}//make_fake_preference_for_epmem_wme

/* ===================================================================
   remove_fake_preference_for_epmem_wme

   This function removes a fake preference on a WME created by
   make_fake_preference_for_epmem_wme().  While it's a one-line
   function I thought it was important to create it so it would be
   clear what's going on in this case.
   
   Created: 21 Oct 2004
   =================================================================== */
void remove_fake_preference_for_epmem_wme(agent *thisAgent, wme *w)
{

    preference_remove_ref(thisAgent, w->preference);
    

}//remove_fake_preference_for_epmem_wme

/* ===================================================================
   make_epmem_header

   Allocates a new epmem_header struct for a given state and fills in
   the proper values.  The caller is responsible for setting the
   index field.

   s - the state that the header is to be attached to
   
   Created: 04 Oct 2004
   =================================================================== */
epmem_header *make_epmem_header(agent *thisAgent, Symbol *s)
{
    epmem_header *h;

    h = (epmem_header *)allocate_memory(thisAgent, sizeof(epmem_header), MISCELLANEOUS_MEM_USAGE);
    h->index = -42; //A bogus value to help with debugging
    h->state = s;
    h->query_cmd = NULL;
    h->negquery_cmd = NULL;
    h->next_cmd = 0;
    h->temporal_cmd = make_epmem_command(thisAgent);
    h->prohibit_list = make_arraylist(thisAgent, 5);
    h->curr_memory = NULL;
    h->result_wmes = make_arraylist(thisAgent, 32);
    h->retrieval_count = 0;
    h->last_cue_size = 0;
    h->last_match_size = 0;
    h->last_match_score = 0.0;
    
    //Find the superstate wme
    h->ss_wme = get_aug_of_id(thisAgent, s, "superstate", NULL);
    if (!h->ss_wme)
    {
        //This should never happen.
        print_with_symbols(thisAgent, "\nepmem.c: Internal error: couldn't find ^superstate WME on state %y\n", s);
        return NULL;
    }
    
    //Create the ^epmem header symbols
    h->epmem = make_new_identifier(thisAgent, 'E', s->id.level);
    h->command = make_new_identifier(thisAgent, 'E', s->id.level);
    h->result = make_new_identifier(thisAgent, 'E', s->id.level);

    /*
     * Create the top level WMEs
     */
    //<s> ^epmem <e>
    h->epmem_wme = add_input_wme(thisAgent, s, make_sym_constant(thisAgent, "epmem"), h->epmem);
    h->epmem_wme->preference =
        make_fake_preference_for_epmem_wme(thisAgent, h, s, h->epmem_wme);
    wme_add_ref(h->epmem_wme);

    //<e> ^command <c>
    h->command_wme = add_input_wme(thisAgent, h->epmem, make_sym_constant(thisAgent, "command"), h->command);
    h->command_wme->preference =
        make_fake_preference_for_epmem_wme(thisAgent, h, s, h->command_wme);
    wme_add_ref(h->command_wme);

    //<e> ^result <r>
    h->result_wme = add_input_wme(thisAgent, h->epmem, make_sym_constant(thisAgent, "result"), h->result);
    h->result_wme->preference =
        make_fake_preference_for_epmem_wme(thisAgent, h, s, h->result_wme);
    wme_add_ref(h->result_wme);

    return h;
}//make_epmem_header

/* ===================================================================
   copy_epmem_header

   Allocates and an initializes a SHALLOW copy of a given epmem
   header.

   CAVEAT:  Pointers are just copied.  Changes made to substructures
            will effect the original!
   CAVEAT:  Caller is responsible for deallocation.  DO NOT use
            destroy_epmem_header() because it will clean up copied
            structures.  Just use deallocate_memory() instead.
   
   Created: 06 Dec 2006
   =================================================================== */
epmem_header *copy_epmem_header(agent *thisAgent, epmem_header *orig)
{
    epmem_header *h;

    h = (epmem_header *)allocate_memory(thisAgent, sizeof(epmem_header), MISCELLANEOUS_MEM_USAGE);
    h->index = orig->index;
    h->state = orig->state;
    h->ss_wme = orig->ss_wme;
    h->epmem = orig->epmem;
    h->epmem_wme = orig->epmem_wme;
    h->command = orig->command;
    h->command_wme = orig->command_wme;
    h->result = orig->result;
    h->result_wme = orig->result_wme;
    h->query_cmd = orig->query_cmd;
    h->negquery_cmd = orig->negquery_cmd;
    h->next_cmd = orig->next_cmd;
    h->temporal_cmd = orig->temporal_cmd;
    h->prohibit_list = orig->prohibit_list;
    h->curr_memory = orig->curr_memory;
    h->result_wmes = orig->result_wmes;
    h->retrieval_count = orig->retrieval_count;
    h->last_cue_size = orig->last_cue_size;
    h->last_match_size = orig->last_match_size;
    h->last_match_score = orig->last_match_score;

    return h;
}//copy_epmem_header

/* ===================================================================
   destroy_epmem_header

   Frees the resources used by an epmem_header struct.

   Created: 04 Oct 2004
   =================================================================== */
void destroy_epmem_header(agent *thisAgent, epmem_header *h)
{
    //Remove any active memory (or a "no-retrieval" WME) from WM
    epmem_clear_curr_result(thisAgent, h);

    //Cleanup attached data structures
    destroy_epmem_command(thisAgent, h->temporal_cmd);
    destroy_arraylist(thisAgent, h->prohibit_list);
    destroy_arraylist(thisAgent, h->result_wmes);
    
    //Remove the ^epmem header WMEs
    remove_input_wme(thisAgent, h->epmem_wme);
    remove_fake_preference_for_epmem_wme(thisAgent, h->epmem_wme);
    wme_remove_ref(thisAgent, h->epmem_wme);

    remove_input_wme(thisAgent, h->command_wme);
    remove_fake_preference_for_epmem_wme(thisAgent, h->command_wme);
    wme_remove_ref(thisAgent, h->command_wme);

    remove_input_wme(thisAgent, h->result_wme);
    remove_fake_preference_for_epmem_wme(thisAgent, h->result_wme);
    wme_remove_ref(thisAgent, h->result_wme);
    

    //Dereference the ^epmem header symbols
    symbol_remove_ref(thisAgent, h->epmem);
    symbol_remove_ref(thisAgent, h->command);
    symbol_remove_ref(thisAgent, h->result);

    //Free the struct
    free_memory(thisAgent, h, MISCELLANEOUS_MEM_USAGE);
    
}//destroy_epmem_header

/* ===================================================================
   wme_has_value

   This routine returns TRUE if the given WMEs attribute and value are
   both symbols and have the names given.  If either of the given
   names are NULL then they are assumed to be a match (i.e., a
   wildcard).  Thus passing NULL for both attr_name and value_name
   will always yield a TRUE result.

   Created: 14 Dec 2002
   =================================================================== */
int wme_has_value(wme *w, char *attr_name, char *value_name)
{
    if (w == NULL) return FALSE;
    
    if (attr_name != NULL)
    {
        if (w->attr->common.symbol_type != SYM_CONSTANT_SYMBOL_TYPE)
        {
            return FALSE;
        }

        if (strcmp(w->attr->sc.name, attr_name) != 0)
        {
            return FALSE;
        }
    }

    if (value_name != NULL)
    {
        if (w->value->common.symbol_type != SYM_CONSTANT_SYMBOL_TYPE)
        {
            return FALSE;
        }

        if (strcmp(w->value->sc.name, value_name) != 0)
        {
            return FALSE;
        }
    }

    return TRUE;

}//wme_has_value

/* ===================================================================
   epmem_get_augs_of_id()

   This routine works just like the one defined in utilities.h.
   Except this one does not use C++ templates because I have an
   irrational dislike for them borne from the years when the STL
   highly un-portable.  I'm told this is no longer true but I'm still
   bitter. 
   
   Created (sort of): 25 Jan 2006
   =================================================================== */
wme **epmem_get_augs_of_id(agent* thisAgent, Symbol * id, tc_number tc, int *num_attr)
{
   slot *s;
   wme *w;


   wme **list;                 /* array of WME pointers, AGR 652 */
   int attr;                   /* attribute index, AGR 652 */
   int n;


/* AGR 652  The plan is to go through the list of WMEs and find out how
  many there are.  Then we malloc an array of that many pointers.
  Then we go through the list again and copy all the pointers to that array.
  Then we qsort the array and print it out.  94.12.13 */


   if (id->common.symbol_type != IDENTIFIER_SYMBOL_TYPE)
       return NULL;
   if (id->id.tc_num == tc)
       return NULL;
   id->id.tc_num = tc;


   /* --- first, count all direct augmentations of this id --- */
   n = 0;
   for (w = id->id.impasse_wmes; w != NIL; w = w->next)
       n++;
   for (w = id->id.input_wmes; w != NIL; w = w->next)
       n++;
   for (s = id->id.slots; s != NIL; s = s->next) {
       for (w = s->wmes; w != NIL; w = w->next)
           n++;
       for (w = s->acceptable_preference_wmes; w != NIL; w = w->next)
           n++;
   }


   /* --- next, construct the array of wme pointers and sort them --- */
   list = static_cast<wme**>(allocate_memory(thisAgent, n * sizeof(wme *), MISCELLANEOUS_MEM_USAGE));
   attr = 0;
   for (w = id->id.impasse_wmes; w != NIL; w = w->next)
       list[attr++] = w;
   for (w = id->id.input_wmes; w != NIL; w = w->next)
       list[attr++] = w;
   for (s = id->id.slots; s != NIL; s = s->next) {
       for (w = s->wmes; w != NIL; w = w->next)
           list[attr++] = w;
       for (w = s->acceptable_preference_wmes; w != NIL; w = w->next)
           list[attr++] = w;
   }


   *num_attr = n;
   return list;
}

/* ===================================================================
   get_aug_of_id()

   This routine examines a symbol for an augmentation that as the
   given attribute and value and returns it.  See wme_has_value()
   for info on how the correct wme is matched to the given strings.
   
   Created: 19 Oct 2004
   =================================================================== */
wme *get_aug_of_id(agent *thisAgent, Symbol *sym, char *attr_name, char *value_name)
{
    wme **wmes;
    int len = 0;
    int i;
    tc_number tc;
    wme *ret_wme = NULL;
    
    tc = sym->id.tc_num + 1;
    wmes = epmem_get_augs_of_id(thisAgent, sym, tc, &len);
    sym->id.tc_num = tc - 1;
    if (wmes == NULL) return NULL;

    for(i = 0; i < len; i++)
    {
        if (wme_has_value(wmes[i], attr_name, value_name))
        {
            ret_wme = wmes[i];
            break;
        }
    }

    free_memory(thisAgent, wmes, MISCELLANEOUS_MEM_USAGE);
    return ret_wme;
}//get_aug_of_id

/* ===================================================================
   retrieve_all_child_wmes

   Generates an arraylist of wme* that points to all the WMEs in
   working memory that descend from a given symbol.  In other words,
   it returns a list of all the WMEs that you can reach from that
   symbol.  Thus, for example, passing S1 would get you all the WMEs
   on the top-state.

   sym   - the parent symbol
   syms  - an arraylist to fill with wme* pointers that descend from the
           parent.
   tc    - a transitive closure number for avoiding loops in working mem

   CAVEAT:  This function is computationally expensive and should only
            be used by output functions.
   
   Created: 06 Dec 2005
   =================================================================== */
void retrieve_all_child_wmes(agent *thisAgent,
                             Symbol *sym, 
                             arraylist *syms,
                             tc_number tc)
{
    wme **wmes = NULL;
    int len = 0;
    int i;
    Symbol *ss = NULL;
    int pos = -1;
    wme *w;

    /*
     * General Appraoch: Fill the syms list with all the WMEs we've
     * encountered so far.  Maintain a pos pointer to the next symbol
     * that hasn't been examined yet.  Add children of the current
     * WME to the list before advancing the pos pointer.  Thus, when
     * you get to the end of the list you've found all the children.
     */
    do
    {
        pos++;
        
        start_timer(thisAgent, &(thisAgent->epmem_getaugs_start_time));
        wmes = epmem_get_augs_of_id(thisAgent,  sym, tc, &len );
        stop_timer(thisAgent, &(thisAgent->epmem_getaugs_start_time), &(thisAgent->epmem_getaugs_total_time));

        if (wmes != NULL)
        {
            for(i = 0; i < len; i++)
            {
                //insert childnode into the arraylist
                append_entry_to_arraylist(thisAgent, syms, (void *)wmes[i]);
            }//for
            
            free_memory(thisAgent, wmes, MISCELLANEOUS_MEM_USAGE);
        }//if

        //Find the next WME in the list that has an identifier as
        //a value (and thus may have child WMEs)
        while (pos < syms->size)
        {
            w = (wme *)get_arraylist_entry(thisAgent, syms, pos);
            sym = w->value;
            if (sym->common.symbol_type == IDENTIFIER_SYMBOL_TYPE)
            {
                break;
            }
            
            pos++;
        }//while
          
    }while(pos < syms->size);

    
}//retrieve_all_child_wmes

/* ===================================================================
   wme_equals_node

   This routine returns TRUE is the given WME's attribute and value are
   both symbols and match the values in the given wmetree node.  If
   the value in the wmetree node is of type identifier/variable
   then it always matches.

   Created: 12 Jan 2004
   =================================================================== */
int wme_equals_node(wme *w, wmetree *node)
{
    if (w == NULL) return FALSE;

    if (w->attr->common.symbol_type != SYM_CONSTANT_SYMBOL_TYPE)
    {
        return FALSE;
    }

    //Compare attribute
    if (strcmp(w->attr->sc.name, node->attr) != 0)
    {
        return FALSE;
    }

    //Ensure values are of the same type
    if (w->value->common.symbol_type != node->val_type)
    {
        return FALSE;
    }

    //Compare value
    switch(node->val_type)
    {
        case SYM_CONSTANT_SYMBOL_TYPE:
            return strcmp(w->value->sc.name, node->val.strval) == 0;

        case INT_CONSTANT_SYMBOL_TYPE:
            return w->value->ic.value == node->val.intval;

        case FLOAT_CONSTANT_SYMBOL_TYPE:
            return w->value->fc.value == node->val.floatval;
            break;

        default:
            return TRUE;
    }

}//wme_equals_node

/* ===================================================================
   epmem_print_status

   This function will print the current status and configuration
   info for the episodic memory subsystem of this agent.
   
   Created: 15 Nov 2006
   =================================================================== */
void epmem_print_status(agent *thisAgent)
{
    int i,j;
    epmem_header *h;
    
#ifdef DISABLE_EPMEM
    return;
#endif

    print(thisAgent, "\nEPISODIC MEMORY INFO");
    print(thisAgent, "\n--------------------");
    if ((thisAgent->sysparams)[EPMEM_SYSPARAM])
    {
        print(thisAgent, "\n               Episodic Memory is currently ACTIVE.");
        if ((thisAgent->sysparams)[EPMEM_SUSPENDED_SYSPARAM])
        {
            print(thisAgent, "\n               Episodic Memory is also currently SUSPENDED.");
        }
    }
    else
    {
        print(thisAgent, "\n               Episodic Memory is currently DISABLED.");
        return;
    }
    print(thisAgent, "\n               Number of stored memories: %d",
          thisAgent->epmem_memories->size);
    print(thisAgent, "\n          Number of retrievals performed: %ld",
          thisAgent->epmem_num_queries);
    if ( (strlen(thisAgent->epmem_autosave_filename) > 0)
         && (thisAgent->epmem_save_freq > 0) )
    {
        print(thisAgent, "\n         Episodes are being autosaved to: %s every %d cycles",
              thisAgent->epmem_autosave_filename, thisAgent->epmem_save_freq);
    }
    print(thisAgent, "\n Total CPU time used by the EpMem module: %g seconds",
          timer_value(&(thisAgent->epmem_total_time)));

    //Print info from each existing epmem header
    for(i = thisAgent->epmem_header_stack->size - 1; i >= 0; i--)
    {
        print(thisAgent, "\n");

        h = (epmem_header *)get_arraylist_entry(thisAgent, thisAgent->epmem_header_stack,i);
        print_with_symbols(thisAgent, "\nState %y", h->state);

        if (h->curr_memory == NULL)
        {
            print(thisAgent, "\n          Currently retrieved memory: none");
            break;
        }
        
        print(thisAgent, "\n     Currently retrieved memory: %d",
              h->curr_memory->index);

        if (h->temporal_cmd->name != NULL)
        {
            print(thisAgent, "\n         Current temporal command issued: %s",
                  h->temporal_cmd->name);
        }//if
        else
        {
            print(thisAgent, "\n         Current command issued: none");
        }//else
            
        print(thisAgent, "\n  Currently prohibited memories:");
        if (h->prohibit_list->size > 0)
        {
            for(j = 0; j < h->prohibit_list->size; j++)
            {
                long nProbID = (long)get_arraylist_entry(thisAgent,
                                                         h->prohibit_list,
                                                         j);
                print(thisAgent, " %ld", nProbID);
            }
        }//if
        else
        {
            print(thisAgent, " none");
        }


        //Metadata Info
        print(thisAgent, "\n                Sequence length: %d", h->retrieval_count);
        print(thisAgent, "\n                    Match score: %f", h->last_match_score);
        print(thisAgent, "\n                       Cue size: %d", h->last_cue_size);
        print(thisAgent, "\n         Normalized Match Score: %f",
              h->last_match_score / (float)h->last_cue_size);
        print(thisAgent, "\n              Match Cardinality: %f",
              (float)h->last_match_size / (float)h->last_cue_size);
        
    }//for
    
}//epmem_print_status


/* ===================================================================
   print_wmetree                           *RECURSIVE*

   Prints an ASCII graphic representation of the wmetree rooted by the
   given node.  This is VERY USEFUL for debugging but not actually
   used during normal epmem operation.

   node - wmetree to print
   indent - how many spaces to indent
   depth - how many levels to descend into the tree.  A depth of zero
           will just print the node.  
   
   Created: 09 Nov 2002
   =================================================================== */
void print_wmetree(agent *thisAgent, wmetree *node, int indent, int depth)
{
    unsigned long hash_value;
    wmetree *child;
    
    if (node == NULL) return;

    if (node->parent != NULL) // check for root
    {
        if (indent)
        {
            print(thisAgent, "%*s+--", indent, "");
        }
        print(thisAgent, "%s",node->attr);
        switch(node->val_type)
        {
            case SYM_CONSTANT_SYMBOL_TYPE:
                print(thisAgent, " %s", node->val.strval);
                break;
            case INT_CONSTANT_SYMBOL_TYPE:
                print(thisAgent, " %ld", node->val.intval);
                break;
            case FLOAT_CONSTANT_SYMBOL_TYPE:
                print(thisAgent, " %f", node->val.floatval);
                break;
            default:
                break;
        }//switch
        print(thisAgent, "\n");
    }//if

    if (depth > 0)
    {
        for (hash_value = 0; hash_value < node->children->size; hash_value++)
        {
            child = (wmetree *) (*(node->children->buckets + hash_value));
            for (; child != NIL; child = child->next)
            {
                print_wmetree(thisAgent, child, indent + 3, depth - 1);
            }
        }
    }
}//print_wmetree

/* ===================================================================
   epmem_find_wmetree_entry

   Finds a descendent of a given wmetree node that has a particular id
   and attribute and is in a given memory.  If the specified wmetree
   node is the root (thisAgent->epmem_wmetree) then it is assumed to
   be a wildcard match.  Returns NULL if not found.

   epmem - an arraylist of actwme structs to search
   id - the id we're searching for (which will be the "parent" pointer
        in the target wmetree struct)
   s  - the attribute we're searching for

   Created: 20 Feb 2004
   =================================================================== */
wmetree *epmem_find_wmetree_entry(agent *thisAgent,
                                  arraylist *epmem,
                                  wmetree *id,
                                  char *s)
{
    int i;

    if (epmem == NULL) return NULL;
    
    for(i = 0; i < epmem->size; i++)
    {
        wmetree *node = ((actwme *)get_arraylist_entry(thisAgent, epmem, i))->node;
        if ( (id == thisAgent->epmem_wmetree) || (id == node->parent) )
        { 
            if (strcmp(node->attr, s) == 0)
            {
                return node;
            }
        }
    }//for

    return NULL;
    
}//epmem_find_wmetree_entry

/* ===================================================================
   epmem_find_actwme_entry

   Finds an actwme entry in a given episodic memory that points to a
   given wmetree.

   Returns NULL if not found.

   Created: 29 Nov 2004
   =================================================================== */
actwme *epmem_find_actwme_entry(agent *thisAgent,
                                arraylist *epmem,
                                wmetree *target)
{
    int i;

    if (epmem == NULL) return NULL;
    
    for(i = 0; i < epmem->size; i++)
    {
        actwme *entry = (actwme *)get_arraylist_entry(thisAgent, epmem, i);
        if (entry->node == target) return entry;
    }//for

    return NULL;
    
}//epmem_find_actwme_entry

/* ===================================================================
   epmem_find_similar_actwme_entry

   Finds an actwme entry in a given episodic memory that is similar
   but not the same as a given wmetree node.  To be similar, it must
   have all these traits:
   - shares the same parent as a given wmetree node
   - has the same attribute.
   - has a value with the same type

   Returns NULL if not found.

   Created: 01 Dec 2006
   =================================================================== */
actwme *epmem_find_similar_actwme_entry(agent *thisAgent,
                                        arraylist *epmem,
                                        wmetree *target)
{
    int i;

    for(i = 0; i < epmem->size; i++)
    {
        actwme *entry = (actwme *)get_arraylist_entry(thisAgent, epmem, i);
        if (entry->node == target) continue; // Don't return an exact match
        if ( (entry->node->parent == target->parent)
             && (entry->node->val_type == target->val_type)
             && (strcmp(entry->node->attr, target->attr) == 0) )
        {
            return entry;
        }
    }//for

    return NULL;
    
}//epmem_find_similar_actwme_entry

/* ===================================================================
   print_memory                       *RECURSIVE*

   Prints the content of an episodic memory.
   
   thisAgent - duh
   epmem - an arraylist containing the epmem
   node - the wmetree this memory is drawn from (probably thisAgent->epmem_wmetree)
   indent - number of space to indent
   depth - how deep to traverse the tree
   attrs - an optional string containing a list of attributes that
           should be printed
   
   Created: 01 Mar 2004
   =================================================================== */
void print_memory(agent *thisAgent,
                  arraylist *epmem,
                  wmetree *node,
                  int indent,
                  int depth,
                  char *attrs = NULL)
{
    int i;
    unsigned long hash_value;
    wmetree *child;
    actwme *aw;

    if (epmem == NULL) return;
    if (node == NULL) return;
    
    if (node->parent != NULL) // check for root
    {
        int bFound = FALSE;
        
        //Find out if this node is in the arraylist
        for(i = 0; i < epmem->size; i++)
        {
            aw = (actwme *)get_arraylist_entry(thisAgent, epmem, i);
            if (aw->node == node)
            {
                bFound = TRUE;
                break;
            }
        }
        
        if (!bFound) return;

        //Check to make sure I have an attr I'm allowed to print
        if ( (attrs != NULL) && (!strstr(attrs, node->attr)) ) return;
        
        if (indent)
        {
            print(thisAgent, "%*s+--", indent, "");
        }
        print(thisAgent, "%s",node->attr);
        switch(node->val_type)
        {
            case SYM_CONSTANT_SYMBOL_TYPE:
                print(thisAgent, " %s", node->val.strval);
                break;
            case INT_CONSTANT_SYMBOL_TYPE:
                print(thisAgent, " %ld", node->val.intval);
                break;
            case FLOAT_CONSTANT_SYMBOL_TYPE:
                print(thisAgent, " %f", node->val.floatval);
                break;
            default:
                break;
        }//switch

        //print the activation level
        print(thisAgent, "(%d)", aw->activation);

    }//if
    print(thisAgent, "\n");

    if (depth > 0)
    {
        for (hash_value = 0; hash_value < node->children->size; hash_value++)
        {
            child = (wmetree *) (*(node->children->buckets + hash_value));
            for (; child != NIL; child = child->next)
            {
                print_memory(thisAgent, epmem, child, indent + 3, depth - 1, attrs);
            }
        }
    }
    
}//print_memory

/* ===================================================================
   print_memory_match                        *RECURSIVE*

   Print the WMEs in a memory that match (or do not match) a given
   cue.  
   
   
   thisAgent - duh
   epmem - an arraylist containing the epmem
   node - the wmetree this memory is drawn from (almost always
          thisAgent->epmem_wmetree)
   indent - number of space to indent
   cue - the cue
   negcue - the negative cue
   reverse - (boolean) if TRUE then print the non-matching rather
             than matching entries
   
   Created: 20 Nov 2006
   =================================================================== */
void print_memory_match(agent *thisAgent, arraylist *epmem,
                        wmetree *node, int indent,
                        arraylist *cue, arraylist *negcue,
                        int reverse)
{
    unsigned long hash_value;
    wmetree *child;
    actwme *aw = NULL;
    actwme *aw_cue = NULL;
    actwme *aw_negcue = NULL;
    actwme *aw_similar = NULL;
    int match = FALSE;
    wmetree *cueNode = NULL;

    if (epmem == NULL) return;
    if (node == NULL) return;
    if ((cue == NULL) && (negcue == NULL)) return;
    
    if (node->parent != NULL) // check for root
    {
        //Find out if this node is in the episodic memory
        aw = epmem_find_actwme_entry(thisAgent, epmem, node);
        
        //Find out if this node is in the positive cue
        if (cue != NULL)
        {
            aw_cue = epmem_find_actwme_entry(thisAgent, cue, node);
        }
        
        //If it's not the positive cue, then check the negative cue
        if ( (aw_cue == NULL) && (negcue != NULL) )
        {
            aw_negcue = epmem_find_actwme_entry(thisAgent, negcue, node);
        }

        //If all the values are NULL then return
        if ( (aw_cue == NULL) && (aw_negcue == NULL) )
        {
            return;
        }
        
        //Figure out if this node is a match to the cue.  A match
        //is the case where the node is in the memory and the positive cue
        //OR it's not in the memory but it is in the negative cue
        match = ( ( (aw != NULL) && (aw_cue != NULL) )
                  || ( (aw == NULL) && (aw_negcue != NULL) ) );
        
        //Print the node if it's a match (or NOT a match if
        //the reverse boolean was set).
        if ( (!reverse && match ) || (reverse && !match) )
        {
            //If the node is not in the memory, try to find a similar node
            //to print instead
            if (aw == NULL)
            {
                aw = epmem_find_similar_actwme_entry(thisAgent, epmem, node);
                if (aw != NULL)
                {
                    cueNode = node;
                    node = aw->node;
                }
            }//if
            
            
            if (indent > 0)
            {
                print(thisAgent, "%*s+--", indent, "");
            }
            print(thisAgent, "%s",node->attr);

            //Print the value of the node (or a default if not available)
            if (aw == NULL)
            {
                if (node->children->count == 0)
                {
                    print(thisAgent, " MISSING");
                }
            }
            else
            {
                switch(node->val_type)
                {
                    case SYM_CONSTANT_SYMBOL_TYPE:
                        print(thisAgent, " %s", node->val.strval);
                        break;
                    case INT_CONSTANT_SYMBOL_TYPE:
                        print(thisAgent, " %ld", node->val.intval);
                        break;
                    case FLOAT_CONSTANT_SYMBOL_TYPE:
                        print(thisAgent, " %f", node->val.floatval);
                        break;
                    default:
                        break;
                }//switch
            }

            //print the activation level if available and relevant
            if ( (aw != NULL) && (node->children->count == 0) )
            {
                print(thisAgent, "(%d)", aw->activation);
            }

            //if it was a negative cue match then note that
            if ((aw != NULL) && (aw_negcue != NULL) && (cueNode == NULL))
            {
                print(thisAgent, " <--matches negative cue");
            }
            
            //If the node wasn't in the memory, print the cue's value
            //for user reference
            if ((aw == NULL) && (cueNode == NULL))
            {
                if (aw_cue != NULL)
                {
                    cueNode = aw_cue->node;
                }
                else if (aw_negcue != NULL)
                {
                    cueNode = aw_negcue->node;
                }
            }
            if (cueNode != NULL)
            {
                if ( (aw_negcue != NULL) && (cueNode == aw_negcue->node) )
                {
                    print(thisAgent, " [neg ");
                }
                else 
                {
                    print(thisAgent, " [");
                }
                switch(cueNode->val_type)
                {
                    case SYM_CONSTANT_SYMBOL_TYPE:
                        print(thisAgent, "cue value: %s]", cueNode->val.strval);
                        break;
                    case INT_CONSTANT_SYMBOL_TYPE:
                        print(thisAgent, "cue value: %ld]", cueNode->val.intval);
                        break;
                    case FLOAT_CONSTANT_SYMBOL_TYPE:
                        print(thisAgent, "cue value: %f]", cueNode->val.floatval);
                        break;
                    default:
                        print(thisAgent, "cue match]");
                        break;
                }//switch
            }//if
            
            print(thisAgent, "\n");
        }//if
    }//if
    
    for (hash_value = 0; hash_value < node->children->size; hash_value++)
    {
        child = (wmetree *) (*(node->children->buckets + hash_value));
        for (; child != NIL; child = child->next)
        {
            print_memory_match(thisAgent, epmem, child,
                               indent + 3, cue, negcue, reverse);
        }
    }
    
}//print_memory_match

/* ===================================================================
   epmem_print_memory_by_id

   This function will print an episodic memory from an agent's memory
   store based on its id.  (Used in response to the epmem -p command.)
   
   thisAgent - duh
   id        - the memory id
   
   Created: 14 Nov 2006
   =================================================================== */
void epmem_print_memory_by_id(agent *thisAgent, int id)
{
    episodic_memory *epmem;
    char buf[1024];
    
    if (id >= thisAgent->epmem_memories->size)
    {
        sprintf(buf, "Invalid memory id (%d).  Pick a number between %d and %d.",
                id, 1, thisAgent->epmem_memories->size);
        print(thisAgent, buf);
        return;
    }//if

    epmem = (episodic_memory *)get_arraylist_entry(thisAgent,
                                                   thisAgent->epmem_memories,
                                                   id);
    
    print_memory(thisAgent, epmem->content, thisAgent->epmem_wmetree, 2, 1000);
    
}//epmem_print_memory_by_id

/* ===================================================================
   find_nth_best_matches

   This function prints the top N matches for a given memory
   cue excepting the one that's already been retrieved.
   
   thisAgent - duh
   num - the number of matches to retrieve (i.e., "N")

   CAVEAT:  Caller must deallocate the list
   
   Created: 06 Dec 2006
   =================================================================== */
arraylist *find_nth_best_matches(agent *thisAgent, epmem_header *h,
                                 arraylist *cue, arraylist *negcue, int num)
{
    epmem_header *dummy;
    int i;
    arraylist *matches = make_arraylist(thisAgent, num);

    //Create a dummy header that's a copy with a unique prohibit list
    dummy = copy_epmem_header(thisAgent, h);
    dummy->prohibit_list = copy_arraylist(thisAgent, h->prohibit_list);

    //Retrieve other matches by continually prohibiting the current one
    for(i = 0; i < num; i++)
    {
        append_entry_to_arraylist(thisAgent,
                                  dummy->prohibit_list,
                                  (void *)dummy->curr_memory->index);
        
        dummy->curr_memory = find_best_match(thisAgent, dummy, cue, negcue);

        //Check for no match and stop
        if (dummy->curr_memory == NULL)
        {
            break;
        }
            
        append_entry_to_arraylist(thisAgent,
                                  matches,
                                  (void *)dummy->curr_memory->index);
    }//for

    //Cleanup
    destroy_arraylist(thisAgent, dummy->prohibit_list);
    free_memory(thisAgent, dummy, MISCELLANEOUS_MEM_USAGE);
    
    return matches;
    
}//print_nth_best_matches

/* ===================================================================
   find_epmem_header_from_id

   Given a state number (e.g., 76 indicates the state s76) this
   function finds the epmem header attached to that state if it exists

   =================================================================== */
epmem_header *find_epmem_header_from_id(agent *thisAgent, int state_num)
{
    int i;
    epmem_header *h = NULL;
    
    for(i = thisAgent->epmem_header_stack->size - 1; i >= 0; i--)
    {
        h = (epmem_header *)get_arraylist_entry(thisAgent, thisAgent->epmem_header_stack,i);
        if (state_num == h->state->id.name_number)
        {
            break;
        }
    }

    return h;

}//find_epmem_header_from_id

/* ===================================================================
   epmem_print_cue_comparison

   This function will print information about how well a given memory
   matches the cue on a given state.  
   
   thisAgent - duh
   state_num - the state number.  For example, if the symbol of the
               desired state is S76 then this argument should be 76.
   epmem_id  - the index of the memory
   
   Created: 08 Dec 2006
   =================================================================== */
void epmem_print_cue_comparison(agent *thisAgent, int state_num, int epmem_id)
{
    epmem_header *h;

    /*
     * Step 1:  Find the specified state and verify it has an active retrieval
     */
    h = find_epmem_header_from_id(thisAgent, state_num);
    if (h == NULL)
    {
        print(thisAgent, "\nERROR:  The given state (S%d) does not exist.", state_num);
        return;
    }

    if (h->curr_memory == NULL)
    {
        print(thisAgent, "\nERROR:  The given state (S%d) has no currently retrieved memory.", state_num);
        return;
    }

    /*
     * Step 2:  Find the specified memory
     */
    episodic_memory *epmem;
    char buf[1024];
    
    if (epmem_id >= thisAgent->epmem_memories->size)
    {
        sprintf(buf, "Invalid memory id (%d).  Pick a number between %d and %d.",
                epmem_id, 1, thisAgent->epmem_memories->size);
        print(thisAgent, buf);
        return;
    }//if

    epmem = (episodic_memory *)get_arraylist_entry(thisAgent,
                                                   thisAgent->epmem_memories,
                                                   epmem_id);
    
    /*
     * Step 3:  Retrieve the cue info
     */
    arraylist *al_query;
    arraylist *al_negquery;
    tc_number tc;
    
    //Create an arraylist representing the current query
    if (h->query_cmd != NULL)
    {
        al_query = make_arraylist(thisAgent, 32);
        tc = h->query_cmd->id.tc_num + 1;
        update_wmetree(thisAgent, thisAgent->epmem_wmetree, h->query_cmd, al_query, tc);
    }
    
    //Create an arraylist representing the current negative query
    if (h->negquery_cmd != NULL)
    {
        al_negquery = make_arraylist(thisAgent, 32);
        tc = h->negquery_cmd->id.tc_num + 1;
        update_wmetree(thisAgent, thisAgent->epmem_wmetree,
                       h->negquery_cmd, al_negquery, tc);
    }

    /*
     * Step 4:  Print the matching and non-matching parts of the memory
     */
    print(thisAgent, "\n\nMATCHING ENTRIES IN EPMEM %d:\n", epmem->index);
    print_memory_match(thisAgent,
                       epmem->content,
                       thisAgent->epmem_wmetree,
                       2, al_query, al_negquery,
                       FALSE);
    print(thisAgent, "\nNON-MATCHING ENTRIES IN EPMEM %d:\n", epmem->index);
    print_memory_match(thisAgent,
                       epmem->content,
                       thisAgent->epmem_wmetree,
                       2, al_query, al_negquery,
                       TRUE);

    /*
     * Step 5: Cleanup
     */
    destroy_arraylist(thisAgent, al_query);
    destroy_arraylist(thisAgent, al_negquery);
    
    
}//epmem_print_cue_comparison

/* ===================================================================
   epmem_print_match_diagnostic

   This function will print complete information about the currently
   retrieved epmem in the given state and how it was retrieved.  This
   is used in response to the "epmem -m" command.
   
   thisAgent - duh
   state_num - the state number.  For example, if the symbol of the
               desired state is S76 then this argument should be 76.
   
   Created: 15 Nov 2006
   =================================================================== */
void epmem_print_match_diagnostic(agent *thisAgent, int state_num)
{
    int i;
    epmem_header *h;

    /*
     * Step 1:  Find the specified state and verify it has an epmem
     */
    //Find the appropriate epmem header
    h = find_epmem_header_from_id(thisAgent, state_num);
    if (h == NULL)
    {
        print(thisAgent, "\nERROR:  The given state (S%d) does not exist.", state_num);
        return;
    }

    if (h->curr_memory == NULL)
    {
        print(thisAgent, "\nERROR:  The given state (S%d) has no currently retrieved memory.", state_num);
        return;
    }

    print(thisAgent, "\nEPISODIC MATCH DIAGNOSTIC");
    print(thisAgent, "\n-------------------------");
    
    /*
     * Step 2:  Retrieve the Cue Info
     */
    arraylist *al_query = NULL;
    arraylist *al_negquery = NULL;
    tc_number tc;
    
    //Create an arraylist representing the current query
    if (h->query_cmd != NULL)
    {
        al_query = make_arraylist(thisAgent, 32);
        tc = h->query_cmd->id.tc_num + 1;
        update_wmetree(thisAgent, thisAgent->epmem_wmetree, h->query_cmd, al_query, tc);
    }
    
    //Create an arraylist representing the current negative query
    if (h->negquery_cmd != NULL)
    {
        al_negquery = make_arraylist(thisAgent, 32);
        tc = h->negquery_cmd->id.tc_num + 1;
        update_wmetree(thisAgent, thisAgent->epmem_wmetree, h->negquery_cmd, al_negquery, tc);
    }
    
    /*
     * Step 3: Print Metadata Info
     */
    print(thisAgent, "\n       Retrieved Epmem: #%d", h->curr_memory->index);
    print(thisAgent, "\n       Sequence length: %d", h->retrieval_count);
    print(thisAgent, "\n           Match score:  %f", h->last_match_score);
    print(thisAgent, "\n              Cue size:  %d", h->last_cue_size);
    print(thisAgent, "\nNormalized Match Score:  %f",
          h->last_match_score / (float)h->last_cue_size);
    print(thisAgent, "\n   Card/Act Bias Ratio:  %f", g_card_act_ratio);
    print(thisAgent, "\n     Match Cardinality:  %d", h->last_match_size);
    print(thisAgent, "\n  Cardinality Fraction:  %f",
          (float)h->last_match_size / (float)h->last_cue_size);

    /*
     * Step 4: Retrieve and print the next closest matches
     */
    arraylist *best_matches;
    arraylist *best_matches_by_act;
    arraylist *best_matches_by_card;
    episodic_memory *epmem;
    long id;
    float save_ratio = g_card_act_ratio;

    //Best matches using the current setting
    best_matches = find_nth_best_matches(thisAgent, h, al_query, al_negquery, 5);
    print(thisAgent, "\n\nNext 5 best matches: ");
    for(i = 0; i < best_matches->size; i++)
    {
        id = (long)get_arraylist_entry(thisAgent, best_matches, i);
        epmem = (episodic_memory *)get_arraylist_entry(thisAgent,
                                                       thisAgent->epmem_memories,
                                                       id);

        print(thisAgent, "\n              #%ld: score=%f activation=%f cardinality=%d",
              id, epmem->match_score, epmem->act_total, epmem->num_matches);
    }

    //Best matches using activation only
    g_card_act_ratio = (float)0.000001;
    best_matches_by_act = find_nth_best_matches(thisAgent, h, al_query, al_negquery, 5);
    print(thisAgent, "\n\nTop 5 memories by activation only: ");
    for(i = 0; i < best_matches_by_act->size; i++)
    {
        id = (long)get_arraylist_entry(thisAgent, best_matches_by_act, i);
        epmem = (episodic_memory *)get_arraylist_entry(thisAgent,
                                                       thisAgent->epmem_memories,
                                                       id);

        print(thisAgent, "\n              #%ld: activation=%f cardinality=%d ",
              id, epmem->act_total, epmem->num_matches);
    }
    
    //Best matches using cardinality only
    g_card_act_ratio = (float)1000000.0;
    best_matches_by_card = find_nth_best_matches(thisAgent, h, al_query, al_negquery, 5);
    print(thisAgent, "\n\nTop 5 memories by cardinality: ");
    for(i = 0; i < best_matches_by_card->size; i++)
    {
        id = (long)get_arraylist_entry(thisAgent, best_matches_by_card, i);
        epmem = (episodic_memory *)get_arraylist_entry(thisAgent,
                                                       thisAgent->epmem_memories,
                                                       id);

        print(thisAgent, "\n              #%ld: activation=%f cardinality=%d ",
              id, epmem->act_total, epmem->num_matches);
    }
    
    //reset the global g_card_act_ratio
    g_card_act_ratio = save_ratio;
    
    /*
     * Step 5:  Print the matching and non-matching parts of the memory
     */
    print(thisAgent, "\n\nMATCHING ENTRIES IN EPMEM %d:\n", h->curr_memory->index);
    print_memory_match(thisAgent,
                       h->curr_memory->content,
                       thisAgent->epmem_wmetree,
                       2, al_query, al_negquery,
                       FALSE);
    print(thisAgent, "\nNON-MATCHING ENTRIES IN EPMEM %d:\n", h->curr_memory->index);
    print_memory_match(thisAgent,
                       h->curr_memory->content,
                       thisAgent->epmem_wmetree,
                       2, al_query, al_negquery,
                       TRUE);

    /*
     * Step 6: Cleanup
     */
    destroy_arraylist(thisAgent, al_query);
    destroy_arraylist(thisAgent, al_negquery);
    
    
}//epmem_print_match_diagnostic

/* ===================================================================
   epmem_print_memory_comparison          *RECURSIVE*

   This function will print a side-by-side comparison of two
   episodic memories.  It only prints WMEs that are different
   between the memories.
   
   thisAgent - duh
   epmem1/epmem2 - the memories to print
   node - the wmetree this memory is drawn from (probably thisAgent->epmem_wmetree)
   indent - number of space to indent
   depth - how deep to traverse the tree
   
   Created: 07 Dec 2006
   =================================================================== */
void epmem_print_memory_comparison(agent *thisAgent,
                                   arraylist *epmem1, arraylist *epmem2,
                                   wmetree *node, int indent, int depth)
{
    int i;
    unsigned long hash_value;
    wmetree *child;
    actwme *aw1 = NULL;
    actwme *aw2 = NULL;
    int bEqual = FALSE;
    char buf[512];
    int padLength = 0;

    if (node == NULL) return;
    if (epmem1 == NULL) return;
    if (epmem2 == NULL) return;
    if (indent < 0) indent = 0;

    if (node->parent != NULL) // check for root
    {
        int bFound = FALSE;
        
        //Find out if this node is in each arraylist
        for(i = 0; i < epmem1->size; i++)
        {
            aw1 = (actwme *)get_arraylist_entry(thisAgent, epmem1, i);
            if (aw1->node == node)
            {
                bFound = TRUE;
                break;
            }
            aw1 = NULL;
        }
        
        for(i = 0; i < epmem2->size; i++)
        {
            aw2 = (actwme *)get_arraylist_entry(thisAgent, epmem2, i);
            if (aw2->node == node)
            {
                bFound = TRUE;
                break;
            }
            aw2 = NULL;
        }
        
        if (!bFound) return;

        //Don't print if neither of the memories has a positive activation
        if (!( ((aw1 != NULL) && (aw1->activation >= 0))
               || ((aw2 != NULL) && (aw2->activation >= 0)) ))
        {
            bEqual = TRUE;
        }

        //Don't print if both memories have the entry at the same activation
        //level
        if ( (aw1 != NULL) && (aw2 != NULL)
             && (aw1->activation == aw2->activation) )
        {
            bEqual = TRUE;
        }

        //If it's still ok to print the do so.
        if (!bEqual)
        {
            //Print the WME to a buffer
            buf[0] = '\0';
            switch(node->val_type)
            {
                case SYM_CONSTANT_SYMBOL_TYPE:
                    sprintf(buf, "%*s+--%s %s", indent, "", node->attr, node->val.strval);
                    break;
                case INT_CONSTANT_SYMBOL_TYPE:
                    sprintf(buf, "%*s+--%s %ld", indent, "", node->attr, node->val.intval);
                    break;
                case FLOAT_CONSTANT_SYMBOL_TYPE:
                    sprintf(buf, "%*s+--%s %f", indent, "", node->attr, node->val.floatval);
                    break;
                default:
                    sprintf(buf, "%*s+--%s", indent, "", node->attr);
                    break;
            }//switch

            //Truncate the buffer if necessary
            if (strlen(buf) >= 33)
            {
                buf[33] = '\0';
            }

            //Calculate the required pad lengths
            padLength = 38 - strlen(buf);
            if (aw1 != NULL)
            {
                if (aw1->activation > 99)
                {
                    padLength -= 3;
                }
                else if ((aw1->activation > 9) || (aw1->activation < 0))
                {
                    padLength -= 2;
                }
                else
                {
                    padLength -= 1;
                }
            }//if            
            //Print as necessary w/ activation levels
            if (aw1 != NULL)
            {
                print(thisAgent, "%s(%d)%*s", buf, aw1->activation, padLength, "");
            }
            else
            {
                print(thisAgent, "                                        ");
            }

            if (aw2 != NULL)
            {
                print(thisAgent, "%s(%d)", buf, aw2->activation);
            }
                
            
            print(thisAgent, "\n");
        }//if
    }//if

    
    if (depth > 0)
    {
        for (hash_value = 0; hash_value < node->children->size; hash_value++)
        {
            child = (wmetree *) (*(node->children->buckets + hash_value));
            for (; child != NIL; child = child->next)
            {
                epmem_print_memory_comparison(thisAgent, epmem1, epmem2,
                                              child, indent+3, depth -1);
            }
        }
    }//if
    
}//epmem_print_memory_comparison

/* ===================================================================
   epmem_print_memory_comparison_by_id

   This function will print a side-by-side comparison of two
   episodic memories.
   
   thisAgent - duh
   id1/id2 - ids of the memories to print
   
   Created: 07 Dec 2006
   =================================================================== */
void epmem_print_memory_comparison_by_id(agent *thisAgent, int id1, int id2)
{
    episodic_memory *mem1;
    episodic_memory *mem2;
    char buf[512];

    //Check for bad IDs
    if (id1 >= thisAgent->epmem_memories->size)
    {
        sprintf(buf, "Invalid memory id (%d).  Pick a number between %d and %d.",
                id1, 1, thisAgent->epmem_memories->size);
        print(thisAgent, buf);
        return;
    }//if
    if (id2 >= thisAgent->epmem_memories->size)
    {
        sprintf(buf, "Invalid memory id (%d).  Pick a number between %d and %d.",
                id2, 1, thisAgent->epmem_memories->size);
        print(thisAgent, buf);
        return;
    }//if


    //Retrieve the memory contents
    mem1 = (episodic_memory *)get_arraylist_entry(thisAgent,
                                                   thisAgent->epmem_memories,
                                                   id1);
    mem2 = (episodic_memory *)get_arraylist_entry(thisAgent,
                                                   thisAgent->epmem_memories,
                                                   id2);

    //Print the comparison
    epmem_print_memory_comparison(thisAgent, mem1->content, mem2->content,
                                  thisAgent->epmem_wmetree, 2, 1000);
    

}//epmem_print_memory_comparison_by_id

/* ===================================================================
   find_child_node

   Given a wmetree node and a wme, this function returns the child
   node of the given node that represents that WME (or NULL).
   
   Created: 12 Jan 2004
   =================================================================== */
wmetree *find_child_node(wmetree *node, wme *w)
{
    unsigned long hash_value;
    wmetree *child;

    hash_value = hash_wme(w, node->children->log2size);
    child = (wmetree *) (*(node->children->buckets + hash_value));
    for (; child != NIL; child = child->next)
    {
        if (wme_equals_node(w, child))
        {
            return child;
        }
    }
    
    return NULL;
    
}//find_child_node



/* ===================================================================
   add_node_to_memory

   This function adds a wmetree node to a given episodic memory.
   
   Created: 12 Jan 2004
   =================================================================== */
void add_node_to_memory(agent *thisAgent,
                        arraylist *epmem,
                        wmetree *node,
                        int activation)
{
    //Allocate and init a new actwme
    actwme *aw = (actwme *)allocate_memory(thisAgent, sizeof(actwme),
                                           MISCELLANEOUS_MEM_USAGE);
    aw->node = node;
    aw->activation = activation;
    
    append_entry_to_arraylist(thisAgent, epmem, (void *)aw);
    
}//add_node_to_memory

/* ===================================================================
   handle_relation

   This function examines a given wme found in an episodic memory cue
   to see if it's a special relation for a cue (e.g.,
   "greater--than").  If it is, it extracts the relation info and
   updates the given node with that info.

   The function returns TRUE if a relation was handled and false otherwise. 
   
   Created: 18 Jul 2006
   =================================================================== */
int handle_relation(agent *thisAgent, wmetree *node, wme *w)
{
    enum wmetree_relation relation;
    wmetree *tmpnode;

    //Check for a relation
    if (wme_has_value(w, "greater--than", NULL))
    {
        relation = GREATER_THAN;
    }
    else if (wme_has_value(w, "less--than", NULL))
    {
        relation = LESS_THAN;
    }
    else return FALSE;              // no relation found

    
    //Get the values from the WME using make_wmetree_node
    //to make a temporary node
    tmpnode = make_wmetree_node(thisAgent, w);
    node->relation = relation;
    node->val_type = tmpnode->val_type;
    node->val = tmpnode->val;

    //Deallocate the temporary node
    if (node->val_type == SYM_CONSTANT_SYMBOL_TYPE)
    {
        //This prevents destroy_wmetree from deallocating the string
        tmpnode->val_type = INT_CONSTANT_SYMBOL_TYPE;
        tmpnode->val.intval = 0;
    }
    destroy_wmetree(thisAgent, tmpnode);

    return TRUE;
    
}//handle_relation

/* ===================================================================
   update_wmetree

   Updates the wmetree given a pointer to a corresponding wme in working
   memory.  The wmetree node is assumed to be initialized and empty.
   Each wme that is discovered by this algorithm is also added to a given
   episodic memory.

   If this function finds a ^superstate WME it does not traverse that link.
   Instead, it records the find and returns it to the caller.  The caller
   can then call update_wmetree again if desired.

   *This is a critical function in the epmem operation and is worth
   understanding completely.  It is used as part of the process of
   recording a new episodic memory.  It is also used to process
   episodic memory cues.

   node - the root of the WME tree to be updated
   sym - the root of the tree in working memory to update it with
   epmem - this function generates an arraylist of actwme structs
           representing all the nodes in the wmetree that are
           referenced by the working memory tree rooted by sym.
   tc - a transitive closure number for avoiding loops in working mem

   Created: 09 Jan 2004
   Updated: 23 Feb 2004 - made breadth first and non-recursive
   =================================================================== */
Symbol *update_wmetree(agent *thisAgent,
                       wmetree *node,
                       Symbol *sym,
                       arraylist *epmem,
                       tc_number tc)
{
    wme **wmes = NULL;
    wmetree *childnode;
    int len = 0;
    int i;
    Symbol *ss = NULL;
    arraylist *syms = make_arraylist(thisAgent, 32);
    int pos = 0;

    start_timer(thisAgent, &(thisAgent->epmem_updatewmetree_start_time));

    /*
     * The epmem arraylist is filled with wmetree equivalents of all
     * the WMEs that are found as children of the given Symbol (sym).
     * The pos pointer indicates the current place in the list.  If
     * pos reaches the end of the list we're done.
     */
    while(pos <= epmem->size)
    {
        start_timer(thisAgent, &(thisAgent->epmem_getaugs_start_time));
        wmes = epmem_get_augs_of_id(thisAgent,  sym, tc, &len );
        stop_timer(thisAgent, &(thisAgent->epmem_getaugs_start_time), &(thisAgent->epmem_getaugs_total_time));

        if (wmes != NULL)
        {
            for(i = 0; i < len; i++)
            {
                //Check for special case: relation specification
                if (handle_relation(thisAgent, node, wmes[i]))
                {
                    continue;
                }

                //Find the wmetree node that corresponds to this wme
                start_timer(thisAgent, &(thisAgent->epmem_findchild_start_time));
                childnode = find_child_node(node, wmes[i]);
                stop_timer(thisAgent, &(thisAgent->epmem_findchild_start_time), &(thisAgent->epmem_findchild_total_time));

                //If a corresponding node was not found, then create one
                if (childnode == NULL)
                {
                    childnode = make_wmetree_node(thisAgent, wmes[i]);
                    childnode->id = thisAgent->epmem_wmetree_size++;
                    childnode->parent = node;
                    childnode->depth = node->depth + 1;
                    add_to_hash_table(thisAgent, node->children, childnode);
                }

                //Check for special case: "superstate" (prevent other states
                //from being traversed).
                if (wme_has_value(wmes[i], "superstate", NULL))
                {
                    if ( (ss == NULL)
                         && (wmes[i]->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE) )
                    {
                        ss = wmes[i]->value;
                    }
                   continue;
                }

                //insert childnode into the arraylist
                start_timer(thisAgent, &(thisAgent->epmem_addnode_start_time));
                add_node_to_memory(thisAgent, epmem, childnode, decay_activation_level(thisAgent, wmes[i]));
                stop_timer(thisAgent, &(thisAgent->epmem_addnode_start_time), &(thisAgent->epmem_addnode_total_time));
                append_entry_to_arraylist(thisAgent, syms, (void *)wmes[i]->value);

            }//for
        }//if

        //Special Case:  no wmes found attached to the given symbol
        if (epmem->size == 0) break;

        //We've retrieved every WME in the query
        if (epmem->size == pos) break;
        
        node = ((actwme *)get_arraylist_entry(thisAgent, epmem,pos))->node;
        sym = (Symbol *)get_arraylist_entry(thisAgent, syms,pos);
        pos++;

        //Deallocate the last wmes list
        if (wmes != NULL)
        {
            free_memory(thisAgent, wmes, MISCELLANEOUS_MEM_USAGE);
        }
        
    }//while
    
    //Sort the memory's arraylist using the node pointers
    qsort( (void *)epmem->array,
           (size_t)epmem->size,
           sizeof( void * ),
           compare_actwme );

    //Deallocate the symbol list
    destroy_arraylist(thisAgent, syms);

    stop_timer(thisAgent, &(thisAgent->epmem_updatewmetree_start_time), &(thisAgent->epmem_updatewmetree_total_time));
    
    return ss;
    
}//update_wmetree

/* ===================================================================
   trim_epmem

   Removes entries from an episodic memory that have activation
   lower than the given value.

   NOTE:  This routine could be a lot more efficient.
   
   NOTE 16 July 2007: This routine is currently not used but may be
                      useful in the future so I've left it in.

   Created: 13 Dec 2004
   =================================================================== */
arraylist *trim_epmem(arraylist *epmem)
{
    if (epmem == NULL) return NULL;

    //Sort the memory's arraylist using activation
    qsort( (void *)epmem->array,
           (size_t)epmem->size,
           sizeof( void * ),
           compare_actwme_by_act );

    epmem->size = (int)((double)(epmem->size) * (1.0 - fraction_to_trim));

    //Sort the memory's arraylist using the node pointers
    qsort( (void *)epmem->array,
           (size_t)epmem->size,
           sizeof( void * ),
           compare_actwme );

    return epmem;
    
}//trim_epmem


/* ===================================================================
   forget_epmem

   This routine removes the contents of an episodic memory as well
   as associated entries in the working memory tree.  It leaves
   the episodic memory header intact to aid in record keeping (especially
   research related data collection).

   NOTE 16 July 2007: This routine is currently not used but may be
                      useful in the future so I've left it in.  

   Created: 05 Apr 2007
   =================================================================== */
void forget_epmem(agent *thisAgent, episodic_memory *epmem)
{
    int i;
    
    //Confirm the memory exists and hasn't already been forgotten
    if ((epmem == NULL)
        || (epmem->content == NULL)
        || (epmem->content->size == 0))
    {
        return;
    }
        
    //Clean up the actwme list (but not the associated wmetree nodes)
    for(i = 0; i < epmem->content->size; i++)
    {
        actwme *aw = (actwme *)get_arraylist_entry(thisAgent,
                                                   epmem->content,
                                                   i);
        remove_entry_from_arraylist_by_ptr(aw->node->assoc_memories, epmem);
    }//for

    //Remove the episodic memory content and replace with empty list
    destroy_arraylist(thisAgent, epmem->content);
    epmem->content = epmem->content = make_arraylist(thisAgent, 1);

    //%%%DEBUGGING
    print(thisAgent, "\nFORGOT MEMORY %d (last use %d)",
          epmem->index, epmem->last_ret);
    
    //Mark this epmem as removed
    epmem->last_ret = -1;

}//forget_epmem

/* ===================================================================
   epmem_forget_via_recency_of_use

   This routine scans all episodic memories for any that have not
   been created or retrieved since the last N were recorded (where
   N == epmem_age_limit).

   Created: 05 Apr 2007
   =================================================================== */
void epmem_forget_via_recency_of_use(agent *thisAgent)
{
    int i;
    episodic_memory *epmem;

    /*
     * BUGBUG: THIS FUNCTION IS CURRENTLY DISABLED
     *
     * I have performed some experiments with this function and the
     * agent's behavior doesn't seem to jive with my expectations.
     * Specifically, setting the epmem_age_limit very very high
     * should be the same as not forgetting at all.  But, instead
     * there is a significant drop in agent performance (in TankSoar
     * anyway).  I'm leaving this in the to-be-investigated pile for
     * the time being.  -:AMN: 16 July 07
     *
     */
    return;                     // remove this line to re-enable
    
    for (i = 1; i < thisAgent->epmem_memories->size; i++)
    {
        epmem =
            (episodic_memory *)get_arraylist_entry(thisAgent,
                                                   thisAgent->epmem_memories,
                                                   i);

        //Check for missing epmem
        if (epmem == NULL) continue;
        
        if (epmem->last_ret > 0)
        {
            int age = thisAgent->epmem_memories->size - epmem->last_ret;
            if (age > epmem_age_limit)
            {
                forget_epmem(thisAgent, epmem);
            }
        }//if

                
    }//for
    
}//epmem_forget_via_recency_of_use

/* ===================================================================
   record_epmem

   Once it has been determined that an epmem needs to be recorded,
   this routine manages all the steps for recording it.

   Note:  I had epmems with missing WMEs when I just incremented
          the tc number by one.  I found it had to be at least 3
          to work reliably.  A better solution would be to add
          my own tc_epmem number to the Symbol struct but that's
          awfully RAM instensive.
   
   Created: 12 Jan 2004
   =================================================================== */
void record_epmem(agent *thisAgent)
{
    tc_number tc;
    Symbol *sym;
    arraylist *curr_state;
    arraylist *next_state;
    int i;
    episodic_memory *new_epmem;

    //Allocate and initialize the new memory
    new_epmem = (episodic_memory *)allocate_memory(thisAgent,
                                                   sizeof(episodic_memory),
                                                   MISCELLANEOUS_MEM_USAGE);
    new_epmem->last_usage = -1;
    new_epmem->match_score = 0.0;
    new_epmem->act_total = 0.0;
    new_epmem->num_matches = 0;
    new_epmem->last_ret = thisAgent->epmem_memories->size;

    //Starting with bottom_goal and moving toward top_goal, add all
    //the current states to the wmetree and record the full WM
    //state as an arraylist of actwmes
    sym = (thisAgent->bottom_goal);
    
    //Do only top-state for now
    sym = (thisAgent->top_goal);  //%%%TODO: remove this later
    
    curr_state = NULL;
    next_state = NULL;
    while(sym != NULL)
    {
        next_state = make_arraylist(thisAgent, 128);
        next_state->next = curr_state;
        curr_state = next_state;

        tc = sym->id.tc_num + 3;//how much is enough?? (see note above)
        
        sym = update_wmetree(thisAgent, thisAgent->epmem_wmetree, sym, curr_state, tc);

        //Update the assoc_memories link on each wmetree node in curr_state
        for(i = 0; i < curr_state->size; i++)
        {
            actwme *curr_actwme = (actwme *)get_arraylist_entry(thisAgent, curr_state,i);
            wmetree *node = curr_actwme->node;
            int activation = curr_actwme->activation;

            //In order to be recorded, a WME must meet the following criteria:
            //1.  It must be a leaf WME (i.e., it has no children)
            //2.  It must be activated (i.e., it has a decay element)
            //3.  It must not be marked as ubiquitous
            if ( (node->children->count == 0)
                 && (activation != -1)
                 && (! node->ubiquitous) )
            {
                append_entry_to_arraylist(thisAgent, node->assoc_memories, (void *)new_epmem);

                //Test to see if the new arraylist has too many entries.
                //If so, this node has become too ubiquitous and will no
                //longer be used in mat ching
                if (thisAgent->epmem_memories->size > ubiquitous_max)
                {
                    float ubiquity =
                        ((float)node->assoc_memories->size) / ((float)thisAgent->epmem_memories->size);
                    if (ubiquity > ubiquitous_threshold)
                    {
                        node->ubiquitous = TRUE;
                        destroy_arraylist(thisAgent, node->assoc_memories);
                        node->assoc_memories = make_arraylist(thisAgent, 1);
                    }
                    
                }
            }//if
        }//for
    }//while

//      //Reduce the size of the episodic memory
//      new_epmem->content = trim_epmem(curr_state);
    new_epmem->content = curr_state;

    //Store the recorded memory
    new_epmem->index = thisAgent->epmem_memories->size;
    append_entry_to_arraylist(thisAgent, thisAgent->epmem_memories, (void *)new_epmem);

    
    //%%%DEBUGGING
    stop_timer(thisAgent, &(thisAgent->epmem_record_start_time), &(thisAgent->epmem_record_total_time));
    print(thisAgent, "\nRECORDED MEMORY %d:\n", thisAgent->epmem_memories->size - 1);
//      print_memory(thisAgent,
//                   ((episodic_memory *)get_arraylist_entry(thisAgent, thisAgent->epmem_memories,thisAgent->epmem_memories->size - 1))->content,
//                   thisAgent->epmem_wmetree, 0, 5);
    start_timer(thisAgent, &(thisAgent->epmem_record_start_time));

    
    //Check for old memories to forget
    epmem_forget_via_recency_of_use(thisAgent);
    
}//record_epmem


/* ===================================================================
   wme_array_diff()

   Given two arrays of wme* this routine determines how many are different.
   Both arrays must be sorted order.
   
   Created: 08 Nov 2002
   =================================================================== */
int wme_array_diff(wme **arr1, wme** arr2, int len)
{
    int count = len;
    int pos1 = 0;
    int pos2 = 0;

    while((pos1 < len) && (pos2 < len))
    {
        if (arr1[pos1] == arr2[pos2])
        {
            count--;
            pos1++;
            pos2++;
        }
        else if (arr1[pos1] < arr2[pos2])
        {
            pos1++;
        }
        else
        {
            pos2++;
        }
    }//while

    return count;

}//wme_array_diff

/* ===================================================================
   get_most_activated_wmes

   Use the decay wmes array to retrive the n most activated wmes.  The
   given array should already be allocated and of length n. This
   routine can can be directed to favor leaf wmes (i.e., a wme whose
   value is not an identifier) via the threshold parameter.  The
   threshold specifies how much more activated a non-leaf wme must be
   than a leaf wme for it to be acceptable.  This threshold is
   specified as an nonnegative integer indicating how many positions
   the leaf wme must be behind the non-leaf wme in the decay queue
   (i.e., (thisAgent->decay_timelist)).  A value of 0 for this
   threshold indicates no preference between non-leaf and leaf wmes.
   The  value MAX_DECAY assures that leaf WMEs will always
   be selected.

   This function returns the actual number of wmes that were placed in
   given array.

   See Also: consider_new_epmem_via_activation()
   
   Created: 06 Nov 2002
   Changed: 25 April 2003 - added leaf preference threshold
   =================================================================== */

int get_most_activated_wmes(agent *thisAgent, wme **active_wmes, int n, int nonleaf_threshold)
{
    decay_timelist_element *decay_list;
    int pos = 0;
    int decay_pos;              // current position in the decay array
    int nl_decay_pos = -1;      // non-leaf decay pos
    wme_decay_element *decay_element;
    int i;

    decay_list = (thisAgent->decay_timelist);
    decay_pos = (thisAgent->current_decay_timelist_element)->position;

    for(i = 0; i < MAX_DECAY; i++)
    {
        //Traverse the decay array backwards in order to get the most
        //activated wmes first
        decay_pos = decay_pos > 0 ? decay_pos - 1 : MAX_DECAY - 1;
        
        /*
         * Search for leaf wmes first
         */
        if (decay_list[decay_pos].first_decay_element != NULL)
        {
            decay_element = decay_list[decay_pos].first_decay_element;
            while (decay_element != NULL)
            {
                if (IS_LEAF_WME(decay_element->this_wme))
                {
                    active_wmes[pos] = decay_element->this_wme;
                    pos++;
                }
                decay_element = decay_element->next;
                if (pos == n) return pos;
            }
        }//if
        
        /*
         * Search for non-leaf wmes that are above the threshold
         */
        if (i >= nonleaf_threshold)
        {
            nl_decay_pos = decay_pos - nonleaf_threshold;
            if (nl_decay_pos < 0)
            {
                nl_decay_pos = nl_decay_pos + MAX_DECAY;
            }
        
            if (decay_list[nl_decay_pos].first_decay_element != NULL)
            {
                decay_element = decay_list[nl_decay_pos].first_decay_element;
                while (decay_element != NULL)
                {
                    if (! IS_LEAF_WME(decay_element->this_wme) )
                    {
                        active_wmes[pos] = decay_element->this_wme;
                        pos++;
                    }
                    
                    decay_element = decay_element->next;
                    if (pos == n) return pos;
                }
            }//if
        }//if
        
    }//for
    
    
    /*
     * In the rare case there aren't enough WMEs to fill the array
     * set the remaining entries to NULL.
     */
    if (pos < n)
    {
        for(i = pos; i < n; i++)
        {
            active_wmes[i] = NULL;
        }
    }//if

    return pos;

}//get_most_activated_wmes


/* ===================================================================
   consider_new_epmem_via_output() 

   This routine determines whether a new command has been placed on
   the output link.  If so, record_epmem() is called.

   Created: 01 Mar 2004
   =================================================================== */

void consider_new_epmem_via_output(agent *thisAgent)
{
    slot *s;
    wme *w;
    Symbol *ol = (thisAgent->io_header_output);
    int bNewMemory = FALSE;
    
    //Examine all the commands on the output link for any that have
    //appeared since the last memory was recorded
    for (s = ol->id.slots; s != NIL; s = s->next)
    {
        for (w = s->wmes; w != NIL; w = w->next)
        {
            if (w->timetag > thisAgent->epmem_last_tag)
            {
                bNewMemory = TRUE;
                thisAgent->epmem_last_tag = w->timetag;
            }
        }
    }

    //If there's a new command record a new memory
    if (bNewMemory)
    {
        record_epmem(thisAgent);         // The big one
    }

}//consider_new_epmem_via_output

/* ===================================================================
   consider_new_epmem_via_activation() 

   This routine decides whether the current state is worthy of being
   remembered as an episodic memory based upon recent changes in the
   top N most activated WMEs.  If so, record_epmem() is called.

   Created: 06 Nov 2002
   =================================================================== */

void consider_new_epmem_via_activation(agent *thisAgent)
{
    int i;

    i = get_most_activated_wmes(thisAgent,
                                thisAgent->epmem_curr_active_wmes,
                                num_active_wmes,
                                MAX_DECAY);
    if (i < num_active_wmes)
    {
        // no WMEs are activated right now
        return; 
    }

    //See if enough of these WMEs have changed
    qsort( (void *)thisAgent->epmem_curr_active_wmes, (size_t)num_active_wmes, sizeof( wme * ),compare_ptr );
    i = wme_array_diff(thisAgent->epmem_curr_active_wmes, thisAgent->epmem_prev_active_wmes, num_active_wmes);
    
    if ( i >= num_wmes_changed )
    {
        //Save the WMEs used for this memory in order to compare for
        //the next memory
        for(i = 0; i < num_active_wmes; i++)
        {
            thisAgent->epmem_prev_active_wmes[i] = thisAgent->epmem_curr_active_wmes[i];
        }
        
        record_epmem(thisAgent);         // The big one

    }

    
}//consider_new_epmem_via_activation


/* ===================================================================
   install_match_metadata

   This routine installs WMEs that provide metadata about the last
   retrieved epmem.  This data is retrieved from the epmem header.

   Created: 06 June 2006 <--Devil's date!
   =================================================================== */
void install_match_metadata(agent *thisAgent, epmem_header *h)
{
    wme *new_wme;
    
    //Retrieval count
    new_wme = add_input_wme(thisAgent, h->result,
                            make_sym_constant(thisAgent, "retrieval-count"),
                            make_int_constant(thisAgent, h->retrieval_count));
    append_entry_to_arraylist(thisAgent, h->result_wmes, new_wme);
    wme_add_ref(new_wme);
    new_wme->preference = make_fake_preference_for_epmem_wme(thisAgent, h, h->state, new_wme);

    //Match score
    new_wme = add_input_wme(thisAgent, h->result,
                            make_sym_constant(thisAgent, "match-score"),
                            make_float_constant(thisAgent, h->last_match_score));
    append_entry_to_arraylist(thisAgent, h->result_wmes, new_wme);
    wme_add_ref(new_wme);
    new_wme->preference = make_fake_preference_for_epmem_wme(thisAgent, h, h->state, new_wme);

    //Cue size
    new_wme = add_input_wme(thisAgent, h->result,
                            make_sym_constant(thisAgent, "cue-size"),
                            make_int_constant(thisAgent, h->last_cue_size));
    append_entry_to_arraylist(thisAgent, h->result_wmes, new_wme);
    wme_add_ref(new_wme);
    new_wme->preference = make_fake_preference_for_epmem_wme(thisAgent, h, h->state, new_wme);

    //Normalized Match Score
    new_wme = add_input_wme(thisAgent, h->result,
                            make_sym_constant(thisAgent, "normalized-match-score"),
                            make_float_constant(thisAgent, h->last_match_score / (float)h->last_cue_size));
    append_entry_to_arraylist(thisAgent, h->result_wmes, new_wme);
    wme_add_ref(new_wme);
    new_wme->preference = make_fake_preference_for_epmem_wme(thisAgent, h, h->state, new_wme);

    //Match Cardinality
    new_wme = add_input_wme(thisAgent, h->result,
                            make_sym_constant(thisAgent, "match-cardinality"),
                            make_float_constant(thisAgent, (float)h->last_match_size / (float)h->last_cue_size));
    append_entry_to_arraylist(thisAgent, h->result_wmes, new_wme);
    wme_add_ref(new_wme);
    new_wme->preference = make_fake_preference_for_epmem_wme(thisAgent, h, h->state, new_wme);

    //Memory ID (the IDs are sequential so this provides limited temporal info)
    new_wme = add_input_wme(thisAgent, h->result,
                            make_sym_constant(thisAgent, "memory-id"),
                            make_int_constant(thisAgent, h->curr_memory->index));
    append_entry_to_arraylist(thisAgent, h->result_wmes, new_wme);
    wme_add_ref(new_wme);
    new_wme->preference = make_fake_preference_for_epmem_wme(thisAgent, h, h->state, new_wme);

    //Provide the ID of the next memory that will be created.  This
    //is roughly equivalent to the present (as opposed to the the past).
    new_wme = add_input_wme(thisAgent, h->result,
                            make_sym_constant(thisAgent, "present-id"),
                            make_int_constant(thisAgent, thisAgent->epmem_memories->size));
    append_entry_to_arraylist(thisAgent, h->result_wmes, new_wme);
    wme_add_ref(new_wme);
    new_wme->preference = make_fake_preference_for_epmem_wme(thisAgent, h, h->state, new_wme);

}//install_match_metadata

/* ===================================================================
   remove_result_wmes

   This routine removes all the WMEs with a given query result that are
   NOT part of an episodic memory.  It's used as a helper function
   for epmem_clear_curr_result

   Created: 06 June 2006 <--Devil's date!
   =================================================================== */
void remove_result_wmes(agent *thisAgent, epmem_header *h)
{
    wme *w;
    int i;

    i = h->result_wmes->size - 1;  // last entry in the list
    while(i >= 0)
    {
        //Remove the WME from WM
        w = (wme *)get_arraylist_entry(thisAgent, h->result_wmes, i);
        remove_fake_preference_for_epmem_wme(thisAgent, w);
        remove_input_wme(thisAgent, w);
        wme_remove_ref(thisAgent, w);

        //Remove the pointer from the metadata list
        remove_entry_from_arraylist(h->result_wmes, i);

        //Update the index
        i = h->result_wmes->size - 1;
    }//for
    
}//remove_result_wmes


/* ===================================================================
   remove_curr_epmem

   This routine removes all the WMEs in the current result that are
   associated with the currently retrieved episodic memory.  It's used
   as a helper function for epmem_clear_curr_result

   Created: 16 Feb 2004
   Overhauled: 26 Aug 2004
   =================================================================== */
void remove_curr_epmem(agent *thisAgent, epmem_header *h)
{
    int i;
    wme *w;

    //Check for "no-retrieval" wme
   if (get_arraylist_entry(thisAgent, thisAgent->epmem_wmetree->assoc_wmes,h->index) != NULL)
    {
        w = (wme *)get_arraylist_entry(thisAgent, thisAgent->epmem_wmetree->assoc_wmes,h->index);

        remove_fake_preference_for_epmem_wme(thisAgent, w);
        remove_input_wme(thisAgent, w);
        wme_remove_ref(thisAgent, w);
        set_arraylist_entry(thisAgent, thisAgent->epmem_wmetree->assoc_wmes, h->index, NULL);

        return;
    }

    //Check for degenerate cases
    if ( (h->curr_memory == NULL) || (h->curr_memory->content == NULL) )
    {
        return;
    }

    //Remove the WMEs (Traverse the array in reverse order so that
    //                 children are removed before their parents.)
    for(i = h->curr_memory->content->size - 1; i >=0 ; i--)
    {
        wmetree *node = ((actwme *)get_arraylist_entry(thisAgent, h->curr_memory->content,i))->node;

        if (get_arraylist_entry(thisAgent, node->assoc_wmes,h->index) == NULL)
        {
            //This is a memory leak caused by multi-valued attributes.
            //I'm going to punt on it for now. -:AMN:
            continue;
        }

        //Remove from WM
        w = (wme *)get_arraylist_entry(thisAgent, node->assoc_wmes,h->index);
        remove_fake_preference_for_epmem_wme(thisAgent, w);
        remove_input_wme(thisAgent, w);
        wme_remove_ref(thisAgent, w);

        //Bookkeeping
        set_arraylist_entry(thisAgent, node->assoc_wmes, h->index, NULL);
    
    }//for

    
}//remove_curr_epmem

/* ===================================================================
   epmem_clear_curr_result

   This routine removes all the epmem WMEs from working memory that
   are associated with the current result (h->result).  

   Created: 14 Mar 2007
   =================================================================== */
void epmem_clear_curr_result(agent *thisAgent, epmem_header *h)
{
    remove_curr_epmem(thisAgent, h);
    remove_result_wmes(thisAgent, h);
    
}//epmem_clear_curr_result

/* ===================================================================
   meets_relation

   Given a two wmetree nodes, this function determines whether the
   first meets the relation (e.g., "greater-than") specified by the
   second.

   Created:  18 Jul 2006
   =================================================================== */
int meets_relation(wmetree *candidate, wmetree *archetype)
{
    //Do the easy checks first
    if ( (candidate == NULL) || (archetype == NULL) ) return FALSE;
    if (candidate->val_type != archetype->val_type) return FALSE;
    if (strcmp(candidate->attr, archetype->attr) != 0) return FALSE;

    //Now see if the candidate's value meets the required relation
    switch(candidate->val_type)
    {
        case SYM_CONSTANT_SYMBOL_TYPE:
            switch(archetype->relation)
            {
                case GREATER_THAN:
                    if (strcmp(candidate->val.strval, archetype->val.strval) > 0)
                    {
                        return TRUE;
                    }
                    return FALSE;
                case LESS_THAN:
                    if (strcmp(candidate->val.strval, archetype->val.strval) < 0)
                    {
                        return TRUE;
                    }
                    return FALSE;
                default:
                    return TRUE; // optimism!
            }//switch
            
        case INT_CONSTANT_SYMBOL_TYPE:
            switch(archetype->relation)
            {
                case GREATER_THAN:
                    return candidate->val.intval > archetype->val.intval;
                case LESS_THAN:
                    return candidate->val.intval < archetype->val.intval;
                default:
                    return TRUE; // optimism!
            }//switch

        case FLOAT_CONSTANT_SYMBOL_TYPE:
            switch(archetype->relation)
            {
                case GREATER_THAN:
                    return candidate->val.floatval > archetype->val.floatval;
                case LESS_THAN:
                    return candidate->val.floatval < archetype->val.floatval;
                default:
                    return TRUE; // optimism!
            }//switch

        default:
            return TRUE;        // optimism!
    }//switch                

    return TRUE;                // optimism!
}//meets_relation

/* ===================================================================
   record_cue_element_match

   This function finds all the epmems associated with a given node
   from a cue and updates their match scores to reflect that match.
   In addition, it notes the epmems it found that have the best
   match score and best match cardinality so far.  The function
   returns the number of comparisons it performed.

            thisAgent - duh
                    h - epmem header for the associated state
                 node - the node to find matching memories for
               negcue - (boolean) specifies whether this is a negative
                        cue match.  Otherwise it's positive. 
   negcue_cardinality - specifies the size of the negative cue.  
   poscue_cardinality - specifies the size of the positive cue.  
   best_mem_via_score - the best epmem found so far via match score
                        (a NULL value indicates this does not need to
                         be calculated)
   best_mem_via_card  - the best epmem found so far via cardinality
                        (a NULL value indicates this does not need to
                         be calculated)

   SEE ALSO: This is a fundamental helper routine for find_best_match() 
                         
   CAVEAT: This function assumes that the negative cue is processed first. 
                         
   CAVEAT: The negcue_cardinality is added to the num_matches only for
            epmems that matched none of the entries in the negative
            cue.  This means that epmems that matched some of the
            entries in the negative cue are treated as if they had
            matched all of them.  (Match is *not* treated in this way,
            just the cardinality calculation.)  This is fine as long
            only an exact match overrides the best match score but
            will have to be changed if lesser cardinality matches need
            to be taken into consideration.

   Created:  18 Jul 2006
   Updated:  12 Oct 2006 - support bias between match cardinality
                           vs. match activation in match score
   =================================================================== */
int record_cue_element_match(agent *thisAgent,
                             epmem_header *h,
                             wmetree *node,
                             int negcue,
                             int negcue_cardinality,
                             int poscue_cardinality,
                             episodic_memory **best_mem_via_score,
                             episodic_memory **best_mem_via_cardinality)
{
    int i,j;
    int comp_count = 0;
    
    // card_adjust specifies how much to adjust the match score due to a
    // cardinality match
    float card_adjust = ((float)g_card_act_ratio / (float)(negcue_cardinality + poscue_cardinality));

    // act_adjust specifies what to multiply the activation by to get
    // the correct activation adjustment  for the match score
    float act_adjust = 1.0;     // default
    if (g_card_act_ratio != 0.0)
    {
        act_adjust = (float)((1.0 / g_card_act_ratio) / MAX_DECAY);
    }
    
    //Loop over the associated epmems
    for(i = 1; i < node->assoc_memories->size; i++)
    {
        //get the next associated epmem
        episodic_memory *epmem =
            (episodic_memory *)get_arraylist_entry(thisAgent,
                                                   node->assoc_memories,
                                                   i);
    
        //If the agent as asked for a memory that occurs before or after a
        //particular point, handle that here
        if (h->temporal_cmd->name != NULL)
        {
            if ((strcmp(h->temporal_cmd->name, "before") == 0)
                && (h->temporal_cmd->arg_type == INT_CONSTANT_SYMBOL_TYPE)
                && (epmem->index >= h->temporal_cmd->arg.intval))
            {
                continue;
            }
            else if ((strcmp(h->temporal_cmd->name, "after") == 0)
                     && (h->temporal_cmd->arg_type == INT_CONSTANT_SYMBOL_TYPE)
                     && (epmem->index <= h->temporal_cmd->arg.intval))
            {
                continue;
            }
                
        }//if

        //If the agent is probiting any memories, handle that here
        if (h->prohibit_list->size > 0)
        {
            int bProhibited = FALSE;
            
            for(j = 0; j < h->prohibit_list->size; j++)
            {
                long nProbID = (long)get_arraylist_entry(thisAgent,
                                                         h->prohibit_list,
                                                         j);
                if (nProbID == epmem->index)
                {
                    bProhibited = TRUE;
                    break;
                }
            }
            if (bProhibited) continue;
        }//if
            
        //Record that there was a match
        if (epmem->last_usage != thisAgent->epmem_last_ret_id)
        {
            //This is the first match for this cue, so reinit the
            //match data from last time
            epmem->last_usage = thisAgent->epmem_last_ret_id;
            comp_count++;
            epmem->match_score = 0.0;
            epmem->act_total = 0.0;
            if (negcue)  //**See CAVEAT above
            {
                epmem->num_matches = 0;
            }
            else
            {
                epmem->num_matches = 1 + negcue_cardinality;
            }
        }
        else
        {
            (epmem->num_matches)++;
        }

            
        //Find the specific entry in that epmem that matches the cue entry
        actwme *aw_mem = epmem_find_actwme_entry(thisAgent, epmem->content, node);

        if (aw_mem != NULL)
        {
//              //%%%DEBUGGING
//              print(thisAgent, "%d, ", epmem->index);
                
            //Adjust the match score
            if (negcue)
            {
                epmem->match_score -= (float)aw_mem->activation * act_adjust;
                epmem->match_score -=  card_adjust;
                epmem->act_total -= (float)aw_mem->activation;
            }
            else
            {
                epmem->match_score += (float)aw_mem->activation * act_adjust;
                epmem->match_score +=  card_adjust;
                epmem->act_total += (float)aw_mem->activation;
            }
        }//if

        //Check to see if this mem has the best match score so far
        if (best_mem_via_score != NULL)
        {
            if ( (*best_mem_via_score == NULL)
                 || (epmem->match_score > (*best_mem_via_score)->match_score) )
            {
                *best_mem_via_score = epmem;
            }//if
        }//if
            
        //Check to see if this mem has the best match cardinality so far.
        //(Break ties with the match score)
        if (best_mem_via_cardinality != NULL)
        {
            if ( (*best_mem_via_cardinality == NULL)
                 || (epmem->num_matches > (*best_mem_via_cardinality)->num_matches) )
            {
                *best_mem_via_cardinality = epmem;
            }
            else if ( (epmem->num_matches == (*best_mem_via_cardinality)->num_matches)
                      && (epmem->match_score > (*best_mem_via_cardinality)->match_score) )
            {
                *best_mem_via_cardinality = epmem;
            }
        }//if
    }//for

    return comp_count;
}//record_cue_element_match

/* ===================================================================
   find_best_match

   Finds the index of the episodic memory in thisAgent->epmem_memories that most closely
   matches the cue given to the function.  Meta information about the
   match is placed in the given epmem_header.

   Created:  19 Jan 2004
   Overhauled:  18 Jul 2006 - added new capabilities and moved some
                              guts to a sub function: record_cue_element_match
   =================================================================== */
episodic_memory *find_best_match(agent *thisAgent, epmem_header *h,
                                 arraylist *cue, arraylist *negcue)
{
    episodic_memory *best_mem_via_score = NULL;
    int cue_cardinality = 0;
    int negcue_cardinality = 0;
    int total_cardinality = 0;
    episodic_memory *best_mem_via_cardinality = NULL;
    episodic_memory *selected_mem = NULL;
    int i;
    int comp_count = 0;         // number of epmems that were examined

    start_timer(thisAgent, &(thisAgent->epmem_match_start_time));

    //If there aren't enough memories to examine just return
    //the first one
    if (thisAgent->epmem_memories->size <= memory_match_wait)
    {
        return 0;
    }

    //Give this match a unique id
    thisAgent->epmem_last_ret_id++;

    /*
     * Step 1:  Determine cue cardinality
     *
     * Loop through both cues and count the leaf WMEs in each
     *
     */
    for(i = 0; i < negcue->size; i++)
    {
        //pull an entry out of the negative cue
        actwme *aw_cue = (actwme *)get_arraylist_entry(thisAgent, negcue, i);

        //If the entry is a leaf node then add it to the negative 
        //cue cardinality used for detecting an exact match
        if (aw_cue->node->children->count == 0)
        {
            negcue_cardinality++;
        }
    }//for

    for(i = 0; i < cue->size; i++)
    {
        //pull an entry out of the cue
        actwme *aw_cue = (actwme *)get_arraylist_entry(thisAgent, cue,i);

        //If the entry is a leaf node then add it to the cue cardinality
        //used for detecting an exact match
        if (aw_cue->node->children->count == 0)
        {
            cue_cardinality++;
        }
    }//for
    
    total_cardinality = cue_cardinality + negcue_cardinality;

    /*
     * Step 2:  Process the negative cue.
     *
     * Each memory that matches an entry in the negative cue has its
     * match score decreased by that WME's activation.  In addition,
     * the size of the negative cue (i.e., number of leaf WMEs) is
     * counted.
     *
     */
    
    //Every memory that contains an entry specified in the negative cue
    //receives a match score penalty
    for(i = 0; i < negcue->size; i++)
    {
        //pull an entry out of the negative cue
        actwme *aw_cue = (actwme *)get_arraylist_entry(thisAgent, negcue, i);

        comp_count += record_cue_element_match(thisAgent,
                                               h,
                                               aw_cue->node,
                                               TRUE,
                                               negcue_cardinality,
                                               cue_cardinality,
                                               NULL,
                                               NULL);

    }//for

    /*
     * Step 3:  Process the positive cue.
     *
     * Each memory that matches an entry in the positive cue has its
     * match score decreased by that WME's activation.  In addition,
     * the best match so far (both in terms of match score and
     * exact match) is maintained for step 3.
     *
     */

    //Every memory gets a match score boost if it contains a memory
    //that's in the cue.  So we need to loop over the assoc_memories
    //list for each wmetree node in the cue
    for(i = 0; i < cue->size; i++)
    {
        //pull an entry out of the cue
        actwme *aw_cue = (actwme *)get_arraylist_entry(thisAgent, cue,i);

        // if the entry has associated epmems then note that this node
        // was used in the match
        if (aw_cue->node->assoc_memories->size > 0)
        {
            aw_cue->node->query_count++;
        }

        //Check for special case:  a relation specification
        if (aw_cue->node->relation != NONE)
        {
            //Find all the nodes that match the relation
            //CAVEAT:  This function assumes a memory can only have one
            //         wme that matches a relation.  If there's more than
            //         one then the memory's num_matches value will be wrong
            wmetree *parent = aw_cue->node->parent;
            unsigned long hash_value;
            for (hash_value = 0; hash_value < parent->children->size; hash_value++)
            {
                wmetree *child = (wmetree *) (*(parent->children->buckets + hash_value));

                if (meets_relation(child, aw_cue->node))
                {
                    comp_count +=
                        record_cue_element_match(thisAgent,
                                                 h,
                                                 child,
                                                 FALSE,
                                                 negcue_cardinality,
                                                 cue_cardinality,
                                                 &best_mem_via_score,
                                                 &best_mem_via_cardinality);
                }
            }//for
        }//if
        else
        {
            comp_count +=
                record_cue_element_match(thisAgent,
                                         h,
                                         aw_cue->node,
                                         FALSE,
                                         negcue_cardinality,
                                         cue_cardinality,
                                         &best_mem_via_score,
                                         &best_mem_via_cardinality);
        }
    }//for

    /*
     * Step 4:  Select the final match
     *
     * At this point, best_mem_via_score is a pointer to the epmem
     * with the highest match score.  best_mem_via_cardinality is
     * the closest to an exact match.  Select the one to return and
     * do reporting and cleanup.
     *
     */

    //The selected memory is the exact match.  If there is no exact match, then
    //the epmem with the best match score is returned.
    if ( (best_mem_via_cardinality != NULL)
         && (best_mem_via_cardinality->num_matches == total_cardinality) )
    {
        selected_mem = best_mem_via_cardinality;
    }
    else
    {
        selected_mem = best_mem_via_score;
    }

    stop_timer(thisAgent, &(thisAgent->epmem_match_start_time), &(thisAgent->epmem_match_total_time));

    //Record the statistics for this match
    if (selected_mem != NULL)
    {
        h->last_cue_size = total_cardinality;
        h->last_match_size = selected_mem->num_matches;
        h->last_match_score = selected_mem->match_score;
    }

    return selected_mem;
}//find_best_match

/* ===================================================================
   install_epmem_in_wm

   Given an episodic memory this function recreates the working memory
   fragment represented by the memory in working memory.  The
   retrieved memory is placed in the given ^epmem header.

   Created:    19 Jan 2004
   Overhauled: 26 Aug 2004 
   =================================================================== */
void install_epmem_in_wm(agent *thisAgent, epmem_header *h, arraylist *epmem)
{
    int i;
    Symbol *id;
    Symbol *attr;
    Symbol *val;
    wme *ret_wme;
    wme *new_wme;

    //Create a new retrieved WME
    //(Given: <s> ^epmem.result <res>; Create: <res> ^retrieved <ret>)
    ret_wme = add_input_wme(thisAgent, h->result,
                            make_sym_constant(thisAgent, "retrieved"),
                            make_new_identifier(thisAgent,
                                                'E',
                                                h->result->id.level));
    append_entry_to_arraylist(thisAgent, h->result_wmes, ret_wme);
    wme_add_ref(ret_wme);
    ret_wme->preference = make_fake_preference_for_epmem_wme(thisAgent, h, h->state, ret_wme);

    
    //Install the WMEs
    for(i = 0; i < epmem->size; i++)
    {
        wmetree *node = ((actwme *)get_arraylist_entry(thisAgent, epmem,i))->node;

        //For now, avoid recursing into previous memories.
        if (strcmp(node->attr, "epmem") == 0)
        {
            continue;
        }

        if (get_arraylist_entry(thisAgent, node->assoc_wmes,h->index) != NULL)
        {
            //%%%TODO: This happens when a memory contains the same
            //         node multiple times (e.g., a multi-valued attribute)
            //         Currently, I'm just ignoring it.  That is bad.
            continue;
        }

        //Determine the WME's ID
        if (node->parent->depth == 0)
        {
            id = ret_wme->value;
        }
        else
        {
            //If the parent is not in memory then the child can not be either
            wme *parent_wme = (wme *)get_arraylist_entry(thisAgent, node->parent->assoc_wmes,h->index);
            if (parent_wme == NULL)
            {
                continue;
            }

            //The value of the parent WME is the id for this WME
            id = parent_wme->value;
        }

        //Determine the WME's attribute
        attr = make_sym_constant(thisAgent, node->attr);

        //Determine the WME's value
        switch(node->val_type)
        {
            case SYM_CONSTANT_SYMBOL_TYPE:
                val = make_sym_constant(thisAgent, node->val.strval);
                break;
            case INT_CONSTANT_SYMBOL_TYPE:
                val = make_int_constant(thisAgent, node->val.intval);
                break;
            case FLOAT_CONSTANT_SYMBOL_TYPE:
                val = make_float_constant(thisAgent, node->val.floatval);
                break;
            default:
                val = make_new_identifier(thisAgent, node->attr[0],
                                          id->id.level);
                break;
        }//switch

        new_wme = add_input_wme(thisAgent, id, attr, val);
        set_arraylist_entry(thisAgent, node->assoc_wmes,h->index, new_wme);
        wme_add_ref(new_wme);
        new_wme->preference = make_fake_preference_for_epmem_wme(thisAgent, h, h->state, new_wme);
        
    }//for

}//install_epmem_in_wm

/* ===================================================================
   respond_to_query()

   This routine examines an episodic memory cue attached to a given
   symbol.  The result is attached to another given symbol.  In
   addition, any existing WMEs in the retrieval buffer are removed.

   The routine returns a pointer to the arraylist representing the
   memory which was placed on the the retrieved link (or NULL).

   query - the query
   retrieved - where to install the retrieved result

   Created: 19 Jan 2004
   =================================================================== */
arraylist *respond_to_query(agent *thisAgent, epmem_header *h)
{
    arraylist *al_query;
    arraylist *al_negquery;
    arraylist *al_retrieved;
    tc_number tc;
    wme *new_wme;

    //*TODO: A check for the cue having not changed would improve performance.
    
    
    //Remove the old retrieved memory
    start_timer(thisAgent, &(thisAgent->epmem_clearmem_start_time));
    epmem_clear_curr_result(thisAgent, h);
    stop_timer(thisAgent, &(thisAgent->epmem_clearmem_start_time), &(thisAgent->epmem_clearmem_total_time));
    h->curr_memory = NULL;

    //Create an arraylist representing the current query
    al_query = make_arraylist(thisAgent, 32);
    if (h->query_cmd != NULL)
    {
        tc = h->query_cmd->id.tc_num + 1;
        update_wmetree(thisAgent, thisAgent->epmem_wmetree, h->query_cmd, al_query, tc);
    }
    
    //Create an arraylist representing the current negative query
    al_negquery = make_arraylist(thisAgent, 32);
    if (h->negquery_cmd != NULL)
    {
        tc = h->negquery_cmd->id.tc_num + 1;
        update_wmetree(thisAgent, thisAgent->epmem_wmetree, h->negquery_cmd, al_negquery, tc);
    }
    
    //If both queries are empty then we're done
    if ( (al_query->size == 0) && (al_negquery->size == 0) )
    {
        destroy_arraylist(thisAgent, al_query);
        destroy_arraylist(thisAgent, al_negquery);
        return NULL;
    }

    //Diagnostic Counter
    thisAgent->epmem_num_queries++;

    //Match query to current memories list
    h->curr_memory = find_best_match(thisAgent, h, al_query, al_negquery);

    //Cleanup
    destroy_arraylist(thisAgent, al_query);
    destroy_arraylist(thisAgent, al_negquery);
    
    //Place the best fit on the retrieved link
    if (h->curr_memory != NULL)
    {
        //Install the memory
        al_retrieved = h->curr_memory->content;
        start_timer(thisAgent, &(thisAgent->epmem_installmem_start_time));
        install_epmem_in_wm(thisAgent, h, al_retrieved);
        stop_timer(thisAgent, &(thisAgent->epmem_installmem_start_time), &(thisAgent->epmem_installmem_total_time));
        
        //Provide meta-data on the match
        h->retrieval_count = 1;
        install_match_metadata(thisAgent, h);

        //Update the memory's recency
        h->curr_memory->last_ret = thisAgent->epmem_memories->size;
    }
    else
    {
        al_retrieved = NULL;

        //Notify the user of failed retrieval
        new_wme = add_input_wme(thisAgent, h->result,
                                make_sym_constant(thisAgent, "retrieved"),
                                make_sym_constant(thisAgent, "no-memory"));
        set_arraylist_entry(thisAgent, thisAgent->epmem_wmetree->assoc_wmes, h->index, new_wme);
        wme_add_ref(new_wme);
        new_wme->preference = make_fake_preference_for_epmem_wme(thisAgent, h, h->state, new_wme);
    }

    return al_retrieved;

}//respond_to_query

/* ===================================================================
   epmem_retrieve_commands() 

   This routine examines the ^epmem link for commands.  Commands are
   always of the form (<s> ^epmem.query.command <cmd>) where <cmd> has
   a ^name and an optional ^argument.  The given given command is
   stored in the appropriate place in the epmem_header.  If an error
   occurs then the command's name is set to NULL.

   CAVEAT: The cmd->cmd_wme pointer in the epmem_command struct is a
   direct reference to the WME so the pointer may not be valid on
   subsequent cycles.
   
   Created: 27 Jan 2004
   Changed: 19 Oct 2004 - moved the loop to a general purpose function
   Changed: 13 & 25 July 2006 - overhauled as new commands added
   =================================================================== */
void epmem_retrieve_commands(agent *thisAgent, epmem_header *h)
{
    wme **wmes;
    int len = 0;
    int i;
    tc_number tc;
    wme *cmd_wme = NULL;

    //Clean up any old commands
    h->query_cmd = NULL;
    h->negquery_cmd = NULL;
    h->next_cmd = FALSE;
    cleanup_epmem_command(thisAgent, h->temporal_cmd);
    h->prohibit_list->size = 0;

    //Retrieve all the WMEs on the ^command link
    tc = h->command->id.tc_num + 1;
    wmes = epmem_get_augs_of_id(thisAgent, h->command, tc, &len);
    h->command->id.tc_num = tc - 1;
    if (wmes == NULL) return;

    //Search those WMEs for commands
    for(i = 0; i < len; i++)
    {
        cmd_wme = wmes[i];

        //Parse the command
        char *name = cmd_wme->attr->sc.name;
        Symbol *val = cmd_wme->value;
        switch(name[0])
        {
            case 'a':
                if ((strcmp(name, "after") == 0)
                    && (val->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE) )
                {
                    h->temporal_cmd->name =
                        (char *)allocate_memory(thisAgent,
                                                sizeof(char)*strlen("after") + 1,
                                                MISCELLANEOUS_MEM_USAGE);
                    strcpy(h->temporal_cmd->name, "after");

                    h->temporal_cmd->arg_type = INT_CONSTANT_SYMBOL_TYPE;
                    h->temporal_cmd->arg.intval = val->ic.value;
                }
                break;
            case 'b':
                if ((strcmp(name, "before") == 0)
                    && (val->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE) )
                {
                    h->temporal_cmd->name =
                        (char *)allocate_memory(thisAgent,
                                                sizeof(char)*strlen("before") + 1,
                                                MISCELLANEOUS_MEM_USAGE);
                    strcpy(h->temporal_cmd->name, "before");
                            
                    h->temporal_cmd->arg_type = INT_CONSTANT_SYMBOL_TYPE;
                    h->temporal_cmd->arg.intval = val->ic.value;
                }
                break;
            case 'n':
                if (strcmp(name, "next") == 0)
                {
                    h->next_cmd = TRUE;
                }
                else if (strcmp(name, "neg-query") == 0)
                {
                    h->negquery_cmd = val;
                }
                break;
            case 'p':
                if ((strcmp(name, "prohibit") == 0)
                    && (val->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE) )
                {
                    append_entry_to_arraylist(thisAgent,
                                              h->prohibit_list,
                                              (void *)val->ic.value);
                }
                break;
            case 'q':
                if (strcmp(name, "query") == 0)
                {
                    h->query_cmd = val;
                }
                break;
            default:
                fprintf(stderr, "\nINVALID COMMAND: %s!\n", name);
        }//switch                            
                            
    }//for

    free_memory(thisAgent, wmes, MISCELLANEOUS_MEM_USAGE);

}//epmem_retrieve_commands

/* ===================================================================
   respond_to_command_next() 

   This routine responds to the "next" command from the agent which
   populates ^epmem.retrieved with the next memory in the sequence
   from the one that's currently present (h->curr_memory).

   h - the epmem header where the command was found

   Created: 13 Jul 2006
   =================================================================== */
void respond_to_command_next(agent *thisAgent, epmem_header *h)
{
    int ret_count = h->retrieval_count;
    
    //Remove the old retrieved memory
    start_timer(thisAgent, &(thisAgent->epmem_clearmem_start_time));
    epmem_clear_curr_result(thisAgent, h);
    stop_timer(thisAgent, &(thisAgent->epmem_clearmem_start_time), &(thisAgent->epmem_clearmem_total_time));

    //Check that there is a next memory available
    if ( (h->curr_memory != NULL)
         && (h->curr_memory->index < thisAgent->epmem_memories->size - memory_match_wait) )
    {
        //Update the current memory pointer to point to the next epmem
        h->curr_memory =
            (episodic_memory *)get_arraylist_entry(thisAgent, thisAgent->epmem_memories,
                                                   h->curr_memory->index + 1);
            
        //Install the new memory
        start_timer(thisAgent, &(thisAgent->epmem_installmem_start_time));
        install_epmem_in_wm(thisAgent, h, h->curr_memory->content);
        stop_timer(thisAgent, &(thisAgent->epmem_installmem_start_time), &(thisAgent->epmem_installmem_total_time));

        //Provide meta-data on the match
        h->retrieval_count = ret_count + 1;
        install_match_metadata(thisAgent, h);
        
        //Update the memory's recency
        h->curr_memory->last_ret = thisAgent->epmem_memories->size;
    }
    else
    {
        //Notify the user of failed retrieval
        wme *w = add_input_wme(thisAgent, h->result,
                               make_sym_constant(thisAgent, "retrieved"),
                               make_sym_constant(thisAgent, "no-memory"));
        set_arraylist_entry(thisAgent, thisAgent->epmem_wmetree->assoc_wmes, h->index, w);
        w->preference = make_fake_preference_for_epmem_wme(thisAgent, h, h->state, w);
        wme_add_ref(w);
    }
        
}//respond_to_command_next

/* ===================================================================
   find_superstate()

   Given a symbol for a state, return the symbol for its superstate.

   Created: 05 Oct 2004
   =================================================================== */
Symbol *find_superstate(agent *thisAgent, Symbol *sym)
{
    wme **wmes = NULL;
    tc_number tc = sym->id.tc_num + 1; // Is this going to cause problems?
    int len = 0;
    int i;
    Symbol *ss = NULL;

    start_timer(thisAgent, &(thisAgent->epmem_getaugs_start_time));
    wmes = epmem_get_augs_of_id(thisAgent,  sym, tc, &len );
    stop_timer(thisAgent, &(thisAgent->epmem_getaugs_start_time), &(thisAgent->epmem_getaugs_total_time));

    if (wmes != NULL)
    {
        for(i = 0; i < len; i++)
        {
            //Check for special case: "superstate" 
            if ( (wme_has_value(wmes[i], "superstate", NULL))
                 && (ss == NULL)
                 && (wmes[i]->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE) )
            {
                ss = wmes[i]->value;
                break;
            }
        }

        free_memory(thisAgent, wmes, MISCELLANEOUS_MEM_USAGE);
    }//if

    return ss;
        
}//find_superstate


/* ===================================================================
   epmem_update_header_stack()

   Update the list of epmem_header structs which maintain information
   about what memories have been retrieved for each state in the
   state stack.

   Created: 03 Oct 2004
   =================================================================== */
void epmem_update_header_stack(agent *thisAgent)
{
    Symbol *sym;
    int i;
    epmem_header *h;
    arraylist *new_states = make_arraylist(thisAgent, 20);
    int bFound = FALSE;


    /*
     * Find the lowest state in thisAgent->epmem_header_stack that has an analog
     * in working memory.  As we do the search, record any states
     * that are lower than this in the new_states list.
     */

    //Start at the bottom state and work our way up
    sym = (thisAgent->bottom_goal);
    while(sym != (thisAgent->top_goal))
    {
        //Search for an analog to the state in thisAgent->epmem_header_stack
        for(i = thisAgent->epmem_header_stack->size - 1; i >= 0; i--)
        {
            h = (epmem_header *)get_arraylist_entry(thisAgent, thisAgent->epmem_header_stack,i);
            if (h->state == sym)
            {
                //An analog was found, remove any of its children and break
                int j;
                for(j = thisAgent->epmem_header_stack->size - 1; j > i; j--)
                {
                    destroy_epmem_header(
                        thisAgent, 
                        (epmem_header *)remove_entry_from_arraylist(thisAgent->epmem_header_stack, j));
                }

                bFound = TRUE;
                break;
            }//if
        }//for

        //If an analog was found, exit the loop
        if (bFound) break;
        
        //The current sym is a new state that needs an ^epmem link
        //so save it away for later
        append_entry_to_arraylist(thisAgent, new_states, (void *)sym);

        //Move to the parent state and iterate
        sym = find_superstate(thisAgent, sym);
        
    }//while

    /*
     * The states in new_states need an ^epmem link and a corresponding
     * entry in the thisAgent->epmem_header_stack list.
     */
    for(i = new_states->size - 1; i >= 0; i--)
    {
        h = make_epmem_header(thisAgent, (Symbol *)get_arraylist_entry(thisAgent, new_states,i));
        h->index = thisAgent->epmem_header_stack->size;
        append_entry_to_arraylist(thisAgent, thisAgent->epmem_header_stack, (void *)h);
    }

}//epmem_update_header_stack


/* ===================================================================
   epmem_calc_wmetree_size      *RECURSIVE*

   Calculates the size of a wmetree in terms of numbers of nodes

   SEE ALSO: epmem_print_mem_usage
   
   Created: 15 June 2004
   =================================================================== */
int epmem_calc_wmetree_size(wmetree *node)
{
    int size = 0;
    unsigned long hash_value;
    wmetree *child;
    
    if (node == NULL) return 0;

    size += node->children->size;
    
    for (hash_value = 0; hash_value < node->children->size; hash_value++)
    {
        child = (wmetree *) (*(node->children->buckets + hash_value));
        for (; child != NIL; child = child->next)
        {
            size += epmem_calc_wmetree_size(child);
        }
    }

    return size;
}//epmem_calc_wmetree_size

/* ===================================================================
   epmem_print_detailed_usage_memories

   Prints a memory usage trace for thisAgent->epmem_memories to a
   given file handle.

   SEE ALSO: epmem_print_mem_usage
   
   Created: 15 June 2004
   =================================================================== */
//NOTE:  NUM_BINS must be less than 256 and HISTOGRAM_SIZE must be greater
//       than the maximum size of any memory.
#define BIN_SIZE 3
#define NUM_BINS 200
#define HISTOGRAM_SIZE (BIN_SIZE * NUM_BINS + 1)
void epmem_print_detailed_usage_memories(agent *thisAgent, FILE *f)
{
    int histogram[HISTOGRAM_SIZE];
    int i,j;
    int largest = 0;
    

    for(i = 0; i < HISTOGRAM_SIZE; i++)
    {
        histogram[i] = 0;
    }

    for(i = 0; i < thisAgent->epmem_memories->size; i++)
    {
        int memsize =  ((arraylist *)get_arraylist_entry(thisAgent, thisAgent->epmem_memories,i))->size;
        if (memsize > HISTOGRAM_SIZE)
        {
            fprintf(f, "ERROR!  Memories are too large.\n");
        }
        else
        {
            (histogram[memsize])++;
        }

        if (memsize > largest) largest = memsize;
    }

    //To accomodate maximum excel spreadsheet dimensions , bin the results
    for(i = 0; i < NUM_BINS; i++)
    {
        int sum = 0;
        for(j = 1; j <= BIN_SIZE; j++)
        {
            sum += histogram[i*BIN_SIZE + j];
        }
        fprintf(f, "%d\t", sum);
    }
    fprintf(f, "\n");
    
}//epmem_print_detailed_usage_memories

/* ===================================================================
   epmem_print_detailed_usage_wmetree      *RECURSIVE*

   Prints a memory usage trace for a wmetree to a given file handle.

   f - file handle to print to
   node - the wmetree to print the usage stats about
   largest - initial caller should set an int to zero and pass in a
             ptr to it here

   SEE ALSO: epmem_print_mem_usage
   
   Created: 15 June 2004
   =================================================================== */
void epduw_helper(wmetree *node, int *histogram, int *largest)
{
    int nodesize;
    unsigned long hash_value;
    wmetree *child;

    if (node->assoc_memories != NULL)
    {
        nodesize = node->assoc_memories->size;
        (histogram[nodesize])++;
        if (nodesize > *largest) *largest = nodesize;
    }
    
    for (hash_value = 0; hash_value < node->children->size; hash_value++)
    {
        child = (wmetree *) (*(node->children->buckets + hash_value));
        for (; child != NIL; child = child->next)
        {
            epduw_helper(child, histogram, largest);
        }
    }
    
}//epduw_helper

void epmem_print_detailed_usage_wmetree(FILE *f, wmetree *node)
{
    static int histogram[HISTOGRAM_SIZE];
    int i;
    int largest = 0;
    
    if (node == NULL)
    {
        fprintf(f, "\n");
        return;
    }

    for(i = 0; i < HISTOGRAM_SIZE; i++)
    {
        histogram[i] = 0;
    }

    epduw_helper(node, histogram, &largest);
    
    fprintf(f, "%d", histogram[2]);
    for(i = 2; i <= largest; i++)
    {
        fprintf(f, "\t%d", histogram[i]);
    }
    fprintf(f, "\n");
    
}//epmem_print_detailed_usage_wmetree



/* ===================================================================
   epmem_print_mem_usage

   Prints information about how much RAM the episodic memories are
   using to a file.

   CAVEAT: The output has hardcoded values for the size of a given
            struct.  I can't just use sizeof() here because I have to
            take into account sub-structures that are always there.  A
            proper routine would actually caculate the total size
            using a series of sizeof commands but it was overkill for
            the experiment I was doing and even that's not foolproof
            if new substructs are added. -:AMN:

   Created: 14 June 2004
   =================================================================== */
void epmem_print_mem_usage(agent *thisAgent)
{
    wmetree *parent = thisAgent->epmem_wmetree;
    wmetree *child;
    arraylist *queue = make_arraylist(thisAgent, 32);
    int qpos = 0;                // current position in the queue
    unsigned long hash_value;
    int num_wmetrees = 0;
    int num_am_ptrs = 0;
    int num_strings = 0;
    int total_strlen = 0;
    int num_epmems = thisAgent->epmem_memories->size;
    int num_actwmes = 0;
    int i;
    episodic_memory *epmem;

    while(parent != NULL)
    {
        for (hash_value = 0; hash_value < parent->children->size; hash_value++)
        {
            child = (wmetree *) (*(parent->children->buckets + hash_value));
            for (; child != NIL; child = child->next)
            {
                if (child->val_type == SYM_CONSTANT_SYMBOL_TYPE)
                {
                    num_strings++;
                    total_strlen += strlen(child->val.strval);
                }

                num_am_ptrs += child->assoc_memories->size;
                
                num_wmetrees++;

                //If this WME has children add it to the queue for future
                //processing
                if (child->children->count > 0)
                {
                    append_entry_to_arraylist(thisAgent, queue, child);
                }

            }//for
        }//for

        //Retrieve the next node from the queue
        parent = (wmetree *)get_arraylist_entry(thisAgent, queue, qpos);
        qpos++;
    }//while

    destroy_arraylist(thisAgent, queue);

    for(i = 1; i < thisAgent->epmem_memories->size; i++)
    {
        epmem = (episodic_memory *)get_arraylist_entry(thisAgent, thisAgent->epmem_memories, i);
        if (epmem->content != NULL)
        {
            num_actwmes += epmem->content->size;
        }
    }

    //Print the info
    print(thisAgent, "\n");
    print(thisAgent, "%.6d wmetree structs = %.10d bytes\n", num_wmetrees, num_wmetrees*65);
    print(thisAgent, "%.6d epmem ptrs      = %.10d bytes\n", num_am_ptrs, num_am_ptrs*4);
    print(thisAgent, "%.6d symbol strings  = %.10d bytes\n", num_strings, total_strlen);
    print(thisAgent, "%.6d epmem structs   = %.10d bytes\n", num_epmems, num_epmems*28);
    print(thisAgent, "%.6d actwme structs  = %.10d bytes\n", num_actwmes, num_actwmes*8);
    print(thisAgent, "     TOTAL EPMEM ALLOC = %.10d bytes\n",
          num_wmetrees*65 + num_am_ptrs*4 + total_strlen + num_epmems*28 + num_actwmes*8);
    
}//epmem_print_mem_usage


/* ===================================================================
   epmem_reset_cpu_usage_timers() 

   Reset all the timing info recorded about the CPU usage of
   the epmem subsystem.
   
   Created: 07 Jul 2004
   =================================================================== */
void epmem_reset_cpu_usage_timers(agent *thisAgent)
{
    reset_timer(&(thisAgent->epmem_total_time));
    reset_timer(&(thisAgent->epmem_record_total_time));
    reset_timer(&(thisAgent->epmem_retrieve_total_time));
    reset_timer(&(thisAgent->epmem_clearmem_start_time));
    reset_timer(&(thisAgent->epmem_clearmem_total_time));
    reset_timer(&(thisAgent->epmem_updatewmetree_total_time));
    reset_timer(&(thisAgent->epmem_getaugs_total_time));
    reset_timer(&(thisAgent->epmem_match_total_time));
    reset_timer(&(thisAgent->epmem_findchild_total_time));
    reset_timer(&(thisAgent->epmem_addnode_total_time));
    reset_timer(&(thisAgent->epmem_installmem_total_time));
    reset_timer(&(thisAgent->epmem_misc1_total_time));
    reset_timer(&(thisAgent->epmem_misc2_total_time));
}

/* ===================================================================
   epmem_print_cpu_usage() 

   Print CPU usage info for the epmem subsystem.
   
   Created: 07 Jul 2004
   =================================================================== */
void epmem_print_cpu_usage(agent *thisAgent)
{
    double total = timer_value(&(thisAgent->epmem_total_time));
    double f;
    
    print(thisAgent, "\n**** Epmem CPU Usage Results ****\n\n");
    print(thisAgent, "Routine                             Time      Fraction of Total Time\n");
    print(thisAgent, "----------------------------------  --------  ----------------------\n");

    f = timer_value(&(thisAgent->epmem_record_total_time));
    print(thisAgent, "record_epmem()                       %.3lf     %.3lf\n", f, f / total);
    f = timer_value(&(thisAgent->epmem_updatewmetree_total_time));
    print(thisAgent, "    update_wmetree( )                 %.3lf     %.3lf\n", f, f / total);
    f = timer_value(&(thisAgent->epmem_getaugs_total_time));
    print(thisAgent, "        get_augs_of_id()             %.3lf     %.3lf\n", f, f / total);
    f = timer_value(&(thisAgent->epmem_findchild_total_time));
    print(thisAgent, "        find_child_node()            %.3lf     %.3lf\n", f, f / total);
    f = timer_value(&(thisAgent->epmem_addnode_total_time));
    print(thisAgent, "        add_node_to_memory()         %.3lf     %.3lf\n", f, f / total);
    f = timer_value(&(thisAgent->epmem_retrieve_total_time));
    print(thisAgent, "episodic retrieval                   %.3lf     %.3lf\n", f, f / total);
    f = timer_value(&(thisAgent->epmem_clearmem_total_time));
    print(thisAgent, "    epmem_clear_curr_result()           %.3lf     %.3lf\n", f, f / total);
    print(thisAgent, "    update_wmetree() is also called here (see above)\n");
    f = timer_value(&(thisAgent->epmem_match_total_time));
    print(thisAgent, "    find_best_match()                %.3lf     %.3lf\n", f, f / total);
    f = timer_value(&(thisAgent->epmem_installmem_total_time));
    print(thisAgent, "    install_epmem_in_wm()            %.3lf     %.3lf\n", f, f / total);

    print(thisAgent, "--------------------------------------------------------------------\n");
    print(thisAgent, "TOTAL                                %.3lf     1.0\n", total);

    epmem_reset_cpu_usage_timers(thisAgent);

}//epmem_print_cpu_usage

/* ===================================================================
   epmem_save_wmetree_to_file()             *RECURSIVE*

   This routine saves the contents of a given wmetree to a given file.  It
   returns the total number of nodes that were written.
   
   f - file to write to
   node - root of tree to write
   parent_id - numerical index of the parent
   index - number of nodes written so far

   Created: 23 Mar 2006
   =================================================================== */
#define MAX_LIST_LEN 15
int epmem_save_wmetree_to_file(agent *thisAgent,
                               FILE *f,
                               wmetree *node,
                               int parent_index,
                               int total)
{
    int i;
    unsigned long hash_value;
    wmetree *child;
    int len;

    if (node->attr == NULL)
    {
        fputs("ROOT\n", f);
    }
    else
    {
        fprintf(f, "%i ", node->id);
        
        fputs(node->attr, f);
        fputs(" ", f);
    
        switch(node->val_type)
        {
            case SYM_CONSTANT_SYMBOL_TYPE:
                fprintf(f, "s %s ", node->val.strval);
                break;
            case INT_CONSTANT_SYMBOL_TYPE:
                fprintf(f, "i %i ", node->val.intval);
                break;
            case FLOAT_CONSTANT_SYMBOL_TYPE:
                fprintf(f, "f %f ", node->val.floatval);
                break;
            default:
                fprintf(f, "n <id> ");
                break;
        }//switch

        fprintf(f, "%i ", parent_index);
    
        fprintf(f, "%i ", node->depth);

        len = 0;
        fputs("[ ", f);
        if (node->assoc_memories != NULL)
        {
            for(i = 1; i < node->assoc_memories->size; i++)
            {
                episodic_memory *epmem =
                    (episodic_memory *)get_arraylist_entry(thisAgent, node->assoc_memories,i);
                fprintf(f, "%i ", epmem->index);

                //Keep a single line from getting too long
                len++;
                if (len >= MAX_LIST_LEN)
                {
                    fprintf(f, "@\n");
                    len = 0;
                }
            }//for
        }//if
        fputs("] ", f);

        fprintf(f, "%i %i\n", node->query_count, node->ubiquitous);

    }//else

    
    total ++;
    
    //Recurse into children
    for (hash_value = 0; hash_value < node->children->size; hash_value++)
    {
        child = (wmetree *) (*(node->children->buckets + hash_value));
        for (; child != NIL; child = child->next)
        {
            total = epmem_save_wmetree_to_file(thisAgent,
                                               f,
                                               child,
                                               node->id,
                                               total);
        }
    }

    if (node->attr == NULL)
    {
        fputs("END OF NODE LIST\n", f);
    }
    
    return total;
    
    
}//epmem_save_wmetree_to_file

/* ===================================================================
   epmem_save_epmems_to_file()

   This routine saves all all entries in a given arraylist of
   episodic_memory structs (usually thisAgent->epmem_memories).
   
   f - file to write to
   node - root of tree to write
   parent_id - numerical index of the parent
   index - number of nodes written so far

   Created: 28 Mar 2006
   =================================================================== */
int epmem_save_epmems_to_file(agent *thisAgent,
                               FILE *f,
                               arraylist *memlist)
{
    int i,j;
    int total = 0;

    fputs("BEGIN MEM LIST\n", f);

    for(i = 1; i < memlist->size; i++)
    {
        episodic_memory *epmem =
            (episodic_memory *)get_arraylist_entry(thisAgent, thisAgent->epmem_memories, i);
        
        //Write index
        fprintf(f, "%i ", i);

        //Save the last_ret prefaced with an 'a' character
        fprintf(f, "a%i ", epmem->last_ret);

        //Note:  There's no need to save last_usage and match_score

        //Write beginning of actwme list
        fputs("[ ", f);

       //Print the id and activation of each actwme
        for(j = 0; j < epmem->content->size; j++)
        {
            actwme *aw = (actwme *)get_arraylist_entry(thisAgent,
                                                           epmem->content,
                                                           j);
            fprintf(f, "%i ", aw->node->id);
            fprintf(f, "%i ", aw->activation);
        }//for

        //Write end of actwme list
        fputs("] \n", f);

        total++;
        
    }//for

    fputs("END MEM LIST\n", f);

    return total;
}//epmem_save_epmems_to_file

/* ===================================================================
   epmem_save_episodic_memory_to_file() 

   This routine writes the agent's currently saved episodes to a file
   so that they can be reloaded.

   Created: 23 Mar 2006
   =================================================================== */
void epmem_save_episodic_memory_to_file(agent *thisAgent, char *fn)
{
    FILE *f;

    if ( (fn == NULL) || (strlen(fn) == 0) )
    {
        fn = thisAgent->epmem_autosave_filename;
    }

    f = fopen(fn, "w");
    if (f == NULL)
    {
        print(thisAgent, "\nERROR:  could not save episodes to file: ");
        print(thisAgent, fn);
        return;
    }//if

    epmem_save_wmetree_to_file(thisAgent, f, thisAgent->epmem_wmetree, 0, 0);
    epmem_save_epmems_to_file(thisAgent, f, thisAgent->epmem_memories);

    fclose(f);
    
}//epmem_save_episodic_memory_to_file


/* ===================================================================
   epmem_clear_all_memories

   This routine causes the agent to forget all its episodic memories.

   Created: 31 Mar 2006
   =================================================================== */
void epmem_clear_all_memories(agent *thisAgent)
{
    int i,j;
    episodic_memory *epmem;
    unsigned long hash_value;
    wmetree *child, *next_child;
    wme *w;
    
    /*
     * Step 1: Remove all currently retreived epmems from WM
     */
    for(i = 0; i < thisAgent->epmem_header_stack->size; i++)
    {
        epmem_header *h = (epmem_header *)get_arraylist_entry(thisAgent, thisAgent->epmem_header_stack, i);
        epmem_clear_curr_result(thisAgent, h);
    }
    
    /*
     * Step 2: Remove all episodic memories
     */
    for(i = 0; i < thisAgent->epmem_memories->size; i++)
    {
        epmem = (episodic_memory *)get_arraylist_entry(thisAgent, thisAgent->epmem_memories, i);

        //If memories are unloaded and reloaded the first entry can be NULL
        if ((epmem == NULL) || (epmem->content == NULL)) continue;
        
        //Clean up the actwme list (but not the associated wmetree nodes)
        for(j = 0; j < epmem->content->size; j++)
        {
            actwme *aw = (actwme *)get_arraylist_entry(thisAgent,
                                                       epmem->content,
                                                       j);
            free_memory(thisAgent, aw, MISCELLANEOUS_MEM_USAGE);
        }//for

        destroy_arraylist(thisAgent, epmem->content);
        free_memory(thisAgent, epmem, MISCELLANEOUS_MEM_USAGE);
        set_arraylist_entry(thisAgent, thisAgent->epmem_memories, i, NULL);
    }//for
    thisAgent->epmem_memories->size = 0;
    
    /*
     * Step 3: Remove all nodes from the wmetree
     */
    for (hash_value = 0; hash_value < thisAgent->epmem_wmetree->children->size; hash_value++)
    {
        child = (wmetree *) (*(thisAgent->epmem_wmetree->children->buckets + hash_value));
        while (child != NULL)
        {
            next_child = child->next;
            remove_from_hash_table(thisAgent, thisAgent->epmem_wmetree->children, child);
            destroy_wmetree(thisAgent, child);
            child = next_child;
        }//while
    }//for

    //Sanity check for lost retrieved memories
    for(i = 0; i < thisAgent->epmem_wmetree->assoc_wmes->size; i++)
    {
        w = (wme *)get_arraylist_entry(thisAgent, thisAgent->epmem_wmetree->assoc_wmes, i);
        if (w != NULL)
        {
            print(thisAgent, "ERROR!:  ROOT  wmetree node still has associated WMEs.");
        }
    }//for

    //Remove and recreate the children (probably overkill)
    free_memory(thisAgent, thisAgent->epmem_wmetree->children->buckets, HASH_TABLE_MEM_USAGE);
    free_memory (thisAgent, thisAgent->epmem_wmetree->children, HASH_TABLE_MEM_USAGE);
    thisAgent->epmem_wmetree->children = make_hash_table(thisAgent, 0, hash_wmetree);;


    //Remove and recreate the assoc_wmes list (probably overkill)
    destroy_arraylist(thisAgent, thisAgent->epmem_wmetree->assoc_wmes);
    thisAgent->epmem_wmetree->assoc_wmes = make_arraylist(thisAgent, 20);
    thisAgent->epmem_wmetree_size = 0;
    
    
    
}//epmem_clear_all_memories


/* ===================================================================
   epmem_load_wmetree_from_file() 

   This routine loads the contents of a given wmetree from a given file.
   It returns an arraylist of all the nodes that were loaded.
   f - file to load from
   node - root of tree to add the nodes too

   NOTE: This function assumes the given wmetree node has already been
         allocated and initialized (including the assoc_wmes,
         assoc_memories and children lists).

   CAVEAT:  The caller is responsible for deallocating the returned
            arraylist.

   Created: 23 Mar 2006
   =================================================================== */
//#define EPMEM_BUFLEN 262143 (this is too big!  why?)
#define EPMEM_BUFLEN 210000
arraylist *epmem_load_wmetree_from_file(agent *thisAgent,
                                        FILE *f,
                                        wmetree *root_node,
                                        int parent_index)
{
    char *buf;
    char *str = NULL;
    char *str2 = NULL;
    int mem_id;
    int parent_id;
    arraylist *nodelist;
    wmetree *parent_node;
    wmetree *node;

    buf = (char *)malloc(sizeof(char)*EPMEM_BUFLEN);

    //Check for the appropriate file format for the first node
    fgets(buf, EPMEM_BUFLEN, f);
    if (strcmp(buf, "ROOT\n") != 0)
    {
        print(thisAgent, "ERROR: File improperly formatted.  \"ROOT\" node not found. Episodes not loaded.");
        free(buf);
        return NULL;
    }

    //Allocate and init  nodelist
    nodelist = make_arraylist(thisAgent, 100);

    //Set the first entry to the root node
    set_arraylist_entry(thisAgent, nodelist, 0, thisAgent->epmem_wmetree);
    
    //Init node pointer to the root
    node = root_node;

    //Each iteraton of this loop reads one node from the file
    while (!feof(f))
    {
        //Read the next node from the file
        fgets(buf, EPMEM_BUFLEN, f);
        if (strlen(buf) == 0) break;
        if (strcmp(buf, "END OF NODE LIST\n") == 0) break;

        //Allocate and initialize a wmetree node for the next entry
        node = make_wmetree_node(thisAgent, NULL);
        append_entry_to_arraylist(thisAgent, node->assoc_memories, NULL); // dummy entry

        //Read node->id and insert myself into the nodelist
        str = strtok(buf, " ");
        node->id = atoi(str);
        set_arraylist_entry(thisAgent, nodelist, node->id, node);

        //Read node->attr
        str = strtok(NULL, " ");
        node->attr = (char *)allocate_memory(thisAgent,
                                             strlen(str)+1,
                                             MISCELLANEOUS_MEM_USAGE);
        strcpy(node->attr, str);

        //Read node->val_type
        str = strtok(NULL, " ");
        switch(str[0])
        {
            case 's':
                node->val_type = SYM_CONSTANT_SYMBOL_TYPE;
                str = strtok(NULL, " ");
                node->val.strval = (char *)allocate_memory(thisAgent,
                                                           strlen(str)+1,
                                                           MISCELLANEOUS_MEM_USAGE);
                strcpy(node->val.strval, str);
                break;
            case 'i':
                node->val_type = INT_CONSTANT_SYMBOL_TYPE;
                str = strtok(NULL, " ");
                node->val.intval = atoi(str);
                break;
            case 'f':
                node->val_type = FLOAT_CONSTANT_SYMBOL_TYPE;
                str = strtok(NULL, " ");
                node->val.floatval = (float)atof(str);
                break;
            case 'n':
            default: 
                node->val_type = IDENTIFIER_SYMBOL_TYPE;
                str = strtok(NULL, " ");
                if (strcmp(str, "<id>") != 0)
                {
                    print(thisAgent, "ERROR: File improperly formatted.  Expected \"<id>\" but found %s. Episodes not loaded.", str);
                    //%%%I should clean up already loaded data here
                    destroy_arraylist(thisAgent, nodelist);
                    free(buf);
                    return NULL;
                }
                break;
        }

        //Read parent index add this node to the parent
        str = strtok(NULL, " ");
        parent_id = atoi(str);
        parent_node = (wmetree *)get_arraylist_entry(thisAgent,
                                                      nodelist,
                                                      parent_id);
        if (parent_node == NULL)
        {
            print(thisAgent, "ERROR: File improperly formatted.  Child node references nonexistant parent. Episode load aborted.", str);
            //%%%I should clean up already loaded data here
            destroy_arraylist(thisAgent, nodelist);
            free(buf);
            return NULL;
        }
        add_to_hash_table(thisAgent, parent_node->children, node);
        node->parent = parent_node;
        thisAgent->epmem_wmetree_size++;

        //Read depth
        str = strtok(NULL, " ");
        node->depth = atoi(str);

        //Sanity check
        str = strtok(NULL, " ");
        if (str[0] != '[')
        {
            print(thisAgent, "ERROR: File improperly formatted.  Expected '[' but found %s. Episode load aborted.", str);
            //%%%I should clean up already loaded data here
            destroy_arraylist(thisAgent, nodelist);
            free(buf);
            return NULL;
        }

        //Read the associated memory ids
        str = strtok(NULL, " ");
        mem_id = atoi(str);
        while(str[0] != ']')
        {
            episodic_memory *epmem =
                (episodic_memory *)get_arraylist_entry(thisAgent,
                                                       thisAgent->epmem_memories,
                                                       mem_id);
            if (epmem == NULL)
            {
                //Allocate and init a structure for this to-be-read epmem
                epmem = (episodic_memory *)allocate_memory(thisAgent,
                                                           sizeof(episodic_memory),
                                                           MISCELLANEOUS_MEM_USAGE);
                epmem->content = NULL;
                epmem->last_usage = -1;
                epmem->match_score = 0.0;
                epmem->act_total = 0.0;
                epmem->index = mem_id;

                //Add the structure to the global memories list
                set_arraylist_entry(thisAgent, thisAgent->epmem_memories, mem_id, (void *)epmem);
            }//if

            //Add the structure to the assoc_memories list
            append_entry_to_arraylist(thisAgent, node->assoc_memories, (void *)epmem);

            //Get the next id
            str = strtok(NULL, " ");
            if (str == NULL)
            {
                print(thisAgent, "ERROR: File improperly formatted.  Expected a node id but reached end-of-string.  Could this be a buffer overrun? Episode load aborted.", str);
                //%%%I should clean up already loaded data here
                destroy_arraylist(thisAgent, nodelist);
                free(buf);
                return NULL;
            }

            //Handle end of line token
            if (str[0] == '@')
            {
                fgets(buf, EPMEM_BUFLEN, f);
                if ( (strlen(buf) == 0)
                     || ( ((buf[0] < '0') || (buf[0] > '9'))
                          && (buf[0] != ']') ) )
                {
                    print(thisAgent, "ERROR: File improperly formatted.  Expected a node id.  Episode load aborted.");
                    //%%%I should clean up already loaded data here
                    destroy_arraylist(thisAgent, nodelist);
                    free(buf);
                    return NULL;
                }
                
                str = strtok(buf, " ");
            }//if

            //Get the next id
            mem_id = atoi(str);
        }//while
        
        //Read node->query_count
        str = strtok(NULL, " ");
        node->query_count = atoi(str);

        //Read node->ubiquitous
        str = strtok(NULL, " ");
        node->ubiquitous = atoi(str);


    }//while

    free(buf);
    return nodelist;
    
}//epmem_load_wmetree_from_file

/* ===================================================================
   epmem_load_epmems_from_file()

   This routine loads a list of episodic_memory structs (and their
   associated actwme lists) from a file.

   NOTE:  The thisAgent->epmem_memories list is assumed to already contain init'd
          entries for all of these epmems (which is a side effect of
          epmem_load_wmetree_from_file() above).
   
   f - file to write to
   nodelist - an arraylist list wmetree* that the loaded memories
              will index into (created by epmem_load_wmetree_from_file())

   Created: 28 Mar 2006
   =================================================================== */
int epmem_load_epmems_from_file(agent *thisAgent,
                               FILE *f,
                               arraylist *nodelist)
{
    char buf[EPMEM_BUFLEN];
    char *str = NULL;
    int my_index;
    episodic_memory *epmem;
    int total = 0;

    //Check for the appropriate file format for the first node
    fgets(buf, EPMEM_BUFLEN, f);
    if (strcmp(buf, "BEGIN MEM LIST\n") != 0)
    {
        print(thisAgent, "ERROR: File improperly formatted.  \"BEGIN MEM LIST\" not found. Memories not loaded.");
        return 0;
    }

    //Each iteraton of this loop reads one node from the file
    while (!feof(f))
    {
        //Read the next node from the file
        fgets(buf, EPMEM_BUFLEN, f);
        if (strlen(buf) == 0) break;
        if (strcmp(buf, "END MEM LIST\n") == 0) break;

        //Read the index and find the appropriate epmem
        str = strtok(buf, " ");
        my_index = atoi(str);
        epmem = (episodic_memory *)get_arraylist_entry(thisAgent,
                                                       thisAgent->epmem_memories,
                                                       my_index);
        if (epmem == NULL)
        {
            //I've encountered an epmem that's never referenced by any
            //node in the WME tree.  This should not occur.
            //Handle it by going ahead and loading the epmem but
            //give a warning to the user
            epmem = (episodic_memory *)allocate_memory(thisAgent,
                                                       sizeof(episodic_memory),
                                                       MISCELLANEOUS_MEM_USAGE);
            epmem->content = NULL;
            epmem->last_usage = -1;
            epmem->match_score = 0.0;
            epmem->act_total = 0.0;
            epmem->index = my_index;
            
            //Add the structure to the global memories list
            set_arraylist_entry(thisAgent, thisAgent->epmem_memories, my_index, (void *)epmem);

            //Notify the user
            print(thisAgent, "WARNING: Encountered unreferenced epmem (#%d) in load file.", my_index);
        }
        
        if (epmem->content != NULL)
        {
            print(thisAgent, "ERROR: File improperly formatted.  Duplicate index found.  Memory skipped.");
            continue;
        }

        //*Possibly* read epmem->last_ret.  In some older epmem files the
        //last_ret value is missing so I have to detect and handle that case.
        //If the value is present it will be prefaced with an 'a' character
        str = strtok(NULL, " ");
        switch(str[0])
        {
            case '[':
                epmem->last_ret = thisAgent->epmem_memories->size;
                break;
            case 'a':
                str = str + 1;
                epmem->last_ret = atoi(str);
                str = strtok(NULL, " ");
                if (str[0] == '[') break; // sanity check
                //otherwise fall through to unexpected token error
            default:
                print(thisAgent, "ERROR: File improperly formatted.  Expected '[' but found %s. Epmems load aborted.", str);
                //%%%I should clean up already loaded data here
                return 0;
        }//switch

        //Allocate the content list
        epmem->content = make_arraylist(thisAgent, 20);

        //Load the actwmes
        str = strtok(NULL, " ");
        while(str[0] != ']')
        {
            int node_id;
            int activation;
            wmetree *node;

            //Read and find the wmetree node
            node_id = atoi(str);
            if (node_id >= nodelist->size)
            {
                print(thisAgent, "ERROR: File improperly formatted.  Illegal node index loaded. Epmems load aborted.", str);
                //%%%I should clean up already loaded data here
                return 0;
            }
            node = (wmetree *)get_arraylist_entry(thisAgent, nodelist, node_id);

            //Read the activation
            str = strtok(NULL, " ");
            activation = atoi(str);

            //Create and add the actwme
            add_node_to_memory(thisAgent, epmem->content, node, activation);

            //Init for next iteration
            str = strtok(NULL, " ");

        }//while

        total++;
    }//while

    return total;
}//epmem_load_epmems_from_file


/* ===================================================================
   epmem_load_episodic_memory_from_file() 

   This routine loads memories from a file.

   CAVEAT:  All existing memories (if any) are deleted.

   Created: 23 Mar 2006
   =================================================================== */
void epmem_load_episodic_memory_from_file(agent *thisAgent, char *fn)
{
    FILE *f;
    arraylist *nodelist;

    if ( (fn == NULL) || (strlen(fn) == 0) )
    {
        fn = thisAgent->epmem_autosave_filename;
    }

    //Check for a good file
    f = fopen(fn, "r");
    if (f == NULL)
    {
        print(thisAgent, "\nERROR:  could not load episodes from file: ");
        print(thisAgent, fn);
        return;
    }//if

    //Remove all memories that are currently stored
    epmem_clear_all_memories(thisAgent);

    //Load the new memories
    nodelist = epmem_load_wmetree_from_file(thisAgent, f, thisAgent->epmem_wmetree, 0);
    epmem_load_epmems_from_file(thisAgent, f, nodelist);
    
    fclose(f);
    
}//epmem_load_episodic_memory_from_file


/* ===================================================================
   epmem_update() 

   This routine is called at every output phase to allow the episodic
   memory system to update its current memory store and respond to any
   queries.

   NOTE:  This function is the "grand central station" of the epmem
          subsystem. 

   Created: 19 Jan 2004
   =================================================================== */
void epmem_update(agent *thisAgent)
{
    arraylist *epmem = NULL;
    static int count = 0;
    int i;

    count++;

#ifdef DISABLE_EPMEM
    return;
#endif

    //Double check that I'm ready to go
    if ( (! ((thisAgent->sysparams)[EPMEM_SYSPARAM]))
         || ((thisAgent->sysparams)[EPMEM_SUSPENDED_SYSPARAM]))
    {
        return;
    }

#ifdef SILENT_CRASH
    //See the comment below
    __try
    {
#endif
    
    start_timer(thisAgent, &(thisAgent->epmem_start_time));

    //Update the stack of epmem_header structs
    epmem_update_header_stack(thisAgent);
    
    start_timer(thisAgent, &(thisAgent->epmem_record_start_time));

    
    //Consider recording a new epmem
    consider_new_epmem_via_output(thisAgent);

    stop_timer(thisAgent, &(thisAgent->epmem_record_start_time), &(thisAgent->epmem_record_total_time));
    start_timer(thisAgent, &(thisAgent->epmem_retrieve_start_time));
    
    /*
     * Update the ^retrieved link on each epmem header as necessary
     */

    for(i = 0; i < thisAgent->epmem_header_stack->size; i++)
    {
        epmem_header *h = (epmem_header *)get_arraylist_entry(thisAgent, thisAgent->epmem_header_stack, i);

        //Process any commands from the agent
        epmem_retrieve_commands(thisAgent, h);

        //Respond to the "next" command
        if (h->next_cmd) 
        {
            respond_to_command_next(thisAgent, h);
            continue;
        }
        
        //Respond to the "query" and/or "negquery" command
        if ( (h->query_cmd) || (h->negquery_cmd) )
        {
            //Handle any queries on the input link
            epmem = respond_to_query(thisAgent, h);
        }
    }
        
    //Save the current epmem store at regular intervals
    if ((thisAgent->epmem_save_freq != 0)
        && (count % thisAgent->epmem_save_freq == 0)
        && (strlen(thisAgent->epmem_autosave_filename) != 0))
    {
        epmem_save_episodic_memory_to_file(thisAgent,
                                           thisAgent->epmem_autosave_filename);
    }
    
    stop_timer(thisAgent, &(thisAgent->epmem_retrieve_start_time), &(thisAgent->epmem_retrieve_total_time));
    stop_timer(thisAgent, &(thisAgent->epmem_start_time), &(thisAgent->epmem_total_time));

    
    


    //This is Windows specific code to avoid the GPF dialog box
    //when things go awry.  It's VERY HANDY when running multiple
    //batches.
#ifdef SILENT_CRASH
    }
    __except(1)
    {
        print(thisAgent, "\nENCOUNTERED EPMEM CRASH!  Exiting quietly. (log message)\n");
        stop_log_file (thisAgent);

        fprintf(stderr, "\nENCOUNTERED EPMEM CRASH!  Exiting quietly. (stderr message)\n");
        fflush(stderr);
        
        _set_abort_behavior( 0, _WRITE_ABORT_MSG);

        TerminateProcess(GetCurrentProcess(), 0); 
    }
#endif //SILENT_CRASH    
    
}//epmem_update

    
    
/* ===================================================================
   epmem_create_buffer()

   This routine creates the ^epmem link on the top state.  This is
   used to query the episodic memory and provided the retrieved
   result.  The Soar kernel is expected to call this function once
   the top state has been created.

   Created: 08 Jan 2004
   Updated: 04 Oct 2004 - to handle new epmem_header stack
   =================================================================== */

void epmem_create_buffer(agent *thisAgent, Symbol *s)
{
    epmem_header *top_state_header;

#ifdef DISABLE_EPMEM
    return;
#endif
    
    top_state_header = make_epmem_header(thisAgent, s);
    top_state_header->index = 0;
    append_entry_to_arraylist(thisAgent, thisAgent->epmem_header_stack, (void *)top_state_header);
    
}//epmem_create_buffer


/* ===================================================================
   epmem_deinit()

   This routine deinitializes the epmem system.

   Created: 02 Mar 2007
   =================================================================== */

void epmem_deinit(agent *thisAgent) 
{
    int i;
    
#ifdef DISABLE_EPMEM
    return;
#endif

    //If I'm not initialized then don't re-deinitialize
    if (! ((thisAgent->sysparams)[EPMEM_SYSPARAM]))
    {
        return;
    }

    //Deallocate the active wmes arrays
    if (thisAgent->epmem_curr_active_wmes)
    {
        free(thisAgent->epmem_curr_active_wmes);
        thisAgent->epmem_curr_active_wmes = NULL;
    }
    if (thisAgent->epmem_prev_active_wmes)
    {
        free(thisAgent->epmem_prev_active_wmes);
        thisAgent->epmem_prev_active_wmes = NULL;
    }

    //Remove all episodic memories
    epmem_clear_all_memories(thisAgent);
    

    //Deallocate the thisAgent->epmem_header_stack array
    for(i = thisAgent->epmem_header_stack->size - 1; i >= 0; i--)
    {
        destroy_epmem_header(
            thisAgent, 
            (epmem_header *)remove_entry_from_arraylist(thisAgent->epmem_header_stack, i));
    }
    destroy_arraylist(thisAgent, thisAgent->epmem_header_stack);

    //Note that epmem is now inactive
    set_sysparam(thisAgent, EPMEM_SYSPARAM, FALSE);
    
}/*epmem_deinit*/


/* ===================================================================
   init_epemem()

   This routine is called once to initialize the episodic memory system.

   Created: 03 Nov 2002
   =================================================================== */

void epmem_init(agent *thisAgent) 
{
#ifdef DISABLE_EPMEM
    return;
#endif

    //If I've already initialized don't initialize again
    if ((thisAgent->sysparams)[EPMEM_SYSPARAM])
    {
        return;
    }

    //Allocate the active wmes arrays
    thisAgent->epmem_curr_active_wmes = (wme **)calloc(num_active_wmes, sizeof(wme *));
    thisAgent->epmem_prev_active_wmes = (wme **)calloc(num_active_wmes, sizeof(wme *));

    //Allocate and initialize the wmetree
    thisAgent->epmem_wmetree =
        (wmetree *)allocate_memory(thisAgent, sizeof(wmetree), MISCELLANEOUS_MEM_USAGE);
    thisAgent->epmem_wmetree->next = NULL;
    thisAgent->epmem_wmetree->attr = NULL;
    thisAgent->epmem_wmetree->id = 0;
    thisAgent->epmem_wmetree->val.intval = 0;
    thisAgent->epmem_wmetree->val_type = IDENTIFIER_SYMBOL_TYPE;
    thisAgent->epmem_wmetree->children = make_hash_table(thisAgent, 0, hash_wmetree);;
    thisAgent->epmem_wmetree->parent = NULL;
    thisAgent->epmem_wmetree->depth = 0;
    thisAgent->epmem_wmetree->assoc_wmes = make_arraylist(thisAgent, 20);
    thisAgent->epmem_wmetree_size = 1;

    //Initialize tracking variables used by the match routine
    thisAgent->epmem_last_tag = 0;
    thisAgent->epmem_last_ret_id = 0;
    thisAgent->epmem_num_queries = 0;
    
    
    //Initialize the memories array
    thisAgent->epmem_memories = make_arraylist(thisAgent, 512);

    //Initialize the thisAgent->epmem_header_stack array
    thisAgent->epmem_header_stack = make_arraylist(thisAgent, 20);

    //Initialize autosave values
    thisAgent->epmem_save_freq = 500;
    thisAgent->epmem_autosave_filename[0] = '\0';
#ifdef WIN32
    sprintf(thisAgent->epmem_autosave_filename, "c:\\temp\\%s_epmems.txt", thisAgent->name);
#else
    sprintf(thisAgent->epmem_autosave_filename, "/tmp/%s_epmems.txt", thisAgent->name);
#endif

    //Reset the timers
    epmem_reset_cpu_usage_timers(thisAgent);

    //Note that epmem is now active
    set_sysparam(thisAgent, EPMEM_SYSPARAM, TRUE);
    
}/*epmem_init*/



#ifdef __cplusplus
}//extern "C"
#endif

#endif /* #ifdef EPISODIC_MEMORY */

/* ===================================================================
   =================================================================== */
