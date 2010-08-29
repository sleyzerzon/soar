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

svs_state::svs_state(sym_hnd state_id, svs_state *parent, soar_interface *interface, common_syms *cs)
: state(state_id), si(interface)
{
	link = si->make_id_wme(state, cs->svs_sym).first;
	
	if (si->is_top_state(state)) {
		ltm = si->make_id_wme(link, cs->ltm_sym).first;
		ltm_cmd = si->make_id_wme(ltm, cs->cmd_sym).first;
	}

	scene = si->make_id_wme(link, cs->scene_sym).first;
	scene_cmd = si->make_id_wme(scene, cs->cmd_sym).first;
	scene_contents = si->make_id_wme(scene, cs->contents_sym).first;
	
	if (!parent) {
		sg_root = new nsg_node("world");
	} else {
		sg_root = parent->sg_root->copy();
	}
	wm_sg_root = new wm_sgo(si, scene_contents, (wm_sgo*) NULL, sg_root);
	update_node_name_map(sg_root, nodes);
}

svs_state::~svs_state() {
	delete sg_root; // results in wm_sg_root being deleted also
}

void svs_state::process_cmds() {
	map<wme_hnd,cmd_stats>::iterator i;
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
		}
	}

	// erase removed from command map
	for (j = removed_cmds.begin(); j != removed_cmds.end(); ++j) {
		curr_cmds.erase(*j);
	}
}

inline void svs_state::parse_cmd_id(sym_hnd id, set<wme_hnd>& all_cmds) {
	map<wme_hnd,cmd_stats>::iterator j;
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
			cmd_stats &stats = curr_cmds[w];  // inserts and initializes
			new_cmds.push_back(w);
			detect_id_changes(v, stats);
		} else if (detect_id_changes(v, j->second)) {
			modified_cmds.push_back(w);
		}
	
		all_cmds.insert(w);
	}
}

inline bool svs_state::detect_id_changes(sym_hnd id, cmd_stats &stats) {
	tc_num tc;
	bool changed;
	stack< sym_hnd > to_process;
	wme_list childs;
	wme_list::iterator i;
	sym_hnd parent, v;
	int subtree_size = 0, tt;

	tc = si->new_tc_num();
	changed = false;
	
	to_process.push(id);
	while (!to_process.empty()) {
		parent = to_process.top();
		to_process.pop();
		
		si->get_child_wmes(parent, childs);
		for (i = childs.begin(); i != childs.end(); ++i) {
			v = si->get_wme_val(*i);
			tt = si->get_timetag(*i);
			subtree_size++;
			
			if (tt > stats.max_time_tag) {
				changed = true;
				stats.max_time_tag = tt;
			}

			if (si->is_identifier(v) && si->get_tc_num(v) != tc) {
				si->set_tc_num(v, tc);
				to_process.push(v);
			}
		}
	}

	if (subtree_size != stats.subtree_size)
	{
		changed = true;
		stats.subtree_size = subtree_size;
	}

	return changed;
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
}

void svs::post_env_callback() {
}

void svs::make_common_syms() {
	cs.svs_sym      = si->make_string_sym("svs");
	cs.ltm_sym      = si->make_string_sym("ltm");
	cs.cmd_sym      = si->make_string_sym("command");
	cs.scene_sym    = si->make_string_sym("spatial-scene");
	cs.contents_sym = si->make_string_sym("contents");
	cs.child_sym    = si->make_string_sym("child");
}

void svs::del_common_syms() {
	si->del_string_sym(cs.ltm_sym);
	si->del_string_sym(cs.cmd_sym);
	si->del_string_sym(cs.scene_sym);
	si->del_string_sym(cs.contents_sym);
	si->del_string_sym(cs.child_sym);
}

