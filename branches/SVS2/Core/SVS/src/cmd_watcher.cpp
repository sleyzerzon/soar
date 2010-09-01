#include "cmd_watcher.h"
#include "filter.h"
#include "gen_filter.h"

bool is_reserved_param(const string &name) {
	return name == "result" || name == "parent";
}

cmd_utils::cmd_utils(soar_interface *si, sym_hnd cmd_root, scene *scn)
: si(si), cmd_root(cmd_root), result_wme(NULL), scn(scn), subtree_size(0), max_time_tag(0)
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
	result_wme = si->make_str_wme(cmd_root, "result", r);
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

	if (new_subtree_size != subtree_size)
	{
		changed = true;
		subtree_size = new_subtree_size;
	}

	return changed;
}

filter* cmd_utils::make_filter() {
	return rec_make_filter(cmd_root);
}


/* Filters are specified in WM like this:

   (<cmd> ^filtername1 <f1>)
   (<f1> ^arg1 <subfilter1> ^arg2 <subfilter2> ^arg3 node_name)
   (<subfilter1> ^filtername2 <f2>)
   ...
*/
filter* cmd_utils::rec_make_filter(sym_hnd cmd) {
	sym_hnd v;
	string filter_name, node_name, attr;
	wme_list children;
	wme_list::iterator i;
	wme_hnd w;
	filter_params p;
	filter *nf;
	
	/* a string symbol specifies a node filter */
	if (si->get_val(cmd, node_name)) {
		return make_node_filter(cmd);
	}
	
	si->get_child_wmes(cmd, children);
	w = NULL;
	for (i = children.begin(); i != children.end(); ++i) {
		if (!si->get_val(si->get_wme_attr(*i), filter_name)) {
			continue;
		}
		if (!is_reserved_param(filter_name)) {
			w = *i;
			break;
		}
	}
	if (!w) {
		return NULL;
	}

	v = si->get_wme_val(w);
	nf = make_node_filter(v);  // query wme with string value is shorthand for single node filter as param
	if (nf) {
		p.insert(pair<string,filter*>("",nf));
	} else {
		if (!si->is_identifier(v)) {
			return NULL;
		}
		if (!get_filter_params(v, p)) {
			return NULL;
		}
	}
	
	return ::make_filter(filter_name, p);
}

bool cmd_utils::get_filter_params(sym_hnd id, filter_params &p) {
	wme_list childs;
	wme_list::iterator i;
	filter *f;
	string attr;
	bool fail;
	
	si->get_child_wmes(id, childs);
	fail = false;
	for (i = childs.begin(); i != childs.end(); ++i) {
		if (!si->get_val(si->get_wme_attr(*i), attr)) {
			fail = true;
			break;
		}
		f = rec_make_filter(si->get_wme_val(*i));
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

filter* cmd_utils::make_node_filter(sym_hnd s) {
	string name;
	sg_node *n;
	
	if (!si->get_val(s, name)) {
		return NULL;
	}
	n = scn->get_node(name);
	if (!n) {
		cerr << "NODE " << name << " NOT FOUND" << endl;
		return NULL;
	}
	return new node_ptlist_filter(n);
}

extract_cmd_watcher::extract_cmd_watcher(soar_interface *si, sym_hnd cmd_root, scene *scn)
: utils(si, cmd_root, scn), dirty(true), result_filter(NULL)
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
	string r;
	
	if (utils.cmd_changed()) {
		if (result_filter) {
			delete result_filter;
		}
		result_filter = utils.make_filter();
		if (!result_filter) {
			utils.set_result("COMMAND_ERROR");
			return false;
		}
		result_filter->listen(this);
		dirty = true;
	}
	
	if (result_filter && dirty) {
		dirty = false;
		if (!result_filter->get_result_string(r)) {
			utils.set_result(result_filter->get_error());
			return false;
		} else {
			utils.set_result(r);
		}
	}
	return true;
}

gen_cmd_watcher::gen_cmd_watcher(soar_interface *si, sym_hnd cmd_root, scene *scn) 
: utils(si, cmd_root, scn), cmd_root(cmd_root), parent(NULL), generated(NULL), scn(scn), result_filter(NULL)
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
	if (!(new_parent = scn->get_node(name))) {
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
	filter *f;
	
	if (utils.cmd_changed()) {
		if (result_filter) {
			delete result_filter;
		}
		f = utils.make_filter();
		if (!f) {
			utils.set_result("COMMAND_ERROR");
			return false;
		}
		result_filter = dynamic_cast<node_filter*>(f);
		if (!result_filter) {
			delete f;
			utils.set_result("NOT_NODE_FILTER");
			return false;
		}
		dirty = true;
		result_filter->listen(this);
		get_parent();
	}
	
	if (!parent) {
		utils.set_result("NO_PARENT");
		return false;
	}
	
	if (result_filter && dirty) {
		dirty = false;
		if (generated) {
			delete generated;
			generated = NULL;
		}
		if (!result_filter->get_result(generated)) {
			utils.set_result(result_filter->get_error());
			return false;
		} else {
			parent->attach_child(generated);
			utils.set_result("t");
		}
	}
	
	return true;
}

cmd_watcher* make_cmd_watcher(soar_interface *si, scene *scn, wme_hnd w) {
	sym_hnd v;
	string attr;
	
	if (!si->get_val(si->get_wme_attr(w), attr)) {
		return NULL;
	}
	v = si->get_wme_val(w);
	if (!si->is_identifier(v)) {
		return NULL;
	}
	
	if (attr == "extract") {
		return new extract_cmd_watcher(si, v, scn);
	} else if (attr == "generate") {
		return new gen_cmd_watcher(si, v, scn);
	}
	return NULL;
}
