#include <iostream>
#include <assert.h>
#include "svs.h"
#include "nsg_node.h"
#include "wm_sgo.h"
#include "soar_int.h"

using namespace std;

void update_node_name_map(sg_node *n, node_name_map &m) {
	int i;
	m[n->get_name()] = n;
	for (i = 0; i < n->get_nchilds(); ++i) {
		update_node_name_map(n->get_child(i), m);
	}
}


svs_state::svs_state(sym_hnd state_id, svs_state *parent, soar_interface *interface, common_syms *syms)
: state(state_id), si(interface), cs(syms)
{
	link = si->make_id_wme(state, cs->svs).first;
	
	if (si->is_top_state(state)) {
		ltm = si->make_id_wme(link, cs->ltm).first;
		ltm_cmd = si->make_id_wme(ltm, cs->cmd).first;
	} else {
		ltm = NULL;
		ltm_cmd = NULL;
	}

	scene = si->make_id_wme(link, cs->scene).first;
	scene_cmd = si->make_id_wme(scene, cs->cmd).first;
	scene_contents = si->make_id_wme(scene, cs->contents).first;
	
	if (!parent) {
		sg_root = new nsg_node("world");
	} else {
		sg_root = parent->sg_root->copy();
	}
	if (!parent) {
		ptlist b1pts, b2pts;

		b1pts.push_back(vec3(0, 0, 0));
		b1pts.push_back(vec3(1, 0, 0));
		b1pts.push_back(vec3(1, 1, 0));
		b1pts.push_back(vec3(0, 1, 0));
		b1pts.push_back(vec3(0, 0, 1));
		b1pts.push_back(vec3(1, 0, 1));
		b1pts.push_back(vec3(1, 1, 1));
		b1pts.push_back(vec3(0, 1, 1));
		
		b2pts.push_back(vec3(0, 0, 1.5));
		b2pts.push_back(vec3(1, 0, 1.5));
		b2pts.push_back(vec3(1, 1, 1.5));
		b2pts.push_back(vec3(0, 1, 1.5));
		b2pts.push_back(vec3(0, 0, 2.5));
		b2pts.push_back(vec3(1, 0, 2.5));
		b2pts.push_back(vec3(1, 1, 2.5));
		b2pts.push_back(vec3(0, 1, 2.5));
		
		sg_root->attach_child(new nsg_node("block1", b1pts));
		sg_root->attach_child(new nsg_node("block2", b2pts));
	}
	wm_sg_root = new wm_sgo(si, scene_contents, (wm_sgo*) NULL, sg_root);
	update_node_name_map(sg_root, nodes);
}

svs_state::~svs_state() {
	delete sg_root; // results in wm_sg_root being deleted also
}

void svs_state::update_extract_result(sym_hnd id, cmd_data &d) {
	if (!d.fltr) {
		r = "PARSE_ERROR";
	} else if (!d.fltr->changed()) {
		return;
	} else if (!f->get_result_string(r)) {
		r = f->get_error();
	}
	if (d.result_wme) {
		si->remove_wme(d.result_wme);
	}
	d.result_wme = si->make_str_wme(id, cs->result, r);
}

void svs_state::update_generate_result(sym_hnd id, cmd_data &d) {
}

void svs_state::update_cmd_results() {
	sym_hnd a, v;
	string cmdtype;
	map<wme_hnd,cmd_data>::iterator i;

	for (i = curr_cmds.begin(); i != curr_cmds.end(); ++i) {
		a = si->get_wme_attr(i->first);
		v = si->get_wme_val(i->first);
		if (!si->get_val(a, cmdtype)) {
			continue;
		}
		if (cmdtype == "extract") {
			update_extract_result(v, i->second);
		} else if (cmdtype == "generate") {
			update_generate_result(v, i->second);
		}
	}
}

void svs_state::process_cmds() {
	map<wme_hnd,cmd_data>::iterator i;
	wme_list::iterator j;
	set<wme_hnd> all_cmds;

	new_cmds.clear();
	modified_cmds.clear();
	removed_cmds.clear();

	// populate new, modified, stuff needed for removed
	parse_cmd_id(scene_cmd, all_cmds);
	if ( ltm_cmd ) {
		parse_cmd_id(ltm_cmd, all_cmds);
	}

	// populate removed
	for (i = curr_cmds.begin(); i != curr_cmds.end(); ++i) {
		if (all_cmds.find(i->first) == all_cmds.end()) {
			removed_cmds.push_back(i->first);
			delete i->second.fltr;
		}
	}

	cout << new_cmds.size() << " NEW " << modified_cmds.size() << " MODIFIED " << removed_cmds.size() << " REMOVED" << endl;
	
	// erase removed from command map
	for (j = removed_cmds.begin(); j != removed_cmds.end(); ++j) {
		curr_cmds.erase(*j);
	}
	for (j = new_cmds.begin(); j != new_cmds.end(); ++j) {
		if ((i = curr_cmds.find(*j)) == curr_cmds.end()) {
			assert(false);
		}
		i->second.fltr = make_cmd_filter(si->get_wme_val(i->first));
	}
	for (j = modified_cmds.begin(); j != modified_cmds.end(); ++j) {
		if ((i = curr_cmds.find(*j)) == curr_cmds.end()) {
			assert(false);
		}
		delete i->second.fltr;
		i->second.fltr = make_cmd_filter(si->get_wme_val(i->first));
	}
}

inline void svs_state::parse_cmd_id(sym_hnd id, set<wme_hnd>& all_cmds) {
	map<wme_hnd,cmd_data>::iterator j;
	wme_list childs;
	wme_list::iterator i;
	wme_hnd w;
	sym_hnd v;

	si->get_child_wmes(id, childs);
	
	for (i = childs.begin(); i != childs.end(); ++i) {
		w = *i;
		v = si->get_wme_val(w);
		if (!si->is_identifier(v)) {
			continue;
		}
		
		if ((j = curr_cmds.find(w)) == curr_cmds.end()) {
			cmd_data &data = curr_cmds[w];  // inserts and initializes
			new_cmds.push_back(w);
			detect_id_changes(v, data);
		} else if (detect_id_changes(v, j->second)) {
			modified_cmds.push_back(w);
		}
	
		all_cmds.insert(w);
	}
}

inline bool svs_state::detect_id_changes(sym_hnd id, cmd_data &data) {
	tc_num tc;
	bool changed;
	stack< sym_hnd > to_process;
	wme_list childs;
	wme_list::iterator i;
	sym_hnd parent, v;
	int subtree_size = 0, tt;
	string attr;

	tc = si->new_tc_num();
	changed = false;
	
	to_process.push(id);
	while (!to_process.empty()) {
		parent = to_process.top();
		to_process.pop();
		
		si->get_child_wmes(parent, childs);
		for (i = childs.begin(); i != childs.end(); ++i) {
			if (parent == id) {
				if (si->get_val(si->get_wme_attr(*i), attr) && attr == "result") {
					/* result wmes are added by svs */
					continue;
				}
			}
			v = si->get_wme_val(*i);
			tt = si->get_timetag(*i);
			subtree_size++;
			
			if (tt > data.max_time_tag) {
				changed = true;
				data.max_time_tag = tt;
			}

			if (si->is_identifier(v) && si->get_tc_num(v) != tc) {
				si->set_tc_num(v, tc);
				to_process.push(v);
			}
		}
	}

	if (subtree_size != data.subtree_size)
	{
		changed = true;
		data.subtree_size = subtree_size;
	}

	return changed;
}

/* Filters are specified in WM like this:

   (<cmd> ^filtername1 <f1>)
   (<f1> ^arg1 <subfilter1> ^arg2 <subfilter2> ^arg3 node_name)
   (<subfilter1> ^filtername2 <f2>)
   ...
*/
filter* svs_state::make_cmd_filter(sym_hnd cmd) {
	sym_hnd v;
	string filter_name, node_name, attr;
	wme_list childs;
	wme_list::iterator i;
	wme_hnd w;
	filter_params p;
	filter *nf;
	
	/* a string symbol specifies a node filter */
	if (si->get_val(cmd, node_name)) {
		return get_node_filter(cmd);
	}
	
	si->get_child_wmes(cmd, childs);
	w = NULL;
	for (i = childs.begin(); i != childs.end(); ++i) {
		if (si->get_val(si->get_wme_attr(*i), filter_name) && filter_name != "result") {
			w = *i;
			break;
		}
	}
	if (!w) {
		return NULL;
	}

	v = si->get_wme_val(w);
	nf = get_node_filter(v);  // query wme with string value is shorthand for single node filter as param
	if (nf) {
		p.insert(pair<string,filter*>("",nf));
	} else {
		if (!si->is_identifier(v)) {
			return NULL;
		}
		if (!get_filter_params_wm(v, p)) {
			return NULL;
		}
	}
	
	return make_filter(filter_name, p);
}

bool svs_state::get_filter_params_wm(sym_hnd id, filter_params &p) {
	wme_list childs;
	wme_list::iterator i;
	filter *f;
	filter_params::iterator j;
	string attr;
	bool fail;
	
	si->get_child_wmes(id, childs);
	fail = false;
	for (i = childs.begin(); i != childs.end(); ++i) {
		if (!si->get_val(si->get_wme_attr(*i), attr)) {
			fail = true;
			break;
		}
		f = make_cmd_filter(si->get_wme_val(*i));
		if (!f) {
			fail = true;
			break;
		}
		p.insert(pair<string,filter*>(attr, f));
	}
	
	if (fail) {
		for (j = p.begin(); j != p.end(); ++j) {
			delete j->second;
		}
		p.clear();
		return false;
	}
	return true;
}

filter* svs_state::get_node_filter(sym_hnd s) {
	string name;
	node_name_map::iterator i;
	
	if (!si->get_val(s, name)) {
		return NULL;
	}
	if ((i = nodes.find(name)) == nodes.end()) {
		return NULL;
	}
	return new node_ptlist_filter(i->second);
}

svs::svs(soar_interface *interface)
: si(interface)
{
	make_common_syms();
}

svs::~svs() {
	del_common_syms();
	delete si;
}

void svs::state_creation_callback(sym_hnd state) {
	svs_state *p, *s;
	if (state_stack.empty()) {
		p = NULL;
	} else {
		p = state_stack.back();
	}
	s = new svs_state(state, p, si, &cs);
	state_stack.push_back(s);
}

void svs::state_deletion_callback(sym_hnd state) {
	svs_state *s;
	s = state_stack.back();
	assert(state == s->get_state());
	state_stack.pop_back();
	delete s;
}

void svs::pre_env_callback() {
	vector<svs_state*>::iterator i;
	for (i = state_stack.begin(); i != state_stack.end(); ++i) {
		(**i).process_cmds();
	}
}

void svs::post_env_callback() {
	vector<svs_state*>::iterator i;
	for (i = state_stack.begin(); i != state_stack.end(); ++i) {
		(**i).update_cmd_results();
	}
}

void svs::make_common_syms() {
	cs.svs      = si->make_string_sym("svs");
	cs.ltm      = si->make_string_sym("ltm");
	cs.cmd      = si->make_string_sym("command");
	cs.scene    = si->make_string_sym("spatial-scene");
	cs.contents = si->make_string_sym("contents");
	cs.child    = si->make_string_sym("child");
	cs.result   = si->make_string_sym("result");
}

void svs::del_common_syms() {
	si->del_string_sym(cs.ltm);
	si->del_string_sym(cs.cmd);
	si->del_string_sym(cs.scene);
	si->del_string_sym(cs.contents);
	si->del_string_sym(cs.child);
	si->del_string_sym(cs.result);
}

