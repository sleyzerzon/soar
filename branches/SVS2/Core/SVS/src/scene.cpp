#include <map>
#include <iterator>
#include <sstream>
#include "scene.h"
#include "nsg_node.h"
#include "linalg.h"

using namespace std;

scene::scene(string name, string rootname, bool display) 
: name(name)
{
	if (display) {
		disp = new ipcsocket("/tmp/svsdisp");
		disp->send("newscene", name);
	} else {
		disp = NULL;
	}
	if (disp) 
	root = new nsg_node(rootname);
	handle_add(root);
}

scene::scene(string name, scene *p)
: name(name), disp(p->disp)
{
	if (disp) disp->send("newscene", name);
	root = p->root->copy();
	handle_add(root);
}

scene::~scene() {
	delete root;
	if (disp) disp->send("delscene", name);
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

void scene::update_object(sg_node *n) {
	stringstream ss;
	ptlist pts;
	
	if (n->is_group()) {
		return;
	}
	ss << name << '\n' << n->get_name() << '\n';
	n->get_world_points(pts);
	copy(pts.begin(), pts.end(), ostream_iterator<vec3>(ss, "\n"));
	if (disp) disp->send("updateobject", ss.str());
}

void scene::handle_add(sg_node *n) {
	int i;
	
	update_object(n);
	nodes[n->get_name()] = n;
	n->listen(this);
	for (i = 0; i < n->num_children(); ++i) {
		handle_add(n->get_child(i));
	}
}

void scene::handle_del(sg_node *n) {
	if (!n->is_group()) {
		if (disp) disp->send("delobject", name + '\n' + n->get_name());
	}
	nodes.erase(n->get_name());
}

void scene::handle_ptschange(sg_node *n) {
	update_object(n);
}


void scene::update(sg_node *n, sg_node::change_type t) {
	switch(t) {
		case sg_node::CHILD_ADDED:
			handle_add(n);
			break;
		case sg_node::DELETED:
			handle_del(n);
			break;
		case sg_node::POINTS_CHANGED:
			handle_ptschange(n);
			break;
	}
}
