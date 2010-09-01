#include "scene.h"
#include "nsg_node.h"
#include <map>

using namespace std;

scene::scene(std::string rootname) {
	root = new nsg_node(rootname);
	update_names(root);
}

scene::scene(scene *c) {
	root = c->root->copy();
	update_names(root);
}

scene::~scene() {
	delete root;
}

void scene::update_names(sg_node *n) {
	int i;
	nodes[n->get_name()] = n;
	n->listen(this);
	for (i = 0; i < n->num_children(); ++i) {
		update_names(n->get_child(i));
	}
}

sg_node* scene::get_root() {
	return root;
}

sg_node* scene::get_node(std::string name) {
	node_map::iterator i;
	if ((i = nodes.find(name)) == nodes.end()) {
		return NULL;
	}
	return i->second;
}

void scene::update(sg_node *n, sg_node::change_type t) {
	switch(t) {
		case sg_node::CHILD_ADDED:
			update_names(n);
			break;
		case sg_node::DELETED:
			nodes.erase(n->get_name());
			break;
	}
}
