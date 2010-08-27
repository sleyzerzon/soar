
#include <utility>
#include "svs_soar_int.h"
#include "soar_module.h"
#include "symtab.h"

svs_soar_int::svs_soar_int(agent *_agnt)
: agnt(_agnt), scene_root_hnd(-1)
{ }

wme_hnd svs_soar_int::make_wme(Symbol *id, Symbol *attr, Symbol *val) {
	wme* w = soar_module::add_module_wme(agnt, id, attr, val);
	symbol_remove_ref(agnt, attr);
	symbol_remove_ref(agnt, val);
	
	wmes.push_back(w);
	return wmes.size() - 1;
}

wme_hnd svs_soar_int::make_str_wme(sym_hnd id, string attr, string val) {
	sym_wme_pair p;
	Symbol *idsym = syms[id];
	Symbol *attrsym = make_sym_constant(agnt, attr.c_str());
	Symbol *valsym = make_sym_constant(agnt, val.c_str());
	return make_wme(idsym, attrsym, valsym);
}

sym_wme_pair svs_soar_int::make_id_wme(sym_hnd id, string attr) {
	sym_wme_pair p;
	Symbol *idsym = syms[id];
	Symbol *attrsym = make_sym_constant(agnt, attr.c_str());
	Symbol *valsym = make_new_identifier(agnt, attr[0], idsym->id.level);
	syms.push_back(valsym);
	p.first = syms.size() - 1;
	p.second = make_wme(idsym, attrsym, valsym);
	return p;
}

void svs_soar_int::remove_wme(wme_hnd w) {
	soar_module::remove_module_wme(agnt, wmes[w]);
	wmes[w] = NULL;
}

sym_hnd svs_soar_int::get_scene_root() {
	if (scene_root_hnd < 0) {
		syms.push_back(agnt->top_goal->id.svs_spatial_scene_contents_header);
		scene_root_hnd = syms.size() - 1;
	}
	return scene_root_hnd;
}

