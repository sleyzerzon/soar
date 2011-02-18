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
#include "common.h"

using namespace std;

typedef map<wme_hnd,cmd_watcher*>::iterator cmd_iter;

ipcsocket *disp = NULL;

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

bool usedisplay() {
	return (getenv("SVSDISPLAY") != NULL);
}

string getnamespace() {
	char *s;
	if ((s = getenv("SVSNAMESPACE")) == NULL) {
		return "";
	}
	
	string ns(s);
	if (ns.size() > 0 && *ns.rbegin() != '/') {
		ns.push_back('/');
	}
	return ns;
}

wmsgo::wmsgo(soar_interface *si, sym_hnd ident, wmsgo *parent, sg_node *node) 
: soarint(si), id(ident), parent(parent), node(node)
{
	int i;
	node->listen(this);
	name_wme = soarint->make_wme(id, "id", node->get_name());
	for (i = 0; i < node->num_children(); ++i) {
		add_child(node->get_child(i));
	}
}

wmsgo::~wmsgo() {
	map<wmsgo*,wme_hnd>::iterator i;

	if (node) {
		node->unlisten(this);
	}
	soarint->remove_wme(name_wme);
	for (i = childs.begin(); i != childs.end(); ++i) {
		i->first->parent = NULL;
		soarint->remove_wme(i->second);
	}
	if (parent) {
		map<wmsgo*, wme_hnd>::iterator ci = parent->childs.find(this);
		assert(ci != parent->childs.end());
		soarint->remove_wme(ci->second);
		parent->childs.erase(ci);
	}
}

void wmsgo::update(sg_node *n, sg_node::change_type t) {
	switch (t) {
		case sg_node::CHILD_ADDED:
			add_child(node->get_child(node->num_children()-1));
			break;
		case sg_node::DETACHED:
		case sg_node::DELETED:
			node = NULL;
			delete this;
			break;
	};
}

wmsgo* wmsgo::add_child(sg_node *c) {
	sym_wme_pair cid_wme;
	char letter;
	string cname = c->get_name();
	wmsgo *child;
	
	if (cname.size() == 0 || !isalpha(cname[0])) {
		letter = 'n';
	} else {
		letter = cname[0];
	}
	cid_wme = soarint->make_id_wme(id, "child");
	
	child = new wmsgo(soarint, cid_wme.first, this, c);
	childs[child] = cid_wme.second;
	return child;
}


svs_stats::svs_stats(sym_hnd svs_link, soar_interface *si) 
: svs_link(svs_link), si(si), currmodel(NULL)
{
	stats_link = si->make_id_wme(svs_link, "stats").first;
	models_link = si->make_id_wme(stats_link, "models").first;
}

/* First line is current model number. Subsequent lines have form
   <model num> <stat>:<val> <stat>:<val> ..
 */
void svs_stats::update(const string &msg) {
	vector<string> lines, fields, stat;
	vector<string>::iterator i, j;
	map<string, wme_hnd>::iterator wi;
	map<pair<sym_hnd,string>, wme_hnd>::iterator wj;
	sym_wme_pair tmp;
	pair<sym_hnd, string> statkey;
	const char *b;
	char *e;
	int intval;
	double floatval;
	wme_hnd w;
	sym_hnd modelid;
	
	split(msg, "\n", lines);
	if (currmodel != NULL) {
		si->remove_wme(currmodel);
	}
	currmodel = si->make_wme(stats_link, "current-model", lines[0]);
	
	for (i = lines.begin() + 1; i != lines.end(); ++i) {
		split(*i, " \t", fields);
		assert(fields.size() > 1);
		wi = modelwmes.find(fields[0]);
		if (wi == modelwmes.end()) {
			tmp = si->make_id_wme(models_link, fields[0]);
			modelid = tmp.first;
			modelwmes[fields[0]] = tmp.second;
		} else {
			modelid = si->get_wme_val(wi->second);
		}
		for (j = fields.begin() + 1 ; j != fields.end(); ++j) {
			split(*j, ":", stat);
			statkey = make_pair(modelid, stat[0]);
			wj = statwmes.find(statkey);
			if (wj != statwmes.end()) {
				si->remove_wme(wj->second);
			}
			b = stat[1].c_str();
			intval = strtol(b, &e, 10);
			if (*e == '\0') {
				w = si->make_wme(modelid, stat[0], intval);
			} else {
				floatval = strtod(b, &e);
				if (*e == '\0') {
					w = si->make_wme(modelid, stat[0], floatval);
				} else {
					w = si->make_wme(modelid, stat[0], stat[1]);
				}
			}
			statwmes[statkey] = w;
		}
	}
}

svs_state::svs_state(sym_hnd state, soar_interface *soar, ipcsocket *ipc, common_syms *syms)
: parent(NULL), state(state), si(soar), cs(syms), ipc(ipc), level(0),
  scene_num(-1), scene_num_wme(NULL), scn(NULL), scene_link(NULL),
  ltm_link(NULL), stats(NULL)
{
	string resp;
	
	assert (si->is_top_state(state));
	
	init();
	if (ipc->communicate("initscene", 0, "", resp) == "error") {
		assert(false);
	}
	update(resp);
}

svs_state::svs_state(sym_hnd state, svs_state *parent)
: parent(parent), state(state), si(parent->si), cs(parent->cs),
  ipc(parent->ipc), level(parent->level+1), scene_num(-1),
  scene_num_wme(NULL), scn(NULL), scene_link(NULL), ltm_link(NULL),
  stats(NULL)
{
	
	assert (si->get_parent_state(state) == parent->state);
	init();
}

svs_state::~svs_state() {
	delete scn; // results in wm_sg_root being deleted also
}

void svs_state::init() {
	string name;
	
	si->get_name(state, name);
	svs_link = si->make_id_wme(state, cs->svs).first;
	cmd_link = si->make_id_wme(svs_link, cs->cmd).first;
	scene_link = si->make_id_wme(svs_link, cs->scene).first;
	if (disp == NULL && usedisplay()) {
		disp = new ipcsocket(getnamespace() + "disp");
	}
	scn = new scene(name, "world", disp);
	wm_sg_root = new wmsgo(si, scene_link, (wmsgo*) NULL, scn->get_root());
	if (!parent) {
		ltm_link = si->make_id_wme(svs_link, cs->ltm).first;
		stats = new svs_stats(svs_link, si);
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
	scn->update_sgel(msg.substr(p+1));
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
		cmd_watcher *cw = make_cmd_watcher(this, *i);
		if (cw) {
			curr_cmds.insert(make_pair(*i, cw));
		}
	}
}

void svs_state::wipe_scene() {
	scn->wipe();
}

void svs_state::update_stats(const string &msg) {
	if (stats) {
		stats->update(msg);
	}
}

svs::svs(agent *a)
: ipc(getnamespace() + "ctrl")
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
		s = new svs_state(state, si, &ipc, &cs);
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
	if (ipc.communicate("delstate", s->get_level(), "", resp) == "error") {
		cout << resp << endl;
		exit(1);
	}
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
	string resp;
	vector<svs_state*>::iterator i;
	for (i = state_stack.begin(); i != state_stack.end(); ++i) {
		(**i).update_cmd_results(false);
	}
	if (ipc.communicate("updatestats", 0, "", resp) == "error") {
		assert(false);
	}
	state_stack[0]->update_stats(resp);
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

