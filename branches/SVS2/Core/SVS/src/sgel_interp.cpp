#include <stdlib.h>
#include <vector>
#include "nsg_node.h"
#include "scene.h"
#include "common.h"

using namespace std;

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

int parse_attach(vector<string> &f, scene *scn) {
	sg_node *n, *p;
	ptlist verts;
	int pos;

	if (f.size() < 2) {
		return f.size();
	}
	if (scn->get_node(f[0])) {
		return 0;  // already exists
	}
	if (!(p = scn->get_node(f[1]))) {
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

int parse_detach(vector<string> &f, scene *scn) {
	sg_node *n;
	if (f.size() != 1) {
		return 0;
	}
	if (!(n = scn->get_node(f[0]))) {
		return 0;
	}
	delete n;
	return -1;
}

int parse_change(vector<string> &f, scene *scn) {
	sg_node *n;
	int pos;

	if (f.size() < 1) {
		return f.size();
	}
	if (!(n = scn->get_node(f[0]))) {
		return 0;
	}
	pos = 1;
	if (!parse_transforms(f, pos, n)) {
		return pos;
	}
	return -1;
}

int parse_state_update(string s, scene *scn) {
	vector<string> lines, fields;
	vector<string>::iterator i;
	char cmd;
	
	split(s, "\n", lines);
	for (i = lines.begin(); i != lines.end(); ++i) {
		split(*i, " \t", fields);
		
		if (fields.size() == 0)
			continue;
		if (fields[0].size() != 1)
			return 0;
		
		cmd = fields[0][0];
		fields.erase(fields.begin());
		
		switch(cmd) {
			case 'a':
				parse_attach(fields, scn);
				break;
			case 'd':
				parse_detach(fields, scn);
				break;
			case 'c':
				parse_change(fields, scn);
				break;
			default:
				return 0;
		}
	}
}
