#include <map>
#include <iterator>
#include <sstream>
#include "scene.h"
#include "nsg_node.h"
#include "linalg.h"
#include "ipcsocket.h"
#include "common.h"

using namespace std;

ipcsocket *disp = NULL;

scene::scene(string name, string rootname) 
: name(name), rootname(rootname)
{
	if (!disp) {
		disp = new ipcsocket(getnamespace() + "disp");
	}
	add_node("", rootname);
	root = nodes[rootname];
	disp->send("newscene", name);
}

scene::scene(scene &other)
: name(other.name), rootname(other.rootname)
{
	std::list<sg_node*> all_nodes;
	std::list<sg_node*>::iterator i;
	
	root = other.root->copy();
	root->walk(all_nodes);
	for(i = all_nodes.begin(); i != all_nodes.end(); ++i) {
		nodes[(**i).get_name()] = *i;
	}
}

scene::~scene() {
	delete root;
	if (disp) disp->send("delscene", name);
}

sg_node* scene::get_root() {
	return root;
}

sg_node* scene::get_node(string name) {
	node_map::iterator i;
	if ((i = nodes.find(name)) == nodes.end()) {
		return NULL;
	}
	return i->second;
}

bool scene::add_node(string parent, sg_node *n) {
	sg_node *p = get_node(parent);
	
	if (parent != "" && !p) {
		return false;
	}
	if (parent != "" && !p->attach_child(n)) {
		delete n;
		return false;
	}
	nodes[n->get_name()] = n;
	disp_update_node(n);
	return true;
}

bool scene::add_node(string parent, string name) {
	return add_node(parent, new nsg_node(name));
}

bool scene::add_node(string parent, string name, const ptlist &points) {
	return add_node(parent, new nsg_node(name, points));
}

bool scene::del_node(string name) {
	node_map::iterator i;
	if ((i = nodes.find(name)) == nodes.end()) {
		return false;
	}
	disp_del_node(i->second);
	delete i->second;
	nodes.erase(i);
	return true;
}

bool scene::set_node_trans(string name, char type, vec3 trans) {
	sg_node *n = get_node(name);
	if (!n) return false;
	n->set_trans(type, trans);
	return true;
}

void scene::clear() {
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

bool scene::parse_transforms(vector<string> &f, int &start) {
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
		set_node_trans(f[0], type, vec3(x[0], x[1], x[2]));
	}
	return true;
}

int scene::parse_add(vector<string> &f) {
	ptlist verts;
	int pos;

	if (f.size() < 2) {
		return f.size();
	}
	if (get_node(f[0])) {
		return 0;  // already exists
	}
	
	pos = 2;
	if (!parse_verts(f, pos, verts)) {
		return pos;
	}
	if (verts.size() == 0) {
		if (!add_node(f[1], f[0])) {
			return pos;
		}
	} else {
		if (!add_node(f[1], f[0], verts)) {
			return pos;
		}
	}
	if (!parse_transforms(f, pos)) {
		del_node(f[0]);
		return pos;
	}
	
	return -1;
}

int scene::parse_del(vector<string> &f) {
	if (f.size() != 1) {
		return f.size();
	}
	if (!del_node(f[0])) {
		return 0;
	}
	return -1;
}

int scene::parse_change(vector<string> &f) {
	int pos;

	if (f.size() < 1) {
		return f.size();
	}
	if (!get_node(f[0])) {
		return 0;
	}
	pos = 1;
	if (!parse_transforms(f, pos)) {
		return pos;
	}
	return -1;
}

void scene::parse_sgel(string s) {
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
				errfield = parse_add(fields);
				break;
			case 'd':
				errfield = parse_del(fields);
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

void scene::disp_update_node(sg_node *n) {
	stringstream ss;
	ptlist pts;
	
	if (!n->is_group() && disp) {
		ss << name << '\n' << n->get_name() << '\n';
		n->get_world_points(pts);
		copy(pts.begin(), pts.end(), ostream_iterator<vec3>(ss, "\n"));
		disp->send("updateobject", name + '\n' + n->get_name());
	}
}

void scene::disp_del_node(sg_node *n){
	if (!n->is_group() && disp) {
		disp->send("delobject", name + '\n' + n->get_name());
	}
}
