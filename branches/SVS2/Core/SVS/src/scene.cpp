#include <map>
#include <iterator>
#include <sstream>
#include "scene.h"
#include "nsg_node.h"
#include "linalg.h"

using namespace std;

scene::scene(string name, string rootname, ipcsocket *disp) 
: name(name), rootname(rootname), disp(disp)
{
	if (disp) disp->send("newscene", name);
	root = new nsg_node(rootname);
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

void scene::wipe() {
	int i;
	for (i = 0; i < root->num_children(); ++i) {
		delete root->get_child(i);
	}
}

bool parse_n_floats(vector<string> &f, int &start, int n, double *x) {
	const char *cs;
	char *e;
	if (start + n > f.size()) {
		start = f.size();
		return false;
	}
	for (int i = 0; i < n; ++start, ++i) {
		cs = f[start].c_str();
		x[i] = strtod(cs, &e);
		if (e == cs) {  // conversion failure
			return false;
		}
	}
	return true;
}

bool parse_verts(vector<string> &f, int &start, ptlist &verts) {
	double x[3];
	verts.clear();
	int i;
	if (start >= f.size() || f[start] != "v") {
		return true;
	}
	start++;
	while (start < f.size()) {
		i = start;
		if (!parse_n_floats(f, start, 3, x)) {
			return (i == start);  // end of list
		}
		verts.push_back(vec3(x[0], x[1], x[2]));
	}
	return true;
}

bool parse_transforms(vector<string> &f, int &start, sg_node *n) {
	double x[3];
	char type;
	while (start < f.size()) {
		if (f[start] != "p" && f[start] != "r" && f[start] != "s") {
			return true;
		}
		type = f[start][0];
		start++;
		if (!parse_n_floats(f, start, 3, x)) {
			return false;
		}
		switch (type) {
			case 'p':
				n->set_pos(vec3(x[0], x[1], x[2]));
				break;
			case 'r':
				n->set_rot(vec3(x[0], x[1], x[2]));
				break;
			case 's':
				n->set_scale(vec3(x[0], x[1], x[2]));
				break;
		}
	}
	return true;
}

int scene::parse_attach(vector<string> &f) {
	sg_node *n, *p;
	ptlist verts;
	int pos;

	if (f.size() < 2) {
		return f.size();
	}
	if (get_node(f[0])) {
		return 0;  // already exists
	}
	if (!(p = get_node(f[1]))) {
		return 1;  // parent doesn't exist
	}
	
	pos = 2;
	if (!parse_verts(f, pos, verts)) {
		return pos;
	}
	if (verts.size() == 0) {
		n = new nsg_node(f[0]);
	} else {
		n = new nsg_node(f[0], verts);
	}

	if (!parse_transforms(f, pos, n)) {
		delete n;
		return pos;
	}
	p->attach_child(n);
	
	return -1;
}

int scene::parse_detach(vector<string> &f) {
	sg_node *n;
	if (f.size() != 1) {
		return f.size();
	}
	if (!(n = get_node(f[0]))) {
		return 0;
	}
	delete n;
	return -1;
}

int scene::parse_change(vector<string> &f) {
	sg_node *n;
	int pos;

	if (f.size() < 1) {
		return f.size();
	}
	if (!(n = get_node(f[0]))) {
		return 0;
	}
	pos = 1;
	if (!parse_transforms(f, pos, n)) {
		return pos;
	}
	return -1;
}

void scene::update_sgel(string s) {
	vector<string> lines, fields;
	vector<string>::iterator i;
	char cmd;
	int errfield;
	
	split(s, "\n", lines);
	for (i = lines.begin(); i != lines.end(); ++i) {
		split(*i, " \t", fields);
		
		if (fields.size() == 0)
			continue;
		if (fields[0].size() != 1) {
			cerr << "expecting a|d|c at beginning of line '" << *i << "'" << endl;
			exit(1);
		}
		
		cmd = fields[0][0];
		fields.erase(fields.begin());
		
		switch(cmd) {
			case 'a':
				errfield = parse_attach(fields);
				break;
			case 'd':
				errfield = parse_detach(fields);
				break;
			case 'c':
				errfield = parse_change(fields);
				break;
			default:
				cerr << "expecting a|d|c at beginning of line '" << *i << "'" << endl;
				exit(1);
		}
		
		if (errfield >= 0) {
			cerr << "error in field " << errfield << " of line '" << *i << "' " << endl;
			exit(1);
		}
	}
}
