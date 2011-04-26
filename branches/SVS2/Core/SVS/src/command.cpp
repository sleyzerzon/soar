#include <stdlib.h>
#include <ctype.h>
#include <sstream>
#include <limits>

#include "command.h"
#include "filter.h"
#include "svs.h"
#include "scene.h"
#include "soar_interface.h"
#include "ipcsocket.h"

using namespace std;

bool is_reserved_param(const string &name) {
	return name == "result" || name == "parent";
}

/* Remove weird characters from string */
void cleanstring(string &s) {
	string::iterator i;
	for (i = s.begin(); i != s.end();) {
		if (!isalnum(*i) && *i != '.' && *i != '-' && *i != '_') {
			i = s.erase(i);
		} else {
			++i;
		}
	}
}

cmd_utils::cmd_utils(svs_state *state, Symbol *cmd_root)
: state(state), si(state->get_svs()->get_soar_interface()), cmd_root(cmd_root), result_wme(NULL), subtree_size(0), max_time_tag(0)
{ }

void cmd_utils::set_result(const string &r) {
	string current;
	if (result_wme) {
		if (si->get_val(si->get_wme_val(result_wme), current)) {
			if (r == current) {
				return;
			}
		}
		si->remove_wme(result_wme);
	}
	result_wme = si->make_wme(cmd_root, "result", r);
}

bool cmd_utils::cmd_changed() {
	tc_num tc;
	bool changed;
	stack< Symbol *> to_process;
	wme_list childs;
	wme_list::iterator i;
	Symbol *parent, *v;
	int new_subtree_size = 0, tt;
	string attr;

	tc = si->new_tc_num();
	changed = false;
	
	to_process.push(cmd_root);
	while (!to_process.empty()) {
		parent = to_process.top();
		to_process.pop();
		
		si->get_child_wmes(parent, childs);
		for (i = childs.begin(); i != childs.end(); ++i) {
			if (parent == cmd_root) {
				if (si->get_val(si->get_wme_attr(*i), attr) && attr == "result") {
					/* result wmes are added by svs */
					continue;
				}
			}
			v = si->get_wme_val(*i);
			tt = si->get_timetag(*i);
			new_subtree_size++;
			
			if (tt > max_time_tag) {
				changed = true;
				max_time_tag = tt;
			}

			if (si->is_identifier(v) && si->get_tc_num(v) != tc) {
				si->set_tc_num(v, tc);
				to_process.push(v);
			}
		}
	}

	if (new_subtree_size != subtree_size) {
		changed = true;
		subtree_size = new_subtree_size;
	}

	return changed;
}

bool cmd_utils::get_str_param(const string &name, string &val) {
	wme_list children;
	wme_list::iterator i;
	string attr, v;
	
	si->get_child_wmes(cmd_root, children);
	for(i = children.begin(); i != children.end(); ++i) {
		if (si->get_val(si->get_wme_attr(*i), attr)) {
			if (name != attr) {
				continue;
			}
			if (si->get_val(si->get_wme_val(*i), v)) {
				val = v;
				return true;
			}
		}
	}
	return false;
}

command *_make_extract_command_(svs_state *state, Symbol *root);
command *_make_generate_command_(svs_state *state, Symbol *root);
command *_make_control_command_(svs_state *state, Symbol *root);

command* make_command(svs_state *state, wme *w) {
	string name;
	Symbol *id;
	soar_interface *si;
	
	si = state->get_svs()->get_soar_interface();
	if (!si->get_val(si->get_wme_attr(w), name)) {
		return NULL;
	}
	if (!si->is_identifier(si->get_wme_val(w))) {
		return NULL;
	}
	id = si->get_wme_val(w);
	if (name == "extract") {
		return _make_extract_command_(state, id);
	} else if (name == "generate") {
		return _make_generate_command_(state, id);
	} else if (name == "control") {
		return _make_control_command_(state, id);
	}
	return NULL;
}
