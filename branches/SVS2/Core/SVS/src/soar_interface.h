#ifndef SOAR_INTERFACE_H
#define SOAR_INTERFACE_H

#include <list>
#include <map>
#include <set>
#include "portability.h"
#include "agent.h"

typedef wme*    wme_hnd;
typedef Symbol* sym_hnd;

typedef std::pair<sym_hnd, wme_hnd> sym_wme_pair;
typedef std::list<wme_hnd> wme_list;

class cmd_stats {
public:
	cmd_stats() 
	: subtree_size(0), max_time_tag(0)
	{}
	
	int subtree_size;
	int max_time_tag;
};

typedef struct goal_info_struct {
	Symbol* state;
	Symbol* link;
	Symbol* ltm;
	Symbol* ltm_cmd;
	Symbol* scene;
	Symbol* scene_cmd;
	Symbol* scene_contents;
	
	/* command changes per decision cycle */
	std::list<wme*>          new_cmds;
	std::list<wme*>          modified_cmds;
	std::list<wme*>          removed_cmds;
	std::map<wme*,cmd_stats> curr_cmds;
	
} goal_info;

class soar_interface {
public:
	soar_interface(agent *a);
	~soar_interface();

	sym_wme_pair make_id_wme(sym_hnd id, std::string attr);
	sym_wme_pair make_id_wme(sym_hnd id, sym_hnd attr);
	wme_hnd      make_str_wme(sym_hnd id, std::string attr, std::string val);
	void         remove_wme(wme_hnd w);
	bool         get_child_wmes(sym_hnd id, wme_list &childs);
	
	bool         is_identifier(sym_hnd sym);
	bool         is_string(sym_hnd sym);
	bool         is_int(sym_hnd sym);
	bool         is_float(sym_hnd sym);
	
	bool         get_val(sym_hnd sym, std::string &v);
	bool         get_val(sym_hnd sym, long &v);
	bool         get_val(sym_hnd sym, float &v);
	
	void         make_common_syms();
	void         del_common_syms();
	void         prepare_new_goal(sym_hnd goal);
	void         prepare_del_goal(sym_hnd goal);
	
private:
	wme_hnd make_wme(Symbol *id, Symbol *attr, Symbol *val);
	
	void process_cmds      ();
	void process_goal_cmds (goal_info &g, std::set<wme*>& all_cmds);
	void parse_cmd_id      (goal_info &g, Symbol* id, std::set<wme*>& all_cmds);
	void parse_cmd_wme     (goal_info &g, wme* w, std::set<wme*>& all_cmds);
	bool detect_id_changes (Symbol* id, cmd_stats &stats);

	agent*  agnt;

	/* commonly used string symbols */
	Symbol* svs_sym;
	Symbol* ltm_sym;
	Symbol* cmd_sym;
	Symbol* scene_sym;
	Symbol* contents_sym;
	Symbol* child_sym;

	std::map<Symbol*,goal_info> goals;
};

#endif
