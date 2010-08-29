#include "sgel_interp.h"
#include "nsg_node.h"
#include "scene.h"
#include <stdlib.h>
#include <vector>

using namespace std;

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

sgel_interp::sgel_interp(scene *_scn) 
: scn(_scn)
{}

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
		case 'd':
			return parse_detach(f);
		case 'c':
			return parse_change(f);
		default:
			return 0;
	}
}

int sgel_interp::parse_attach(vector<string> &f) {
	sg_node *n;
	ptlist verts;
	int pos;

	if (f.size() < 2) {
		return f.size();
	}
	if (scn->get_node(f[0])) {
		return 0;  // already exists
	}
	if (!scn->get_node(f[1])) {
		return 1;  // parent doesn't exist
	}
	
	pos = 2;
	if (!parse_verts(f, pos, verts)) {
		return pos;
	}
	if (verts.size() == 0) {
		n = scn->add_group(f[0], f[1]);
	} else {
		n = scn->add_geometry(f[0], f[1], verts);
	}

	if (!parse_transforms(f, pos, n)) {
		return pos;
	}
	return -1;
}

int sgel_interp::parse_detach(vector<string> &f) {
	if (f.size() != 1) {
		return 0;
	}
	if (!scn->del_node(f[0])) {
		return 0;
	}
	return -1;
}

int sgel_interp::parse_change(vector<string> &f) {
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

