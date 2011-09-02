#ifndef SVS_H
#define SVS_H

#include <vector>
#include <map>
#include <set>
#include "sgnode.h"
#include "ipcsocket.h"
#include "soar_interface.h"
#include "model.h"

class command;
class scene;

typedef std::map<std::string,sgnode*> node_name_map;

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
	
	Symbol *svs;
	Symbol *ltm;
	Symbol *cmd;
	Symbol *scene;
	Symbol *child;
	Symbol *result;
};

/* working memory scene graph object - mediates between wmes and scene graph nodes */
class sgwme : public sgnode_listener {
public:
	sgwme(soar_interface *si, Symbol *ident, sgwme *parent, sgnode *node);
	~sgwme();
	void node_update(sgnode *n, sgnode::change_type t, int added_child);

private:
	void add_child(sgnode *c);
	
	sgwme*          parent;
	sgnode*         node;
	Symbol         *id;
	wme            *name_wme;
	soar_interface *soarint;

	std::map<sgwme*,wme*> childs;

};

class svs;

class svs_state {
public:
	svs_state(svs *svsp, Symbol *state, soar_interface *soar, common_syms *syms);
	svs_state(Symbol *state, svs_state *parent);

	~svs_state();
	
	void           update(const std::string &msg);
	void           process_cmds();
	void           update_cmd_results(bool early);
	void           update_scene_num();
	void           clear_scene();
	
	int             get_level()          { return level;     }
	int             get_scene_num()      { return scene_num; }
	scene          *get_scene()          { return scn;       }
	Symbol         *get_state()          { return state;     }
	svs            *get_svs()            { return svsp;      }

private:
	void init();
	void collect_cmds(Symbol* id, std::set<wme*>& all_cmds);

	svs            *svsp;
	int             level;
	svs_state      *parent;
	scene          *scn;
	sgwme          *root;
	soar_interface *si;
	common_syms    *cs;
	
	Symbol *state;
	Symbol *svs_link;
	Symbol *ltm_link;
	Symbol *scene_link;
	Symbol *cmd_link;

	int scene_num;
	wme *scene_num_wme;
	
	/* command changes per decision cycle */
	std::map<wme*, command*> curr_cmds;
};

class svs {
public:
	svs(agent *a);
	~svs();
	
	void state_creation_callback(Symbol *goal);
	void state_deletion_callback(Symbol *goal);
	void pre_env_callback();
	void post_env_callback();

	soar_interface *get_soar_interface() { return si; }

	std::string get_env_input(const std::string &sgel);
	void set_next_output(const namedvec &out);

	void add_model(const std::string &name, model *m);
	bool assign_model(const std::string &name,
	                  const std::map<std::string, std::string> &inputs,
	                  const std::map<std::string, std::string> &outputs)
	{
		return models.assign_model(name, inputs, outputs);
	}

	void unassign_model(const std::string &name) {
		models.unassign_model(name);
	}
	
	multi_model *get_model() {
		return &models;
	}
	
private:
	void make_common_syms();
	void del_common_syms();
	void update_models();
	
	soar_interface*          si;
	common_syms              cs;
	std::vector<svs_state*>  state_stack;
	ipcsocket                envsock;
	namedvec                 next_out;
	std::vector<std::string> prev_pnames;
	floatvec                 prev_pvals;
	multi_model              models;
};

#endif
