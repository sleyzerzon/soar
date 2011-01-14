#ifndef SVS_H
#define SVS_H

#include <vector>
#include <map>
#include <set>
#include "ipcsocket.h"
#include "soar_int.h"

class cmd_watcher;
class wm_sgo;
class scene;
class sg_node;

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

class svs_stats {
public:
	svs_stats(sym_hnd svs_link, soar_interface *si);
	void update(const std::string &msg);

private:
	sym_hnd                                          svs_link;
	sym_hnd                                          stats_link;
	sym_hnd                                          models_link;
	soar_interface                                   *si;
	wme_hnd                                          currmodel;
	std::map<std::string,wme_hnd>                    modelwmes;
	std::map<std::pair<sym_hnd,std::string>,wme_hnd> statwmes;
};

class svs_state {
public:
	svs_state(sym_hnd state, soar_interface *soar, ipcsocket *ipc, common_syms *syms);
	svs_state(sym_hnd state, svs_state *parent);

	~svs_state();
	
	void           update(const std::string &msg);
	void           process_cmds();
	void           update_cmd_results(bool early);
	void           update_scene_num();
	void           wipe_scene();
	void           update_stats(const std::string &msg);
	
	int            get_level()           { return level;     }
	int            get_scene_num()       { return scene_num; }
	scene          *get_scene()          { return scn;       }
	sym_hnd        get_state()           { return state;     }
	soar_interface *get_soar_interface() { return si;        }
	ipcsocket      *get_ipc()            { return ipc;       }

private:
	void init();
	void collect_cmds(Symbol* id, std::set<wme*>& all_cmds);
	
	svs_state      *parent;
	int            level;
	scene*         scn;
	wm_sgo*        wm_sg_root;
	soar_interface *si;
	ipcsocket      *ipc;
	common_syms    *cs;
	svs_stats      *stats;
	
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
	ipcsocket               ipc;
};

#endif
