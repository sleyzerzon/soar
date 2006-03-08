#ifndef _SMEM_H_
#define _SMEM_H_

#include "soarkernel.h"
#include "agent.h"
#include <map>
#include <hash_map>
#include <iostream>
#include <string>
#include <sstream> // for StringToInt
using namespace std;

static YJ_debug = 1;

void retrieve_1_20(agent* thisAgent);
void retrieve(agent* thisAgent);
void smem_save_wme (agent* thisAgent, wme* w);
void save_wmes_12_21(agent* thisAgent);
void save_wmes(agent* thisAgent);
void save_wmes_old(agent* thisAgent);
void find_save_ids(agent* thisAgent, set<std::string>&);
void find_save_wmes(agent* thisAgent, set<LME>&);
void find_save_wmes_all(agent* thisAgent, set<LME>&);
bool long_term_value(agent* thisAgent, const set<std::string>& saved_ids, string value, int value_type);
Symbol *make_identifier_from_str (agent* thisAgent, string id_str, goal_stack_level level=TOP_GOAL_LEVEL);
string symbol_constant_to_string(agent* thisAgent, Symbol* s);

#endif /* _SMEM_H_ */