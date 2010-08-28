#include "wm_sgo.h"
#include "assert.h"
#include "soar_int.h"

using namespace std;

typedef map<wm_sgo*, wme_hnd>::iterator child_iter;

wm_sgo::wm_sgo(soar_interface *si, sym_hnd ident, wm_sgo *parent, sg_node *node) 
: soarint(si), id(ident), par(parent), n(node)
{
	name_wme = soarint->make_str_wme(id, "name", node->get_name());
	n->listen(this);
}

wm_sgo::~wm_sgo() {
	soarint->remove_wme(name_wme);
	if (par) {
		child_iter ci = par->childs.find(this);
		assert(ci != par->childs.end());
		soarint->remove_wme(ci->second);
	}
	n->unlisten(this);
}

void wm_sgo::update(sg_node *n, sg_node::change_type t) {
	switch (t) {
		case sg_node::ADDCHILD:
			add_child(n->get_child(n->get_nchilds()-1));
			break;
		case sg_node::DETACH:
		case sg_node::DEL:
			delete this;
			break;
	};
}

wm_sgo* wm_sgo::add_child(sg_node *c) {
	sym_wme_pair cid_wme;
	char letter;
	string cname = c->get_name();
	wm_sgo *child;
	
	if (cname.size() == 0 || !isalpha(cname[0])) {
		letter = 'n';
	} else {
		letter = cname[0];
	}
	cid_wme = soarint->make_id_wme(id, "child");
	
	child = new wm_sgo(soarint, cid_wme.first, this, c);
	childs[child] = cid_wme.second;
	return child;
}

