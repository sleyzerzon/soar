#include <iostream>
#include <assert.h>
#include "svs.h"
#include "nsg_node.h"
#include "wm_sgo.h"
#include "soar_int.h"

#include <iterator>

using namespace std;

typedef map<wme_hnd,cmd_watcher*>::iterator cmd_iter;

void print_tree(sg_node *n) {
	if (n->is_group()) {
		for(int i = 0; i < n->num_children(); ++i) {
			print_tree(n->get_child(i));
		}
	} else {
		ptlist pts;
		n->get_world_points(pts);
		copy(pts.begin(), pts.end(), ostream_iterator<vec3>(cout, ","));
		cout << endl;
	}
}

svs_state::svs_state(sym_hnd state, svs_state *parent, soar_interface *interface, common_syms *syms)
: state(state), si(interface), cs(syms)
{
	svs_link = si->make_id_wme(state, cs->svs).first;
	
	if (si->is_top_state(state)) {
		ltm_link = si->make_id_wme(svs_link, cs->ltm).first;
		ltm_cmd_link = si->make_id_wme(ltm_link, cs->cmd).first;
	} else {
		ltm_link = NULL;
		ltm_cmd_link = NULL;
	}

	scene_link = si->make_id_wme(svs_link, cs->scene).first;
	scene_cmd_link = si->make_id_wme(scene_link, cs->cmd).first;
	scene_contents_link = si->make_id_wme(scene_link, cs->contents).first;
	
	if (!parent) {
		scn = new scene("world");
	} else {
		scn = new scene(parent->scn);
	}
	/*
	if (!parent) {
		ptlist pts;

		pts.push_back(vec3(0, 0, 0));
		pts.push_back(vec3(1, 0, 0));
		pts.push_back(vec3(1, 1, 0));
		pts.push_back(vec3(0, 1, 0));
		pts.push_back(vec3(0, 0, 1));
		pts.push_back(vec3(1, 0, 1));
		pts.push_back(vec3(1, 1, 1));
		pts.push_back(vec3(0, 1, 1));
		
		sg_node *n1, *n2;
		n1 = new nsg_node("block1", pts);
		n1->set_pos(vec3(0, 0, 0));
		
		n2 = new nsg_node("block2", pts);
		n2->set_pos(vec3(5, 0, 0));
		
		scn->get_root()->attach_child(n1);
		scn->get_root()->attach_child(n2);
	}
	*/
	wm_sg_root = new wm_sgo(si, scene_contents_link, (wm_sgo*) NULL, scn->get_root());
}

svs_state::~svs_state() {
	delete scn; // results in wm_sg_root being deleted also
}

scene* svs_state::get_scene() {
	return scn;
}

void svs_state::update_cmd_results() {
	cmd_iter i;

	for (i = curr_cmds.begin(); i != curr_cmds.end(); ++i) {
		i->second->update_result();
	}
}

void svs_state::process_cmds() {
	set<wme_hnd> all_cmds;
	cmd_iter i;
	set<wme_hnd>::iterator j;

	collect_cmds(scene_cmd_link, all_cmds);
	if ( ltm_cmd_link ) {
		collect_cmds(ltm_cmd_link, all_cmds);
	}

	for (i = curr_cmds.begin(); i != curr_cmds.end(); ) {
		if ((j = all_cmds.find(i->first)) == all_cmds.end()) {
			delete i->second;
			curr_cmds.erase(i++);
		} else {
			all_cmds.erase(j);
			++i;
		}
	}
	
	// all_cmds now contains only new commands
	for (j = all_cmds.begin(); j != all_cmds.end(); ++j) {
		cmd_watcher *cw = make_cmd_watcher(si, scn, *j);
		if (cw) {
			curr_cmds.insert(pair<wme_hnd,cmd_watcher*>(*j, cw));
		}
	}
}

inline void svs_state::collect_cmds(sym_hnd id, set<wme_hnd>& all_cmds) {
	wme_list childs;
	wme_list::iterator i;

	si->get_child_wmes(id, childs);
	
	for (i = childs.begin(); i != childs.end(); ++i) {
		all_cmds.insert(*i);
	}
}

svs::svs(soar_interface *interface)
: si(interface), input_interp(NULL)
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
	if (!input_interp) {
		input_interp = new sgel_interp(s->get_scene());
	}
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
	print_tree(state_stack.back()->get_scene()->get_root());
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

int svs::get_env_input(const string &line) {
	if (input_interp) {
		return input_interp->parse_line(line);
	} else {
		assert(false);
		return 0;
	}
}
