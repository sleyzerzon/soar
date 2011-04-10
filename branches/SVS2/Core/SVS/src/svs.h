#ifndef SVS_H
#define SVS_H

#include <vector>
#include <map>
#include <set>
#include "sg_node.h"
#include "ipcsocket.h"
#include "soar_interface.h"
#include "env.h"
#include "model.h"

class cmd_watcher;
class scene;

typedef std::map<std::string,sg_node*> node_name_map;

class common_syms {
public:
	common_syms() {
		svs        = NULL;
		ltm        = NULL;
		scene      = NULL;
		cmd        = NULL;
		child      = NULL;
		result     = NULL;
	}
	
	sym_hnd svs;
	sym_hnd ltm;
	sym_hnd cmd;
	sym_hnd scene;
	sym_hnd child;
	sym_hnd result;
};

/* working memory scene graph object - mediates between wmes and scene graph nodes */
class sgwme : public sg_listener {
public:
	sgwme(soar_interface *si, sym_hnd ident, sgwme *parent, sg_node *node);
	~sgwme();
	void update(sg_node *n, sg_node::change_type t, int added_child);

private:
	void add_child(sg_node *c);
	
	sgwme*          parent;
	sg_node*        node;
	sym_hnd         id;
	wme_hnd         name_wme;
	soar_interface* soarint;

	std::map<sgwme*,wme_hnd> childs;

};

class svs;

class svs_state {
public:
	svs_state(svs *svsp, sym_hnd state, soar_interface *soar, common_syms *syms);
	svs_state(sym_hnd state, svs_state *parent);

	~svs_state();
	
	void           update(const std::string &msg);
	void           process_cmds();
	void           update_cmd_results(bool early);
	void           update_scene_num();
	void           clear_scene();
	
	int            get_level()           { return level;     }
	int            get_scene_num()       { return scene_num; }
	scene          *get_scene()          { return scn;       }
	sym_hnd        get_state()           { return state;     }
	soar_interface *get_soar_interface() { return si;        }
	svs            *get_svs()            { return svsp;       }

private:
	void init();
	void collect_cmds(Symbol* id, std::set<wme*>& all_cmds);

	svs            *svsp;
	int            level;
	svs_state      *parent;
	scene          *scn;
	sgwme          *root;
	soar_interface *si;
	ipcsocket      *ipc;
	common_syms    *cs;
	
	sym_hnd state;
	sym_hnd svs_link;
	sym_hnd ltm_link;
	sym_hnd scene_link;
	sym_hnd cmd_link;

	int scene_num;
	wme_hnd scene_num_wme;
	
	/* command changes per decision cycle */
	std::map<wme_hnd, cmd_watcher*> curr_cmds;
};

class svs {
public:
	svs(agent *a);
	~svs();
	
	void state_creation_callback(sym_hnd goal);
	void state_deletion_callback(sym_hnd goal);
	void pre_env_callback();
	void post_env_callback();

	model *get_model() { return mdl; }
	
	environment *get_env() { return &env; }

	std::string get_env_input(const std::string &sgel);

private:
	void make_common_syms();
	void del_common_syms();
	
	soar_interface*         si;
	common_syms             cs;
	std::vector<svs_state*> state_stack;
	environment             env;
	model                   *mdl;
};

#endif
