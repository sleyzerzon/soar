#include <iostream>
#include "command.h"
#include "filter.h"
#include "svs.h"

using namespace std;

class extract_command : public command {
public:
	extract_command(svs_state *state, Symbol *root)
	: command(state, root), root(root), state(state), fltr(NULL), res(NULL), first(true)
	{
		si = state->get_svs()->get_soar_interface();
	}
	
	~extract_command() {
		if (fltr) {
			delete fltr;
		}
	}
	
	string description() {
		return string("extract");
	}
	
	bool update() {
		if (changed()) {
			clear_results();
			if (fltr) {
				delete fltr;
			}
			fltr = parse_filter_spec(state->get_svs()->get_soar_interface(), root, state->get_scene());
			if (!fltr) {
				set_status("command error");
				return false;
			}
			res = fltr->get_result();
			first = true;
		}
		
		if (fltr) {
			fltr->update();
				
			if (fltr->is_error()) {
				set_status(fltr->get_error());
				clear_results();
				return false;
			}
			if (first) {
				reset_results();
				first = false;
			} else {
				update_results();
			}
			set_status("success");
		}
		return true;
	}
	
	bool early() { return false; }
	
	void reset_results() {
		filter_result::iter i;
		
		clear_results();
		for (i = res->curr_begin(); i != res->curr_end(); ++i) {
			res2wme[*i] = make_result_wme(*i);
		}
		res->clear_changes();
	}
	
	void update_results() {
		filter_result::iter i;
		wme *w;
		for (i = res->added_begin(); i != res->added_end(); ++i) {
			if (!map_get(res2wme, *i, w)) {
				assert(false);
			}
			res2wme[*i] = make_result_wme(*i);
		}
		for (i = res->removed_begin(); i != res->removed_end(); ++i) {
			if (!map_pop(res2wme, *i, w)) {
				assert(false);
			}
			si->remove_wme(w);
		}
		for (i = res->changed_begin(); i != res->changed_end(); ++i) {
			if (!map_get(res2wme, *i, w)) {
				assert(false);
			}
			si->remove_wme(w);
			res2wme[*i] = make_result_wme(*i);
		}
	}
	
	void clear_results() {
		std::map<filter_val*, wme*>::iterator i;
		for (i = res2wme.begin(); i != res2wme.end(); ++i) {
			si->remove_wme(i->second);
		}
		res2wme.clear();
	}
	
	wme *make_result_wme(filter_val *v) {
		return si->make_wme(root, "result", v->get_string());
	}
	
private:
	Symbol         *root;
	svs_state      *state;
	soar_interface *si;
	filter         *fltr;
	filter_result  *res;
	bool            first;
	
	std::map<filter_val*, wme*> res2wme;
};

command *_make_extract_command_(svs_state *state, Symbol *root) {
	return new extract_command(state, root);
}
