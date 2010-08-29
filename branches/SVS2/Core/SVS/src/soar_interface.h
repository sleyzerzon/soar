#ifndef SOAR_INTERFACE_H
#define SOAR_INTERFACE_H

#include <utility>
#include <list>
#include "portability.h"
#include "agent.h"
#include "symtab.h"
#include "wmem.h"

typedef wme*      wme_hnd;
typedef Symbol*   sym_hnd;
typedef tc_number tc_num;

typedef std::pair<sym_hnd, wme_hnd> sym_wme_pair;
typedef std::list<wme_hnd> wme_list;


class soar_interface {
public:
	soar_interface(agent *a);
	~soar_interface();

	sym_hnd      make_string_sym(std::string val);
	void         del_string_sym(sym_hnd s);
	
	sym_wme_pair make_id_wme(sym_hnd id, std::string attr);
	sym_wme_pair make_id_wme(sym_hnd id, sym_hnd attr);
	wme_hnd      make_str_wme(sym_hnd id, std::string attr, std::string val);
	void         remove_wme(wme_hnd w);
	bool         get_child_wmes(sym_hnd id, wme_list &childs);
	
	bool         is_identifier(sym_hnd sym);
	bool         is_string(sym_hnd sym);
	bool         is_int(sym_hnd sym);
	bool         is_float(sym_hnd sym);
	bool         is_state(sym_hnd sym);
	bool         is_top_state(sym_hnd sym);
	
	bool         get_val(sym_hnd sym, std::string &v);
	bool         get_val(sym_hnd sym, long &v);
	bool         get_val(sym_hnd sym, float &v);
	
	sym_hnd      get_wme_val(wme_hnd w);

	tc_num       new_tc_num();
	tc_num       get_tc_num(sym_hnd s);
	void         set_tc_num(sym_hnd s, tc_num n);
	
	int          get_timetag(wme_hnd w);
	
private:
	agent*  agnt;

};

inline sym_hnd soar_interface::make_string_sym(std::string val) {
	make_sym_constant(agnt, val.c_str());
}

inline void soar_interface::del_string_sym(sym_hnd s) {
	symbol_remove_ref(agnt, s);
}

inline bool soar_interface::is_identifier(sym_hnd sym) {
	return sym->common.symbol_type == IDENTIFIER_SYMBOL_TYPE;
}

inline bool soar_interface::is_string(sym_hnd sym) {
	return sym->common.symbol_type == SYM_CONSTANT_SYMBOL_TYPE;
}

inline bool soar_interface::is_int(sym_hnd sym) {
	return sym->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE;
}

inline bool soar_interface::is_float(sym_hnd sym) {
	return sym->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE;
}

inline bool soar_interface::is_state(sym_hnd sym) {
	return is_identifier(sym) && sym->id.isa_goal;
}

inline bool soar_interface::is_top_state(sym_hnd sym) {
	return is_state(sym) && (sym->id.higher_goal == NULL);
}

inline bool soar_interface::get_val(sym_hnd sym, std::string &v) {
	if (is_string(sym)) {
		v = sym->sc.name;
		return true;
	}
	return false;
}

inline bool soar_interface::get_val(sym_hnd sym, long &v) {
	if (is_int(sym)) {
		v = sym->ic.value;
		return true;
	}
	return false;
}

inline bool soar_interface::get_val(sym_hnd sym, float &v) {
	if (is_float(sym)) {
		v = sym->fc.value;
		return true;
	}
	return false;
}

inline sym_hnd soar_interface::get_wme_val(wme_hnd w) {
	return w->value;
}

inline tc_num soar_interface::new_tc_num() {
	return get_new_tc_number(agnt);
}

inline tc_num soar_interface::get_tc_num(sym_hnd s) {
	return s->id.tc_num;
}

inline void soar_interface::set_tc_num(sym_hnd s, tc_num n) {
	s->id.tc_num = n;
}

inline int soar_interface::get_timetag(wme_hnd w) {
	return w->timetag;
}

#endif
