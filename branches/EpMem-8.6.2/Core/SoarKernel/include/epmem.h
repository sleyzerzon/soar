/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/* =======================================================================
                               epmem.h
======================================================================= */

#ifndef EPMEM_H
#define EPMEM_H

#include "symtab.h"

#ifdef __cplusplus
extern "C"
{
#endif

void init_epmem(agent *thisAgent);
void epmem_create_buffer(agent *thisAgent, Symbol *);
void epmem_update(agent *thisAgent);
void epmem_print_status(agent *thisAgent);
void epmem_print_memory_by_id(agent *thisAgent, int id);
void epmem_print_match_diagnostic(agent *thisAgent, int state_num);
void epmem_print_memory_comparison_by_id(agent *thisAgent, int id1, int id2);
void epmem_print_cue_comparison(agent *thisAgent, int state_num, int epmem_id);
    
#ifdef __cplusplus
}//extern "C"
#endif



#endif  //EPMEM_H
