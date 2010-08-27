#ifndef SVS_SOAR_INT_H
#define SVS_SOAR_INT_H

#include <vector>
#include "soar_interface.h"
#include "agent.h"

class svs_soar_int : public soar_interface {
public:
	svs_soar_int(agent *_agnt);

	wme_hnd      make_str_wme(sym_hnd id, std::string attr, std::string val);
	sym_wme_pair make_id_wme(sym_hnd id, std::string attr);
	void         remove_wme(wme_hnd w);

	sym_hnd      get_scene_root();
	
private:
	wme_hnd make_wme(Symbol *id, Symbol *attr, Symbol *val);

	agent*                    agnt;
	std::vector<Symbol*>      syms;
	std::vector<wme*>         wmes;
	std::map<std::string,int> const_syms;

	sym_hnd scene_root_hnd;
};

#endif
