#include <stdlib.h>
#include <ctype.h>
#include <sstream>

#include "cmd_watcher.h"
#include "filter.h"
#include "svs.h"
#include "scene.h"
#include "soar_interface.h"
#include "ipcsocket.h"
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

class cmd_base : public cmd_watcher {
public:
	cmd_base(svs_state *state, sym_hnd cmd_root)
	: state(state), si(state->get_soar_interface()), cmd_root(cmd_root), result_wme(NULL), subtree_size(0), max_time_tag(0)
	{ }
	
	void set_result(const string &r) {
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
	
	bool cmd_changed() {
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
	
	bool get_str_param(const string &name, string &val) {
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
	filter* make_filter() {
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

protected:
	svs_state       *state;
	sym_hnd         cmd_root;
	soar_interface* si;

private:
	filter* rec_make_filter(wme_hnd cmd) {
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
	
	filter* make_node_filter(string name) {
		sg_node *n;
		
		n = state->get_scene()->get_node(name);
		if (!n) {
			return NULL;
		}
		return new const_node_filter(n);
	}
	
	bool get_filter_params(sym_hnd id, filter_params &p) {
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

	wme_hnd result_wme;
	int     subtree_size;
	int     max_time_tag;
};

class extract_cmd_watcher : public cmd_base, public filter_listener {
public:
	extract_cmd_watcher(svs_state *state, sym_hnd cmd_root)
	: cmd_base(state, cmd_root), dirty(true), result_filter(NULL)
	{}
	
	~extract_cmd_watcher() {
		if (result_filter) {
			delete result_filter;
		}
	}
	
	void update(filter *f) {
		dirty = true;
	}
	
	bool update_result() {
		filter_result *r;
		
		if (cmd_changed()) {
			if (result_filter) {
				delete result_filter;
			}
			result_filter = make_filter();
			if (!result_filter) {
				set_result("command error");
				return false;
			}
			result_filter->listen(this);
			dirty = true;
		}
		
		if (result_filter && dirty) {
			dirty = false;
			if (!(r = result_filter->get_result())) {
				set_result(result_filter->get_error());
				return false;
			} else {
				set_result(r->get_string());
			}
		}
		return true;
	}
	
	bool early() { return false; }
	
private:
	filter* result_filter;
	bool    dirty;
};

class gen_cmd_watcher : public cmd_base, public filter_listener, public sg_listener {
public:
	gen_cmd_watcher(svs_state *state, sym_hnd cmd_root)
	: cmd_base(state, cmd_root), parent(NULL), generated(NULL), result_filter(NULL)
	{}
	
	~gen_cmd_watcher() {
		if (result_filter) {
			delete result_filter;
		}
		if (generated) {
			delete generated;
		}
	}
	
	void update(sg_node* n, sg_node::change_type t) {
		if (t == sg_node::DELETED) {
			parent = NULL;
			generated = NULL;  // only valid if node deletion is recursive, may change
		}
	}
	
	void update(filter *f) {
		dirty = true;
	}

	bool update_result() {
		filter_result *r;
		if (cmd_changed()) {
			if (result_filter) {
				delete result_filter;
			}
			if (!(result_filter = make_filter())) {
				set_result("command error");
				return false;
			}
			dirty = true;
			result_filter->listen(this);
			get_parent();
		}
		
		if (!parent) {
			set_result("no parent");
			return false;
		}
		
		if (result_filter && dirty) {
			dirty = false;
			if (generated) {
				delete generated;
				generated = NULL;
			}
			if (!get_node_filter_result_value(NULL, result_filter, generated)) {
				set_result(result_filter->get_error());
				return false;
			}
			parent->attach_child(generated);
			set_result("t");
		}
		
		return true;
	}
	
	bool early() { return false; }
	
private:

	bool get_parent() {
		string name;
		sg_node *new_parent;
		
		if (!get_str_param("parent", name)) {
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
	
	sg_node   *parent;
	sg_node   *generated;
	filter    *result_filter;
	bool      dirty;
};

class recall_cmd_watcher : public cmd_base {
public:
	recall_cmd_watcher(svs_state *state, sym_hnd cmd_root)
	: cmd_base(state, cmd_root)
	{
		wme_hnd w;
		long scene_num;
		stringstream ss;
		string resp;
		
		ipcsocket *ipc = state->get_ipc();
		
		if (!si->find_child_wme(cmd_root, "scene-num", w)) {
			set_result("missing state_num parameter");
			return;
		}
		
		if (!si->get_val(si->get_wme_val(w), scene_num)) {
			set_result("scene-num not integer type");
			return;
		}
		
		ss << scene_num;
		
		if (ipc->communicate("recall", state->get_level(), ss.str(), resp) == "error") {
			set_result(resp);
		} else {
			state->wipe_scene();
			state->update(resp);
			set_result("success");
		}
	}

	bool update_result() {
		return true;
	}
	
	bool early() {
		return true;
	}
};

class ctrl_cmd_watcher : public cmd_base {
public:
	ctrl_cmd_watcher(svs_state *state, sym_hnd cmd_root)
	: cmd_base(state, cmd_root), ipc(state->get_ipc()), step(0), stepwme(NULL), broken(false)
	{
		wme_hnd w;
		string type, msg, resp;
		int r;
		
		type = parse(msg);
		if (type == "error") {
			broken = true;
			set_result(msg);
			return;
		}
		if (ipc->communicate(type, state->get_level(), msg, resp) == "error") {
			broken = true;
			set_result(msg);
			return;
		}
		id = atoi(resp.c_str());
		si->make_wme(cmd_root, "trajectory", id);
		update_step();
	}
	
	bool update_result() {
		string resp, obj;
		stringstream ss;
		
		if (broken) {
			return false;
		}
		ss << id << endl;
		if (ipc->communicate("stepctrl", state->get_level(), ss.str(), resp) == "error") {
			set_result(resp);
			return false;
		}
		state->update(resp);
		set_result("success");
		step++;
		update_step();
		return true;
	}
	
	bool early() { return true; }
	
private:
	string parse(string &msg) {
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

	void update_step() {
		if (stepwme)
			si->remove_wme(stepwme);
		stepwme = si->make_wme(cmd_root, "step", step);
	}
	
	ipcsocket *ipc;
	wme_hnd   stepwme;
	int       id;
	int       step;
	bool      broken;
};

class model_cmd_watcher : public cmd_base {
public:
	model_cmd_watcher(svs_state *state, sym_hnd cmd_root)
	: cmd_base(state, cmd_root), ipc(state->get_ipc())
	{ }
	
	bool update_result() {
		wme_hnd hashwme;
		sym_hnd hashid;
		wme_list childs;
		wme_list::iterator i;
		stringstream ss;
		string attr, val, resp;
		
		if (!cmd_changed()) {
			return true;
		}
		if (!si->find_child_wme(cmd_root, "hash", hashwme)) {
			set_result("no hash");
			return false;
		}
		
		si->get_child_wmes(si->get_wme_val(hashwme), childs);
		for (i = childs.begin(); i != childs.end(); ++i) {
			if (!si->get_val(si->get_wme_attr(*i), attr) ||
			    !si->get_val(si->get_wme_val(*i), val))
			{
				set_result("invalid child");
				return false;
			}
			ss << attr << " " << val << endl;
		}
		if (ipc->communicate("model", state->get_level(), ss.str(), resp) == "error") {
			set_result(resp);
			return false;
		}
		return true;
	}
	
	bool early() {
		return true;
	}
	
private:
	ipcsocket *ipc;
};

class hold_cmd_watcher : public cmd_base {
public:
	hold_cmd_watcher(svs_state *state, sym_hnd cmd_root)
	: cmd_base(state, cmd_root), ipc(state->get_ipc())
	{ }
	
	~hold_cmd_watcher() {
		stringstream s;
		string r;
		copy(ids.begin(), ids.end(), ostream_iterator<string>(s, " "));
		ipc->communicate("unhold", state->get_level(), s.str(), r);
	}
	
	bool update_result() {
		wme_list childs;
		wme_list::iterator i;
		stringstream s;
		string n, r;
		
		if (!cmd_changed()) {
			return true;
		}
		if (!si->get_child_wmes(cmd_root, childs)) {
			set_result("error");
			return false;
		}
		
		if (ids.size() > 0) {
			copy(ids.begin(), ids.end(), ostream_iterator<string>(s, " "));
			if (ipc->communicate("unhold", state->get_level(), s.str(), r) == "error") {
				set_result(r);
				return false;
			}
		}
		ids.clear();
		for (i = childs.begin(); i != childs.end(); ++i) {
			if (!si->get_val(si->get_wme_val(*i), n)) {
				continue;
			}
			ids.push_back(n);
			s << n << " ";
		}
		if (ipc->communicate("hold", state->get_level(), s.str(), r) == "error") {
			set_result(r);
			return false;
		}
		return true;
	}
	
	bool early() {
		return true;
	}
	
private:
	ipcsocket      *ipc;
	vector<string> ids;
};


cmd_watcher* make_cmd_watcher(svs_state *state, wme_hnd w) {
	sym_hnd id;
	string attr;
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
	} else if (attr == "hold") {
		return new hold_cmd_watcher(state, id);
	}
	return NULL;
}



