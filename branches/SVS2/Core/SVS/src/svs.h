#ifndef SVS_H
#define SVS_H

#include "sg_node.h"
#include "soar_int.h"
#include "wm_sgo.h"
#include "filter.h"
#include <vector>

typedef std::map<std::string,sg_node*> node_name_map;


class cmd_data {
public:
	cmd_data() 
	: subtree_size(0), max_time_tag(0), fltr(NULL), result_wme(NULL)
	{}
	
	int         subtree_size;
	int         max_time_tag;
	filter*     fltr;
	wme_hnd     result_wme;
};

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
	sym_hnd get_state() { return state; }
	
private:
	void    parse_cmd_id(Symbol* id, std::set<wme*>& all_cmds);
	bool    detect_id_changes(Symbol* id, cmd_data &data);

	filter* make_cmd_filter(sym_hnd cmd);
	bool    get_filter_params_wm(sym_hnd id, filter_params &p);
    filter* get_node_filter(sym_hnd s);
    
    void    update_extract_result(sym_hnd id, cmd_data &d);
	void    update_generate_result(sym_hnd id, cmd_data &d);
	
	soar_interface *si;

	sg_node*      sg_root;
	wm_sgo*       wm_sg_root;
	node_name_map nodes;

	common_syms *cs;
	
	/* svs link identifiers */
	sym_hnd state;
	sym_hnd link;
	sym_hnd ltm;
	sym_hnd ltm_cmd;
	sym_hnd scene;
	sym_hnd scene_cmd;
	sym_hnd scene_contents;

	/* command changes per decision cycle */
	wme_list new_cmds;
	wme_list modified_cmds;
	wme_list removed_cmds;
	std::map<wme_hnd,cmd_data> curr_cmds;
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
	
	soar_interface*         si;
	common_syms             cs;
	std::vector<svs_state*> state_stack;

};

#endif
