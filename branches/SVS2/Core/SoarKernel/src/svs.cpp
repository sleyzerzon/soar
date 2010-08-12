#include <portability.h>

#include "svs.h"

inline bool _svs_detect_id_changes(Symbol* id, tc_number tc)
{
	bool return_val = false;
	std::stack< Symbol* > to_process;
	slot* s;
	wme* w;
	unsigned long my_counter = 0;

	to_process.push(id);

	while (!to_process.empty())
	{
		id = to_process.top();
		to_process.pop();
		
		for (s=id->id.slots; s!=NIL; s=s->next)
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

inline void _svs_parse_command_wme(agent* my_agent, Symbol* state, wme* w)
{
	if (w->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE)
	{
		svs_data* svs = state->id.svs_info;
		
		if (w->value->id.svs_id == SVS_NEW_COMMAND)
		{
			w->value->id.svs_id = my_agent->svs_command_counter++;
			(*svs->command_map)[w->value->id.svs_id] = w->value;
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
	}
}

void svs_parse_command_id(agent* my_agent, Symbol* state, Symbol* id)
{
	slot* s;
	wme* w;

	for (s=id->id.slots; s!=NIL; s=s->next)
	{
		for (w=s->wmes; w!=NIL; w=w->next)
		{
			_svs_parse_command_wme(my_agent, state, w);
		}
	}
}
