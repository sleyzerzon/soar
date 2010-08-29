#include "scene.h"
#include "nsg_node.h"
#include <map>
#include <list>

using namespace std;

typedef map<string, sg_node*>::iterator node_iter;

void collect_subtree(sg_node* root, std::list<sg_node*> &n) {
	n.push_back(root);
	for (int i = 0; i < root->get_nchilds(); ++i) {
		collect_subtree(root->get_child(i), n);
	}
}

scene::scene(string rootname) {
	nodes[rootname] = new nsg_node(rootname);
}

sg_node* scene::get_node(string name) {
	node_iter i;
	if ((i = nodes.find(name)) == nodes.end()) {
		return NULL;
	}
	return i->second;
}

bool scene::add_node(string par, sg_node *n) {
	sg_node *p = get_node(par);
	if (!p || !p->attach_child(n)) {
		return false;
	}
	if (get_node(n->get_name())) {
		return false;
	}
	nodes[n->get_name()] = n;
	return true;
}

sg_node* scene::add_group(string name, string par) {
	sg_node *n = new nsg_node(name);
	if (!add_node(par, n)) {
		delete n;
		return NULL;
	}
	return n;
}

sg_node* scene::add_geometry(string name, string par, ptlist &points) {
	sg_node *n = new nsg_node(name, points);
	if (!add_node(par, n)) {
		delete n;
		return NULL;
	}
	return n;
}

bool scene::del_node(string name) {
	sg_node *n = get_node(name);
	if (!n) {
		return false;
	}
	delete n;
	return true;
}
