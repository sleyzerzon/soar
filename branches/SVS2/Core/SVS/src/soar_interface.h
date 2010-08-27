#ifndef SOAR_INTERFACE_H
#define SOAR_INTERFACE_H

#include <string>

typedef int wme_hnd;
typedef int sym_hnd;
typedef std::pair<sym_hnd, wme_hnd> sym_wme_pair;

class soar_interface {
public:
	virtual wme_hnd      make_str_wme(sym_hnd id, std::string attr, std::string val) = 0;
	virtual sym_wme_pair make_id_wme(sym_hnd id, std::string attr) = 0;
	virtual void         remove_wme(wme_hnd w) = 0;
	virtual sym_hnd      get_scene_root() = 0;
	
	virtual bool         get_env_line(std::string &line) = 0;
};

#endif
