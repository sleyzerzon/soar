#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <iterator>
#include <utility>
#include <algorithm>

#include "svs.h"
#include "cmd_watcher.h"
#include "nsg_node.h"
#include "wm_sgo.h"
#include "soar_int.h"
#include "scene.h"

using namespace std;

typedef map<wme_hnd,cmd_watcher*>::iterator cmd_iter;

extern int parse_state_update(string msg, scene *scn);

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

svs_state::svs_state(sym_hnd state, soar_interface *soar, ipcsocket *ipc, common_syms *syms)
: parent(NULL), state(state), si(soar), cs(syms), ipc(ipc), level(0), scene_num(-1), scene_num_wme(NULL)
{
	string type, msg;
	
	assert (si->is_top_state(state));
	
	init();
	ipc->receive(type, msg);
	assert(type == "initscene");
	update(msg);
	
	ipc->receive(type, msg);
	cout << type << endl << msg << endl;
	assert(type == "stateupdate");
	update(msg);
}

svs_state::svs_state(sym_hnd state, svs_state *parent)
: parent(parent), state(state), si(parent->si), 
  cs(parent->cs), ipc(parent->ipc), level(parent->level+1), 
  scene_num(parent->scene_num), scene_num_wme(NULL)
{
	
	assert (si->get_parent_state(state) == parent->state);
	
	init();
	update_scene_num();
	ipc->send("newstate",level, "");
}

svs_state::~svs_state() {
	delete scn; // results in wm_sg_root being deleted also
}

void svs_state::init() {
	string name;
	
	si->get_name(state, name);
	svs_link = si->make_id_wme(state, cs->svs).first;
	if (parent) {
		level = parent->level + 1;
		scn = new scene(name, parent->scn);
		ltm_link = NULL;
	} else {
		level = 0;
		scn = new scene(name, "world", true);
		ltm_link = si->make_id_wme(svs_link, cs->ltm).first;
	}
	scene_link = si->make_id_wme(svs_link, cs->scene).first;
	cmd_link = si->make_id_wme(svs_link, cs->cmd).first;
	
	wm_sg_root = new wm_sgo(si, scene_link, (wm_sgo*) NULL, scn->get_root());
	
}

void svs_state::update(const string &msg) {
	size_t p = msg.find_first_of('\n');
	int n;
	
	if (sscanf(msg.c_str(), "%d", &n) != 1) {
		perror("svs_state::update");
		cout << msg << endl;
		exit(1);
	}
	scene_num = n;
	update_scene_num();
	parse_state_update(msg.substr(p+1), scn);
}

void svs_state::update_scene_num() {
	long curr;
	if (scene_num_wme) {
		if (!si->get_val(si->get_wme_val(scene_num_wme), curr)) {
			exit(1);
		}
		if (curr == scene_num) {
			return;
		}
		si->remove_wme(scene_num_wme);
	}
	scene_num_wme = si->make_wme(scene_link, "scene_num", scene_num);
}

void svs_state::update_cmd_results(bool early) {
	cmd_iter i;
	for (i = curr_cmds.begin(); i != curr_cmds.end(); ++i) {
		if (i->second->early() == early) {
			i->second->update_result();
		}
	}
}

void svs_state::process_cmds() {
	wme_list all;
	wme_list::iterator i;
	cmd_iter j;

	si->get_child_wmes(cmd_link, all);

	for (j = curr_cmds.begin(); j != curr_cmds.end(); ) {
		if ((i = find(all.begin(), all.end(), j->first)) == all.end()) {
			delete j->second;
			curr_cmds.erase(j++);
		} else {
			all.erase(i);
			++j;
		}
	}
	
	// all now contains only new commands
	for (i = all.begin(); i != all.end(); ++i) {
		cmd_watcher *cw = make_cmd_watcher(this, *i);
		if (cw) {
			curr_cmds.insert(make_pair(*i, cw));
		}
	}
}

svs::svs(soar_interface *interface)
: si(interface), ipc("/tmp/svsipc")
{
	make_common_syms();
}

svs::~svs() {
	del_common_syms();
	delete si;
}

void svs::state_creation_callback(sym_hnd state) {
	string type, msg;
	svs_state *s;
	
	if (state_stack.empty()) {
		s = new svs_state(state, si, &ipc, &cs);
	} else {
		s = new svs_state(state, state_stack.back());
	}
	
	state_stack.push_back(s);
	
}

void svs::state_deletion_callback(sym_hnd state) {
	svs_state *s;
	s = state_stack.back();
	assert(state == s->get_state());
	state_stack.pop_back();
	ipc.send("delstate", s->get_level(), "");
	delete s;
}

void svs::pre_env_callback() {
	vector<svs_state*>::iterator i;
	for (i = state_stack.begin(); i != state_stack.end(); ++i) {
		(**i).process_cmds();
	}
	for (i = state_stack.begin(); i != state_stack.end(); ++i) {
		(**i).update_cmd_results(true);
	}
}

void svs::post_env_callback() {
	vector<svs_state*>::iterator i;
	for (i = state_stack.begin(); i != state_stack.end(); ++i) {
		(**i).update_cmd_results(false);
	}
	//print_tree(state_stack.back()->get_scene()->get_root());
}

void svs::make_common_syms() {
	cs.svs        = si->make_sym("svs");
	cs.ltm        = si->make_sym("ltm");
	cs.cmd        = si->make_sym("command");
	cs.scene      = si->make_sym("spatial-scene");
	cs.child      = si->make_sym("child");
	cs.result     = si->make_sym("result");
}

void svs::del_common_syms() {
	si->del_sym(cs.ltm);
	si->del_sym(cs.cmd);
	si->del_sym(cs.scene);
	si->del_sym(cs.child);
	si->del_sym(cs.result);
}

int svs::get_env_input(const string &line) {
	return 0;
}

