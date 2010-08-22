#include "sgel_interp.h"
#include "nsg_node.h"
#include <map>
#include <list>
#include <vector>

using namespace std;
typedef map<string, sg_node*>::iterator node_iter;

void split(string s, vector<string> &fields) {
	int start, end = 0;
	fields.clear();
	while (end < s.size()) {
		start = s.find_first_not_of(" \t", end);
		if (start == string::npos) {
			return;
		}
		end = s.find_first_of(" \t", start);
		if (end == string::npos) {
			end = s.size();
		}
		fields.push_back(s.substr(start, end - start));
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

sgel_interp::sgel_interp(sg_node *root) {
	nodes[root->get_name()] = root;
}

int sgel_interp::parse_line(string s) {
	vector<string> f;
	char cmd;
	
	split(s, f);
	if (f.size() == 0) {
		return -1;
	}
	if (f[0].size() != 1) {
		return 0;
	}
	cmd = f[0][0];
	f.erase(f.begin());
	
	switch(cmd) {
		case 'a':
			return parse_attach(f);
		default:
			return 0;
	}
}

int sgel_interp::parse_attach(vector<string> &f) {
	sg_node *n, *par;
	node_iter i;
	string name;
	list<Point3> verts;
	int pos;

	if (f.size() < 2) {
		return f.size();
	}
	i = nodes.find(f[0]);
	if (i != nodes.end()) {
		return 0;
	}
	i = nodes.find(f[1]);
	if (i == nodes.end()) {
		return 1;
	}
	name = f[0];
	par = i->second;
	
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
	nodes[name] = n;
	return -1;
}

int sgel_interp::parse_detach(vector<string> &f) {
	node_iter i;
	sg_node* p;
	if (f.size() != 1) {
		return 0;
	}
	i = nodes.find(f[0]);
	if (i == nodes.end()) {
		return 0;
	}
	p = i->second->get_parent();
	if (!p) {
		return 0;
	}
	p->detach_child(i->second);
	return -1;
}

int sgel_interp::parse_change(vector<string> &f) {
	node_iter i;
	list<Point3> verts;
	int pos;

	if (f.size() < 1) {
		return f.size();
	}
	i = nodes.find(f[0]);
	if (i == nodes.end()) {
		return 0;
	}
	pos = 1;
	if (!parse_transforms(f, pos, i->second)) {
		return pos;
	}
	return -1;
}

