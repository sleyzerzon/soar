#ifndef SOAR_INTERFACE_H
#define SOAR_INTERFACE_H

#include <utility>
#include <list>
#include <sstream>
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

	sym_hnd      make_sym(const std::string &val);
	sym_hnd      make_sym(int val);
	sym_hnd      make_sym(double val);
	void         del_sym(sym_hnd s);
	
	sym_wme_pair make_id_wme(sym_hnd id, const std::string &attr);
	sym_wme_pair make_id_wme(sym_hnd id, sym_hnd attr);
	
	template<class T>
	wme_hnd      make_wme(sym_hnd id, const std::string &attr, const T &val);
	
	template<class T>
	wme_hnd      make_wme(sym_hnd id, sym_hnd attr, const T &val);
	
	void         remove_wme(wme_hnd w);
	bool         get_child_wmes(sym_hnd id, wme_list &childs);
	bool         find_child_wme(sym_hnd id, const std::string &attr, wme_hnd &w);

	bool         is_identifier(sym_hnd sym);
	bool         is_string(sym_hnd sym);
	bool         is_int(sym_hnd sym);
	bool         is_float(sym_hnd sym);
	bool         is_state(sym_hnd sym);
	bool         is_top_state(sym_hnd sym);
	
	bool         get_name(sym_hnd sym, std::string &n);
	bool         get_val(sym_hnd sym, std::string &v);
	bool         get_val(sym_hnd sym, long &v);
	bool         get_val(sym_hnd sym, float &v);
	
	sym_hnd      get_wme_attr(wme_hnd w);
	sym_hnd      get_wme_val(wme_hnd w);

	tc_num       new_tc_num();
	tc_num       get_tc_num(sym_hnd s);
	void         set_tc_num(sym_hnd s, tc_num n);
	
	int          get_timetag(wme_hnd w);
	
	sym_hnd      get_parent_state(sym_hnd sym);
	
private:
	agent*  agnt;

};

inline sym_hnd soar_interface::make_sym(const std::string &val) {
	return make_sym_constant(agnt, val.c_str());
}

inline sym_hnd soar_interface::make_sym(int val) {
	return make_int_constant(agnt, val);
}

inline sym_hnd soar_interface::make_sym(double val) {
	return make_float_constant(agnt, val);
}

inline void soar_interface::del_sym(sym_hnd s) {
	symbol_remove_ref(agnt, s);
}

template<class T>
wme_hnd soar_interface::make_wme(sym_hnd id, const std::string &attr, const T &val) {
	wme* w;
	Symbol *attrsym = make_sym(attr);
	w = make_wme(id, attrsym, val);
	symbol_remove_ref(agnt, attrsym);
	return w;
}

template<class T>
wme_hnd soar_interface::make_wme(sym_hnd id, sym_hnd attr, const T &val) {
	Symbol *valsym = make_sym(val);
	wme* w = soar_module::add_module_wme(agnt, id, attr, valsym);
	symbol_remove_ref(agnt, valsym);
	return w;
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

inline bool soar_interface::get_name(sym_hnd sym, std::string &n) {
	std::stringstream ss;
	if (!is_identifier(sym))
		return false;
	ss << sym->id.name_letter << sym->id.name_number;
	n = ss.str();
	return true;
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

inline sym_hnd soar_interface::get_wme_attr(wme_hnd w) {
	return w->attr;
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

inline sym_hnd soar_interface::get_parent_state(sym_hnd id) {
	return id->id.higher_goal;
}

#endif
