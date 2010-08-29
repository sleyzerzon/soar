#ifndef SVS_H
#define SVS_H

#include "sg_node.h"
#include "soar_int.h"
#include "wm_sgo.h"
#include "sgel_interp.h"
#include <list>

typedef std::map<std::string,sg_node*> node_name_map;

class cmd_stats {
public:
	cmd_stats() 
	: subtree_size(0), max_time_tag(0)
	{}
	
	int subtree_size;
	int max_time_tag;
};

typedef struct common_syms_struct {
	sym_hnd svs_sym;
	sym_hnd ltm_sym;
	sym_hnd cmd_sym;
	sym_hnd scene_sym;
	sym_hnd contents_sym;
	sym_hnd child_sym;
} common_syms;

class svs_state {
public:
	svs_state(sym_hnd state, svs_state *parent, soar_interface *interface, common_syms *cs);
	~svs_state();
	
	void    process_cmds();
	sym_hnd get_state() { return state; }

private:
	void parse_cmd_id      (Symbol* id, std::set<wme*>& all_cmds);
	bool detect_id_changes (Symbol* id, cmd_stats &stats);
	
	soar_interface *si;

	sg_node*      sg_root;
	wm_sgo*       wm_sg_root;
	node_name_map nodes;

	/* svs link identifiers */
	sym_hnd state;
	sym_hnd link;
	sym_hnd ltm;
	sym_hnd ltm_cmd;
	sym_hnd scene;
	sym_hnd scene_cmd;
	sym_hnd scene_contents;

	/* command changes per decision cycle */
	std::list<wme_hnd>          new_cmds;
	std::list<wme_hnd>          modified_cmds;
	std::list<wme_hnd>          removed_cmds;
	std::map<wme_hnd,cmd_stats> curr_cmds;
};

class svs {
public:
	svs(soar_interface *soarinterface);
	~svs();
	
	void state_creation_callback(sym_hnd goal);
	void state_deletion_callback(sym_hnd goal);
	void pre_env_callback();
	void post_env_callback();

private:
	void make_common_syms();
	void del_common_syms();
	
	soar_interface*       si;
	std::list<svs_state*> state_stack;

	common_syms cs;
};

#endif
