
#ifndef SVS_H
#define SVS_H

#include <list>
#include <map>

#define SVS_NEW_COMMAND 0

typedef unsigned long svs_command_id;

// data associated with each state
typedef struct svs_data_struct
{
	std::list<svs_command_id>* new_command_list;			// new commands this dc
	std::list<svs_command_id>* modified_command_list;		// existing commands modified this dc (distinct from new)
	std::list<svs_command_id>* removed_command_list;		// former commands that were removed this dc
	
	std::map< svs_command_id, wme* >* command_map;			// association between namespaced id and identifiers 
} svs_data;

#endif // SVS_H
