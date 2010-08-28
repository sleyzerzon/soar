
#include <utility>
#include <algorithm>
#include <assert.h>
#include "soar_interface.h"
#include "portability.h"
#include "soar_module.h"
#include "symtab.h"
#include "wmem.h"

soar_interface::soar_interface(agent *a)
: agnt(a)
{
	svs_sym      = NULL;
	ltm_sym      = NULL;
	cmd_sym      = NULL;
	scene_sym    = NULL;
	contents_sym = NULL;
	child_sym    = NULL;
}

soar_interface::~soar_interface() {
}

wme_hnd soar_interface::make_wme(Symbol *id, Symbol *attr, Symbol *val) {
	wme* w = soar_module::add_module_wme(agnt, id, attr, val);
	symbol_remove_ref(agnt, attr);
	symbol_remove_ref(agnt, val);
	
	return w;
}

wme_hnd soar_interface::make_str_wme(sym_hnd id, string attr, string val) {
	Symbol *attrsym = make_sym_constant(agnt, attr.c_str());
	Symbol *valsym = make_sym_constant(agnt, val.c_str());
	return make_wme(id, attrsym, valsym);
}

sym_wme_pair soar_interface::make_id_wme(sym_hnd id, string attr) {
	sym_wme_pair p;
	Symbol *attrsym = make_sym_constant(agnt, attr.c_str());
	Symbol *valsym = make_new_identifier(agnt, attr[0], id->id.level);
	p.first = valsym;
	p.second = make_wme(id, attrsym, valsym);
	return p;
}

sym_wme_pair soar_interface::make_id_wme(sym_hnd id, sym_hnd attr) {
	char n;
	sym_wme_pair p;
	Symbol *valsym;
	
	if (attr->common.symbol_type != SYM_CONSTANT_SYMBOL_TYPE || 
	    strlen(attr->sc.name) == 0)
	{
		n = 'a';
	} else {
		n = attr->sc.name[0];
	}
	
	valsym = make_new_identifier(agnt, n, id->id.level);
	p.first = valsym;
	p.second = make_wme(id, attr, valsym);
	return p;
}

void soar_interface::remove_wme(wme_hnd w) {
	soar_module::remove_module_wme(agnt, w);
}

bool soar_interface::get_child_wmes(sym_hnd id, wme_list &childs) {
	slot *s;
	wme *w;
	
	if (!is_identifier(id)) {
		return false;
	}

	childs.clear();
	for ( s=id->id.slots; s!=NULL; s=s->next ) {
		for ( w=s->wmes; w!=NULL; w=w->next ) {
			childs.push_back( w );
		}
	}
	
	return true;
}

bool soar_interface::is_identifier(sym_hnd sym) {
	return (sym->common.symbol_type == IDENTIFIER_SYMBOL_TYPE);
}

bool soar_interface::is_string(sym_hnd sym) {
	return (sym->common.symbol_type == SYM_CONSTANT_SYMBOL_TYPE);
}

bool soar_interface::is_int(sym_hnd sym) {
	return (sym->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE);
}

bool soar_interface::is_float(sym_hnd sym) {
	return (sym->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE);
}

bool soar_interface::get_val(sym_hnd sym, string &v) {
	if (is_string(sym)) {
		v = sym->sc.name;
		return true;
	}
	return false;
}

bool soar_interface::get_val(sym_hnd sym, long &v) {
	if (is_int(sym)) {
		v = sym->ic.value;
		return true;
	}
	return false;
}

bool soar_interface::get_val(sym_hnd sym, float &v) {
	if (is_float(sym)) {
		v = sym->fc.value;
		return true;
	}
	return false;
}

void soar_interface::make_common_syms() {
	svs_sym      = make_sym_constant(agnt, "svs");
	ltm_sym      = make_sym_constant(agnt, "ltm");
	cmd_sym      = make_sym_constant(agnt, "command");
	scene_sym    = make_sym_constant(agnt, "spatial-scene");
	contents_sym = make_sym_constant(agnt, "contents");
	child_sym    = make_sym_constant(agnt, "child");
}

void soar_interface::del_common_syms() {
	symbol_remove_ref(agnt, svs_sym);
	symbol_remove_ref(agnt, ltm_sym);
	symbol_remove_ref(agnt, cmd_sym);
	symbol_remove_ref(agnt, scene_sym);
	symbol_remove_ref(agnt, contents_sym);
	symbol_remove_ref(agnt, child_sym);
}

void soar_interface::prepare_new_goal(sym_hnd goal) {
	goal_info gi;
	
	if (!svs_sym) {
		make_common_syms();
	}
	
	gi.state = goal;
	gi.link = make_id_wme(goal, svs_sym).first;
	
	if (!agnt->top_goal)
	{
		gi.ltm = make_id_wme(gi.link, ltm_sym).first;
		gi.ltm_cmd = make_id_wme(gi.ltm, cmd_sym).first;
	}

	gi.scene = make_id_wme(gi.link, scene_sym).first;
	gi.scene_cmd = make_id_wme(gi.scene, cmd_sym).first;
	gi.scene_contents = make_id_wme(gi.scene, contents_sym).first;
	
	goals[goal] = gi;
}

void soar_interface::prepare_del_goal(sym_hnd goal) {
	/*
	map<Symbol*,goal_info>::iterator i;
	if ((i = goals.find(goal)) == goals.end()) {
		assert(false);
	}
	goal_info &gi = i->second;
	
	symbol_remove_ref(agnt, gi.scene_cmd);
	symbol_remove_ref(agnt, gi.scene_contents);
	symbol_remove_ref(agnt, gi.scene);
	
	if (gi.ltm) {
		symbol_remove_ref(agnt, gi.ltm_cmd);
		symbol_remove_ref(agnt, gi.ltm);
	}
	
	symbol_remove_ref(agnt, gi.link );
	*/
	goals.erase(goal);
}


inline bool soar_interface::detect_id_changes(Symbol* id, cmd_stats &stats)
{
	tc_number tc = get_new_tc_number(agnt);
	bool return_val = false;
	std::stack< Symbol* > to_process;
	slot* s;
	wme* w;
	Symbol* parent;
	int subtree_size = 0;

	to_process.push(id);
	while (!to_process.empty()) {
		parent = to_process.top();
		to_process.pop();
		
		for (s=parent->id.slots; s!=NULL; s=s->next) {
			for (w=s->wmes; w!=NULL; w=w->next) {
				subtree_size++;
				
				if (w->timetag > stats.max_time_tag) {
					return_val = true;
					stats.max_time_tag = w->timetag;
				}

				if (is_identifier(w->value) && w->value->id.tc_num != tc) {
					w->value->id.tc_num = tc;
					to_process.push(w->value);
				}
			}
		}
	}

	if (subtree_size != stats.subtree_size)
	{
		return_val = true;
		stats.subtree_size = subtree_size;
	}

	return return_val;
}

inline void soar_interface::parse_cmd_wme(goal_info& g, wme* w, set<wme*>& all_cmds) {
	map<wme*,cmd_stats>::iterator i;
	
	if (!is_identifier(w->value)) {
		return;
	}
	
	if ((i = g.curr_cmds.find(w)) == g.curr_cmds.end()) {
		cmd_stats &stats = g.curr_cmds[w];  // inserts and initializes
		g.new_cmds.push_back(w);
		detect_id_changes(w->value, stats);
	} else if (detect_id_changes(w->value, i->second)) {
		g.modified_cmds.push_back(w);
	}

	all_cmds.insert(w);
}

inline void soar_interface::parse_cmd_id(goal_info &g, Symbol* id, set<wme*>& all_cmds)
{
	slot* s;
	wme* w;

	for (s=id->id.slots; s!=NULL; s=s->next) {
		for (w=s->wmes; w!=NULL; w=w->next) {
			parse_cmd_wme(g, w, all_cmds);
		}
	}
}

inline void soar_interface::process_goal_cmds(goal_info &g, set<wme*>& all_cmds) {
	parse_cmd_id(g, g.scene_cmd, all_cmds);
	if ( g.ltm_cmd ) {
		parse_cmd_id(g, g.ltm_cmd, all_cmds);
	}
}

void soar_interface::process_cmds() {
	map<Symbol*, goal_info>::iterator i;
	map<wme*,cmd_stats>::iterator j;
	std::list<wme*>::iterator k;
	set<wme*> all_cmds;

	for (i = goals.begin(); i != goals.end(); ++i) {
		goal_info &gi = i->second;
		gi.new_cmds.clear();
		gi.modified_cmds.clear();
		gi.removed_cmds.clear();

		// populate new, modified, stuff needed for removed
		process_goal_cmds(gi, all_cmds);

		// populate removed
		for (j = gi.curr_cmds.begin(); j != gi.curr_cmds.end(); ++j) {
			if (all_cmds.find(j->first) == all_cmds.end()) {
				gi.removed_cmds.push_back(j->first);
			}
		}

		// erase removed from command map
		for (k = gi.removed_cmds.begin(); k != gi.removed_cmds.end(); ++k) {
			gi.curr_cmds.erase(*k);
		}

		all_cmds.clear();
	}
}
