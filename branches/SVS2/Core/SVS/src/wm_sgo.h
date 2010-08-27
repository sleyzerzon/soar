#ifndef WM_SGO_H
#define WM_SGO_H

/* synchronizes a working memory object with its corresponding scene graph node */

#include <map>
#include <utility>
#include "sg_node.h"
#include "soar_interface.h"

class wm_sgo : public sg_listener {
public:
	wm_sgo(soar_interface *soarinterface, sym_hnd ident, wm_sgo *parent, sg_node *n);
	~wm_sgo();
	
	void update(sg_node *n, sg_node::change_type t);
	wm_sgo* add_child(sg_node *c);
	
private:
	void detach();
	
	wm_sgo*         par;
	sg_node*        n;
	sym_hnd         id;
	wme_hnd         name_wme;
	soar_interface* soarint;

	std::map<wm_sgo*,wme_hnd> childs;

};

#endif
