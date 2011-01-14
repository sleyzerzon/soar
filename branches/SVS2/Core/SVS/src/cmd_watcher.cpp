#include <stdlib.h>
#include <ctype.h>
#include <sstream>
#include "cmd_watcher.h"
#include "filter.h"
#include "filters/gen_filter.h"

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

cmd_watcher* make_cmd_watcher(svs_state *state, wme_hnd w) {
	sym_hnd        id;
	string         attr;
	soar_interface *si = state->get_soar_interface();
	
	if (!si->get_val(si->get_wme_attr(w), attr)) {
		return NULL;
	}
	id = si->get_wme_val(w);
	if (!si->is_identifier(id)) {
		return NULL;
	}
	
	if (attr == "extract") {
		return new extract_cmd_watcher(state, id);
	} else if (attr == "generate") {
		return new gen_cmd_watcher(state, id);
	} else if (attr == "control") {
		return new ctrl_cmd_watcher(state, id);
	} else if (attr == "recall") {
		return new recall_cmd_watcher(state, id);
	} else if (attr == "model") {
		return new model_cmd_watcher(state, id);
	} else {
		assert(false);
	}
	return NULL;
}

cmd_utils::cmd_utils(svs_state *state, sym_hnd cmd_root)
: state(state), si(state->get_soar_interface()), cmd_root(cmd_root), result_wme(NULL), subtree_size(0), max_time_tag(0)
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

bool cmd_utils::cmd_changed() {
	tc_num tc;
	bool changed;
	stack< sym_hnd > to_process;
	wme_list childs;
	wme_list::iterator i;
	sym_hnd parent, v;
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

/* Filters are specified in WM like this:

   (<cmd> ^filtername1 <f1>)
   (<f1> ^_named-param1 <f2> 
         ^_named-param1 node_name1
         ^filtername2 <f3>)
   (<f2> ^filtername3 <f4>)
   (<f3> ^filtername4 node_name2)
   ...
   
   Parameters can be named or anonymous ("" will be the param name), and
   filters can either be identifiers, in which case substructure will be
   parsed recursively, or string constants that specify node filters.
*/
filter* cmd_utils::make_filter() {
	wme_list children;
	wme_list::iterator i;
	string attr;
	
	si->get_child_wmes(cmd_root, children);
	for (i = children.begin(); i != children.end(); ++i) {
		if (si->get_val(si->get_wme_attr(*i), attr)) {
			if (!is_reserved_param(attr)) {
				return rec_make_filter(*i);
			}
		}
	}
	return NULL;
}

filter* cmd_utils::rec_make_filter(wme_hnd cmd) {
	string filter_name, node_name;
	filter_params p;
	
	if (!si->get_val(si->get_wme_attr(cmd), filter_name)) {
		return NULL;
	}
	
	if (!get_filter_params(si->get_wme_val(cmd), p)) {
		return NULL;
	}
	
	return ::make_filter(filter_name, p);
}

bool cmd_utils::get_filter_params(sym_hnd id, filter_params &p) {
	wme_list childs, param_childs;
	wme_list::iterator i;
	filter *f;
	string attr, node_name, param_name;
	bool fail;

	if (!si->is_identifier(id)) {
		return false;
	}
	
	si->get_child_wmes(id, childs);
	fail = false;
	for (i = childs.begin(); i != childs.end(); ++i) {
		if (!si->get_val(si->get_wme_attr(*i), attr)) {
			fail = true;
			break;
		}
		if (si->get_val(si->get_wme_val(*i), node_name)) {  // scene graph node
			f = make_node_filter(node_name); 
		} else {   // another filter
			si->get_child_wmes(si->get_wme_val(*i), param_childs);
			if (param_childs.size() != 1) {
				fail = true;
				break;
			}
			f = rec_make_filter(param_childs.front());
		}
		if (!f) {
			fail = true;
			break;
		}
		p.insert(pair<string,filter*>(attr, f));
	}
	
	if (fail) {
		filter_params::iterator j;
		
		for (j = p.begin(); j != p.end(); ++j) {
			delete j->second;
		}
		p.clear();
		return false;
	}
	return true;
}

filter* cmd_utils::make_node_filter(string name) {
	sg_node *n;
	
	n = state->get_scene()->get_node(name);
	if (!n) {
		return NULL;
	}
	return new const_node_filter(n);
}

extract_cmd_watcher::extract_cmd_watcher(svs_state *state, sym_hnd cmd_root)
: state(state), utils(state, cmd_root), dirty(true), result_filter(NULL)
{}

extract_cmd_watcher::~extract_cmd_watcher() {
	if (result_filter) {
		delete result_filter;
	}
}

void extract_cmd_watcher::update(filter *f) {
	dirty = true;
}

bool extract_cmd_watcher::update_result() {
	filter_result *r;
	
	if (utils.cmd_changed()) {
		if (result_filter) {
			delete result_filter;
		}
		result_filter = utils.make_filter();
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

gen_cmd_watcher::gen_cmd_watcher(svs_state *state, sym_hnd cmd_root) 
: state(state), utils(state, cmd_root), cmd_root(cmd_root), parent(NULL), generated(NULL), result_filter(NULL)
{ }

gen_cmd_watcher::~gen_cmd_watcher() {
	if (result_filter) {
		delete result_filter;
	}
	if (generated) {
		delete generated;
	}
}

bool gen_cmd_watcher::get_parent() {
	string name;
	sg_node *new_parent;
	
	if (!utils.get_str_param("parent", name)) {
		parent = NULL;
		return false;
	}
	
	node_name_map::iterator j;
	if (!(new_parent = state->get_scene()->get_node(name))) {
		return false;
	}
	
	if (parent == new_parent) {
		return true;
	}
	
	if (parent) {
		parent->unlisten(this);
	}
	new_parent->listen(this);
	parent = new_parent;
	return true;
}

void gen_cmd_watcher::update(sg_node* n, sg_node::change_type t) {
	if (t == sg_node::DELETED) {
		parent = NULL;
		generated = NULL;  // only valid if node deletion is recursive, may change
	}
}

void gen_cmd_watcher::update(filter *f) {
	dirty = true;
}

bool gen_cmd_watcher::update_result() {
	filter_result *r;
	if (utils.cmd_changed()) {
		if (result_filter) {
			delete result_filter;
		}
		if (!(result_filter = utils.make_filter())) {
			utils.set_result("command error");
			return false;
		}
		dirty = true;
		result_filter->listen(this);
		get_parent();
	}
	
	if (!parent) {
		utils.set_result("no parent");
		return false;
	}
	
	if (result_filter && dirty) {
		dirty = false;
		if (generated) {
			delete generated;
			generated = NULL;
		}
		if (!get_node_filter_result_value(NULL, result_filter, generated)) {
			utils.set_result(result_filter->get_error());
			return false;
		}
		parent->attach_child(generated);
		utils.set_result("t");
	}
	
	return true;
}

ctrl_cmd_watcher::ctrl_cmd_watcher(svs_state *state, sym_hnd cmd_root)
: state(state), si(state->get_soar_interface()), ipc(state->get_ipc()), cmd_root(cmd_root), 
  utils(state, cmd_root), step(0), stepwme(NULL), broken(false)
{
	wme_hnd w;
	string type, msg, resp;
	int r;
	
	type = parse(msg);
	if (type == "error") {
		broken = true;
		utils.set_result(msg);
		return;
	}
	if (ipc->communicate(type, state->get_level(), msg, resp) == "error") {
		broken = true;
		utils.set_result(msg);
		return;
	}
	id = atoi(resp.c_str());
	si->make_wme(cmd_root, "trajectory", id);
	update_step();
}

string ctrl_cmd_watcher::parse(string &msg) {
	wme_hnd w;
	wme_list objargs;
	wme_list::iterator i;
	sym_hnd objid;
	string astr, vstr, name, negate;
	stringstream ss;
	long trajectoryid;
	
	if (si->find_child_wme(cmd_root, "objective", w)) {
		objid = si->get_wme_val(w);
		if (!si->get_child_wmes(objid, objargs)) {
			msg = "invalid objective";
			return "error";
		}
		for(i = objargs.begin(); i != objargs.end(); ++i) {
			if (!si->get_val(si->get_wme_attr(*i), astr) ||
			    !si->get_val(si->get_wme_val(*i), vstr))
			{
				msg = "invalid objective";
				return "error";
			}
			if (astr == "name") {
				name = vstr;
			} else if (astr == "negate") {
				negate = vstr;
				if (negate != "t" && negate != "f") {
					msg = "invalid negate value";
					return "error";
				}
			} else {
				cleanstring(astr); cleanstring(vstr);
				ss << astr << ' ' << vstr << endl;
			}
		}
		msg = name + " " + negate + "\n" + ss.str();
		return "seek";
	} else if (si->find_child_wme(cmd_root, "replay", w)) {
		if (!si->get_val(si->get_wme_val(w), trajectoryid)) {
			return "error";
		}
		ss << trajectoryid;
		msg = ss.str();
		return "replay";
	}
	
	return "random";
}

bool ctrl_cmd_watcher::update_result() {
	string resp, obj;
	stringstream ss;
	
	if (broken) {
		return false;
	}
	ss << id << endl;
	if (ipc->communicate("stepctrl", state->get_level(), ss.str(), resp) == "error") {
		utils.set_result(resp);
		return false;
	}
	state->update(resp);
	utils.set_result("success");
	step++;
	update_step();
	return true;
}

void ctrl_cmd_watcher::update_step() {
	if (stepwme)
		si->remove_wme(stepwme);
	stepwme = si->make_wme(cmd_root, "step", step);
}

recall_cmd_watcher::recall_cmd_watcher(svs_state *state, sym_hnd cmd_root)
: state(state), utils(state, cmd_root)
{
	wme_hnd w;
	long state_num;
	stringstream ss;
	string resp;
	
	ipcsocket *ipc = state->get_ipc();
	soar_interface *si = state->get_soar_interface();
	
	if (!si->find_child_wme(cmd_root, "state_num", w)) {
		utils.set_result("missing state_num parameter");
		return;
	}
	
	if (!si->get_val(si->get_wme_val(w), state_num)) {
		utils.set_result("state_num not integer type");
		return;
	}
	
	ss << state_num;
	
	if (ipc->communicate("recall", state->get_level(), ss.str(), resp) == "error") {
		utils.set_result(resp);
	} else {
		state->wipe_scene();
		state->update(resp);
		utils.set_result("success");
	}
}

bool recall_cmd_watcher::update_result() {
	return true;
}

model_cmd_watcher::model_cmd_watcher(svs_state *state, sym_hnd cmd_root)
: state(state), cmd_root(cmd_root), ipc(state->get_ipc()), si(state->get_soar_interface()), utils(state, cmd_root)
{ }

bool model_cmd_watcher::update_result() {
	wme_hnd hashwme;
	sym_hnd hashid;
	wme_list childs;
	wme_list::iterator i;
	stringstream ss;
	string attr, val, resp;
	
	if (!utils.cmd_changed()) {
		return true;
	}
	if (!si->find_child_wme(cmd_root, "hash", hashwme)) {
		utils.set_result("no hash");
		return false;
	}
	
	si->get_child_wmes(si->get_wme_val(hashwme), childs);
	for (i = childs.begin(); i != childs.end(); ++i) {
		if (!si->get_val(si->get_wme_attr(*i), attr) ||
		    !si->get_val(si->get_wme_val(*i), val))
		{
			utils.set_result("invalid child");
			return false;
		}
		ss << attr << " " << val << endl;
	}
	if (ipc->communicate("model", state->get_level(), ss.str(), resp) == "error") {
		utils.set_result(resp);
		return false;
	}
	return true;
}
