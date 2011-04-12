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
#include "soar_interface.h"
#include "scene.h"
#include "env.h"
#include "common.h"

using namespace std;

typedef map<wme_hnd,command*>::iterator cmd_iter;

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

sgwme::sgwme(soar_interface *si, sym_hnd ident, sgwme *parent, sg_node *node) 
: soarint(si), id(ident), parent(parent), node(node)
{
	int i;
	node->listen(this);
	name_wme = soarint->make_wme(id, "id", node->get_name());
	for (i = 0; i < node->num_children(); ++i) {
		add_child(node->get_child(i));
	}
}

sgwme::~sgwme() {
	map<sgwme*,wme_hnd>::iterator i;

	if (node) {
		node->unlisten(this);
	}
	soarint->remove_wme(name_wme);
	for (i = childs.begin(); i != childs.end(); ++i) {
		i->first->parent = NULL;
		soarint->remove_wme(i->second);
	}
	if (parent) {
		map<sgwme*, wme_hnd>::iterator ci = parent->childs.find(this);
		assert(ci != parent->childs.end());
		soarint->remove_wme(ci->second);
		parent->childs.erase(ci);
	}
}

void sgwme::update(sg_node *n, sg_node::change_type t, int added_child) {
	switch (t) {
		case sg_node::CHILD_ADDED:
			add_child(node->get_child(added_child));
			break;
		case sg_node::DETACHED:
		case sg_node::DELETED:
			node = NULL;
			delete this;
			break;
	};
}

void sgwme::add_child(sg_node *c) {
	sym_wme_pair cid_wme;
	char letter;
	string cname = c->get_name();
	sgwme *child;
	
	if (cname.size() == 0 || !isalpha(cname[0])) {
		letter = 'n';
	} else {
		letter = cname[0];
	}
	cid_wme = soarint->make_id_wme(id, "child");
	
	child = new sgwme(soarint, cid_wme.first, this, c);
	childs[child] = cid_wme.second;
}

svs_state::svs_state(svs *svsp, sym_hnd state, soar_interface *si, common_syms *syms)
: svsp(svsp), parent(NULL), state(state), si(si), cs(syms), level(0),
  scene_num(-1), scene_num_wme(NULL), scn(NULL), scene_link(NULL),
  ltm_link(NULL)
{
	assert (si->is_top_state(state));
	init();
}

svs_state::svs_state(sym_hnd state, svs_state *parent)
: parent(parent), state(state), svsp(parent->svsp), si(parent->si), cs(parent->cs),
  level(parent->level+1), scene_num(-1),
  scene_num_wme(NULL), scn(NULL), scene_link(NULL), ltm_link(NULL)
{
	assert (si->get_parent_state(state) == parent->state);
	init();
}

svs_state::~svs_state() {
	delete scn; // results in root being deleted also
}

void svs_state::init() {
	string name;
	
	si->get_name(state, name);
	svs_link = si->make_id_wme(state, cs->svs).first;
	cmd_link = si->make_id_wme(svs_link, cs->cmd).first;
	scene_link = si->make_id_wme(svs_link, cs->scene).first;
	scn = new scene(name, "world");
	root = new sgwme(si, scene_link, (sgwme*) NULL, scn->get_root());
	if (!parent) {
		ltm_link = si->make_id_wme(svs_link, cs->ltm).first;
	}
}

void svs_state::update(const string &msg) {
	size_t p = msg.find_first_of('\n');
	int n;
	
	if (sscanf(msg.c_str(), "%d", &n) != 1) {
		perror("svs_state::update");
		cerr << msg << endl;
		exit(1);
	}
	scene_num = n;
	update_scene_num();
	scn->parse_sgel(msg.substr(p+1));
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
	if (scene_num >= 0) {
		scene_num_wme = si->make_wme(svs_link, "scene-num", scene_num);
	}
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
		command *c = make_command(this, *i);
		if (c) {
			curr_cmds.insert(make_pair(*i, c));
		}
	}
}

void svs_state::clear_scene() {
	scn->clear();
}

svs::svs(agent *a)
: env(getnamespace() + "env")
{
	si = new soar_interface(a);
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
		s = new svs_state(this, state, si, &cs);
	} else {
		s = new svs_state(state, state_stack.back());
	}
	
	state_stack.push_back(s);
	
}

void svs::state_deletion_callback(sym_hnd state) {
	string resp;
	svs_state *s;
	s = state_stack.back();
	assert(state == s->get_state());
	state_stack.pop_back();
	delete s;
}

void svs::pre_env_callback() {
	vector<svs_state*>::iterator i;
	string sgel;
	
	for (i = state_stack.begin(); i != state_stack.end(); ++i) {
		(**i).process_cmds();
	}
	for (i = state_stack.begin(); i != state_stack.end(); ++i) {
		(**i).update_cmd_results(true);
	}
	env.input(sgel);
	state_stack.front()->get_scene()->parse_sgel(sgel);
}

void svs::post_env_callback() {
	string resp;
	vector<svs_state*>::iterator i;
	for (i = state_stack.begin(); i != state_stack.end(); ++i) {
		(**i).update_cmd_results(false);
	}
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

string svs::get_env_input(const string &sgel) {
	return "";
}
