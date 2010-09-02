#ifndef SVS_H
#define SVS_H

#include "soar_int.h"
#include "sg_node.h"
#include "scene.h"
#include "wm_sgo.h"
#include "filter.h"
#include "cmd_watcher.h"
#include "sgel_interp.h"
#include <vector>
#include <map>
#include <set>

typedef std::map<std::string,sg_node*> node_name_map;

class common_syms {
public:
	common_syms() {
		svs      = NULL;
		ltm      = NULL;
		cmd      = NULL;
		scene    = NULL;
		contents = NULL;
		child    = NULL;
		result   = NULL;
	}
	
	sym_hnd svs;
	sym_hnd ltm;
	sym_hnd cmd;
	sym_hnd scene;
	sym_hnd contents;
	sym_hnd child;
	sym_hnd result;
};

class svs_state {
public:
	svs_state(sym_hnd state, svs_state *parent, soar_interface *interface, common_syms *syms);
	~svs_state();
	
	void    process_cmds();
	void    update_cmd_results();
	scene*  get_scene();
	sym_hnd get_state() { return state; }
	
private:
	void collect_cmds(Symbol* id, std::set<wme*>& all_cmds);
	
	soar_interface *si;

	scene*        scn;
	wm_sgo*       wm_sg_root;

	common_syms *cs;
	
	/* svs link identifiers */
	sym_hnd state;
	sym_hnd svs_link;
	sym_hnd ltm_link;
	sym_hnd ltm_cmd_link;
	sym_hnd scene_link;
	sym_hnd scene_cmd_link;
	sym_hnd scene_contents_link;

	/* command changes per decision cycle */
	wme_list new_cmds;
	wme_list removed_cmds;
	std::map<wme_hnd, cmd_watcher*> curr_cmds;
};

class svs {
public:
	svs(soar_interface *soarinterface);
	~svs();
	
	void state_creation_callback(sym_hnd goal);
	void state_deletion_callback(sym_hnd goal);
	void pre_env_callback();
	void post_env_callback();

	int  get_env_input(const std::string &line);
	
private:
	void make_common_syms();
	void del_common_syms();
	
	soar_interface*         si;
	common_syms             cs;
	std::vector<svs_state*> state_stack;
	
	sgel_interp* input_interp;
};

#endif
