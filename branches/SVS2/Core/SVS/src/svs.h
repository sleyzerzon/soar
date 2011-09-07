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

/*
 Description of a single output dimension.
*/
struct output_dim_spec {
	std::string name;
	float min;
	float max;
	float def;
};

typedef std::vector<output_dim_spec> output_spec;

/*
 Each state in the state stack has its own SVS link, scene, models, etc.
*/
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
	
	int            get_level() const     { return level;     }
	int            get_scene_num() const { return scene_num; }
	scene         *get_scene() const     { return scn;       }
	Symbol        *get_state()           { return state;     }
	svs           *get_svs()             { return svsp;      }
	multi_model   *get_model()           { return &mmdl;     }
	
	output_spec *get_output_spec() { return &outspec; }
	void set_output_spec(const output_spec &s) { outspec = s; }
	
	void set_output(const floatvec &out);
	bool get_output(floatvec &out) const;
	
	void update_models();

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
	
	std::vector<std::string> prev_pnames;
	floatvec                 prev_pvals;
	multi_model              mmdl;
	output_spec              outspec;
	floatvec                 next_out;
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

private:
	void make_common_syms();
	void del_common_syms();
	
	soar_interface*          si;
	common_syms              cs;
	std::vector<svs_state*>  state_stack;
	ipcsocket                envsock;
};

#endif
