#include "wm_sgo.h"
#include "assert.h"
#include "soar_int.h"

using namespace std;

typedef map<wm_sgo*, wme_hnd>::iterator child_iter;

wm_sgo::wm_sgo(soar_interface *si, sym_hnd ident, wm_sgo *parent, sg_node *node) 
: soarint(si), id(ident), parent(parent), node(node)
{
	int i;
	node->listen(this);
	name_wme = soarint->make_str_wme(id, "name", node->get_name());
	for (i = 0; i < node->num_children(); ++i) {
		add_child(node->get_child(i));
	}
}

wm_sgo::~wm_sgo() {
	soarint->remove_wme(name_wme);
	if (parent) {
		child_iter ci = parent->childs.find(this);
		assert(ci != parent->childs.end());
		soarint->remove_wme(ci->second);
	}
	node->unlisten(this);
}

void wm_sgo::update(sg_node *n, sg_node::change_type t) {
	switch (t) {
		case sg_node::CHILD_ADDED:
			add_child(node->get_child(node->num_children()-1));
			break;
		case sg_node::DETACHED:
		case sg_node::DELETED:
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

