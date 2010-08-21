#include "sgel_interp.h"
#include "nsg_node.h"
#include <map>
#include <list>
#include <vector>

using namespace std;
typedef map<string, sg_node*>::iterator node_iter;

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

bool parse_verts(vector<string> &f, int &start, list<Point3> &verts) {
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
		verts.push_back(Point3(x[0], x[1], x[2]));
	}
	return true;
}

bool parse_transforms(vector<string> &f, int &start, sg_node *n) {
	Vector3 v;
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
		v = Vector3(x[0], x[1], x[2]);
		switch (type) {
			case 'p':
				n->set_pos(v);
				break;
			case 'r':
				n->set_rot(v);
				break;
			case 's':
				n->set_scale(v);
				break;
		}
	}
	return true;
}

sgel_interp::sgel_interp(sg_node *world) {
	nodes["world"] = world;
}

int sgel_interp::parse_add(vector<string> &f) {
	sg_node *n, *par;
	node_iter i;
	string name;
	list<Point3> verts;
	int pos;

	if (f.size() < 2) {
		return f.size();
	}
	name = f[0];
	i = nodes.find(f[1]);
	if (i == nodes.end()) {
		return 1;
	} else {
		par = i->second;
	}
	
	pos = 2;
	if (!parse_verts(f, pos, verts)) {
		return pos;
	}
	if (verts.size() == 0) {
		n = new nsg_node(name);
	} else {
		n = new nsg_node(name, verts.begin(), verts.end());
	}

	if (!parse_transforms(f, pos, n)) {
		return pos;
	}
	par->attach_child(n);
	return -1;
}

