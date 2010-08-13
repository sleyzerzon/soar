#include <portability.h>

#include <set>

#include "svs.h"

inline bool _svs_detect_id_changes(Symbol* id, tc_number tc)
{
	bool return_val = false;
	std::stack< Symbol* > to_process;
	slot* s;
	wme* w;
	Symbol* parent;
	unsigned long my_counter = 0;

	to_process.push(id);

	while (!to_process.empty())
	{
		parent = to_process.top();
		to_process.pop();
		
		for (s=parent->id.slots; s!=NIL; s=s->next)
		{
			for (w=s->wmes; w!=NIL; w=w->next)
			{
				my_counter++;
				
				if (w->timetag > id->id.svs_last_tt)
				{
					return_val = true;
					id->id.svs_last_tt = w->timetag;
				}

				if (w->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE)
				{
					if (w->value->id.tc_num != tc)
					{
						w->value->id.tc_num = tc;
						to_process.push(w->value);
					}
				}
			}
		}
	}

	if (my_counter != id->id.svs_last_count)
	{
		return_val = true;
		id->id.svs_last_count = my_counter;
	}

	return return_val;
}

inline void _svs_parse_command_wme(agent* my_agent, Symbol* state, wme* w, std::set< svs_command_id >& all_existing)
{
	if (w->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE)
	{
		svs_data* svs = state->id.svs_info;	
		
		if (w->value->id.svs_id == SVS_NEW_COMMAND)
		{
			w->value->id.svs_id = my_agent->svs_command_counter++;
			(*svs->command_map)[w->value->id.svs_id] = w;
			svs->new_command_list->push_back(w->value->id.svs_id);

			_svs_detect_id_changes(w->value, get_new_tc_number( my_agent ));
		}
		else
		{			
			if (_svs_detect_id_changes(w->value, get_new_tc_number( my_agent )))
			{
				svs->modified_command_list->push_back(w->value->id.svs_id);
			}
		}

		all_existing.insert(w->value->id.svs_id);
	}
}

inline void _svs_parse_command_id(agent* my_agent, Symbol* state, Symbol* id, std::set< svs_command_id >& all_existing)
{
	slot* s;
	wme* w;

	for (s=id->id.slots; s!=NIL; s=s->next)
	{
		for (w=s->wmes; w!=NIL; w=w->next)
		{
			_svs_parse_command_wme(my_agent, state, w, all_existing);
		}
	}
}

inline void _svs_process_state(agent* my_agent, Symbol* state, std::set< svs_command_id >& all_existing)
{
	assert( state->common.symbol_type == IDENTIFIER_SYMBOL_TYPE );
	assert( state->id.isa_goal );
	
	_svs_parse_command_id(my_agent, state, state->id.svs_spatial_scene_command_header, all_existing);
	if ( state->id.svs_ltm_command_header )
	{
		_svs_parse_command_id(my_agent, state, state->id.svs_ltm_command_header, all_existing);
	}
}

void svs_process_commands(agent* my_agent)
{
	Symbol* state;
	svs_data* svs;
	std::set< svs_command_id > all_existing;

	for (state=my_agent->bottom_goal; state!=NIL; state=state->id.higher_goal)
	{
		svs = state->id.svs_info;	

		// clear state lists
		svs->new_command_list->clear();
		svs->modified_command_list->clear();
		svs->removed_command_list->clear();

		// populate new, modified, stuff needed for removed
		_svs_process_state(my_agent, state, all_existing);

		// populate removed
		for (std::map< svs_command_id, wme* >::iterator it=svs->command_map->begin(); it!=svs->command_map->end(); it++)
		{
			if (all_existing.find(it->first) == all_existing.end())
			{
				svs->removed_command_list->push_back(it->first);
			}
		}

		// erase removed from command map
		for (std::list< svs_command_id >::iterator it=svs->removed_command_list->begin(); it!=svs->removed_command_list->end(); it++)
		{
			svs->command_map->erase( svs->command_map->find( *it ) );
		}

		// clear intermediate data
		all_existing.clear();
	}
}
