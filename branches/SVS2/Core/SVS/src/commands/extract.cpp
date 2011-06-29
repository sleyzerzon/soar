#include <iostream>
#include "command.h"
#include "filter.h"
#include "svs.h"

using namespace std;

class extract_command : public command, public filter_listener {
public:
	extract_command(svs_state *state, Symbol *root)
	: root(root), state(state), utils(state, root), dirty(true), result_filter(NULL)
	{}
	
	~extract_command() {
		if (result_filter) {
			delete result_filter;
		}
	}
	
	string description() {
		return string("extract");
	}
	
	void update(filter *f) {
		dirty = true;
	}
	
	bool update_result() {
		filter_result *r;
		
		if (utils.cmd_changed()) {
			if (result_filter) {
				delete result_filter;
			}
			result_filter = parse_filter_struct(state->get_svs()->get_soar_interface(), root, state->get_scene());
			if (!result_filter) {
				utils.set_result("command error");
				return false;
			}
			result_filter->listen(this);
			dirty = true;
		}
		
		if (result_filter && dirty) {
			dirty = false;
			if (!(r = result_filter->get_result())) {
				utils.set_result(result_filter->get_error());
				return false;
			} else {
				utils.set_result(r->get_string());
			}
		}
		return true;
	}
	
	bool early() { return false; }
	
private:
	cmd_utils       utils;
	Symbol         *root;
	svs_state      *state;
	filter         *result_filter;
	bool            dirty;
};

command *_make_extract_command_(svs_state *state, Symbol *root) {
	return new extract_command(state, root);
}
