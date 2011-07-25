#include <stdlib.h>
#include <map>
#include <iterator>
#include <iostream>
#include <sstream>
#include <limits>
#include <utility>
#include "scene.h"
#include "nsg_node.h"
#include "linalg.h"
#include "ipcsocket.h"
#include "common.h"

using namespace std;

ipcsocket *disp = NULL;

/*
 Native properties are currently the position, rotation, and scaling
 transforms of a node, named px, py, pz, rx, ry, rz, sx, sy, sz.
*/
const int NUM_NATIVE_PROPS = 9;
const char *NATIVE_PROPS[] = { "px", "py", "pz", "rx", "ry", "rz", "sx", "sy", "sz" };

bool is_native_prop(const string &name, char &type, int &dim) {
	int d;
	if (name.size() != 2) {
		return false;
	}
	if (name[0] != 'p' && name[0] != 'r' && name[0] != 's') {
		return false;
	}
	d = name[1] - 'x';
	if (d < 0 || d > 2) {
		return false;
	}
	type = name[0];
	dim = d;
	return true;
}

scene::scene(string name, string rootname) 
: name(name), rootname(rootname), iscopy(false), dt(1.0)
{
	if (!disp) {
		disp = new ipcsocket(getnamespace() + "disp", false);
	}
	disp->listen(this);
	add_node("", rootname);
	root = nodes[rootname].node;
	disp_new_scene();
}

scene::scene(const scene &other)
: name(other.name), rootname(other.rootname), iscopy(true), dt(other.dt)
{
	string name;
	std::list<sg_node*> all_nodes;
	std::list<sg_node*>::iterator i;
	
	root = other.root->copy();
	root->walk(all_nodes);
	
	/*
	 Make a deep copy of the nodes table, which will result in
	 a table with pointers to other's nodes, then go through and
	 change to point to our nodes.
	*/
	nodes = other.nodes;
	for(i = all_nodes.begin(); i != all_nodes.end(); ++i) {
		nodes[(**i).get_name()].node = *i;
	}
}

scene::~scene() {
	if (!iscopy) {
		disp_del_scene();
	}
	delete root;
	if (disp) {
		disp->unlisten(this);
	}
}

sg_node* scene::get_root() {
	return root;
}

sg_node* scene::get_node(const string &name) {
	node_map::const_iterator i;
	if ((i = nodes.find(name)) == nodes.end()) {
		return NULL;
	}
	return i->second.node;
}

sg_node const *scene::get_node(const string &name) const {
	node_map::const_iterator i;
	if ((i = nodes.find(name)) == nodes.end()) {
		return NULL;
	}
	return i->second.node;
}

void scene::get_all_nodes(vector<sg_node*> &n) {
	node_map::const_iterator i;
	for (i = nodes.begin(); i != nodes.end(); ++i) {
		n.push_back(i->second.node);
	}
}

bool scene::add_node(string parent, sg_node *n) {
	node_info info;
	info.node = n;
	sg_node *p = get_node(parent);
	
	if (parent != "" && !p) {
		return false;
	}
	if (parent != "" && !p->attach_child(n)) {
		delete n;
		return false;
	}
	nodes[n->get_name()] = info;
	if (!iscopy) {
		disp_update_node(n);
	}
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
	if (!iscopy) {
		disp_del_node(i->second.node);
	}
	delete i->second.node;
	nodes.erase(i);
	return true;
}

bool scene::set_node_trans(string name, char type, vec3 trans) {
	sg_node *n = get_node(name);
	if (!n) return false;
	n->set_trans(type, trans);
	if (!iscopy) {
		disp_update_node(n);
	}
	return true;
}

void scene::clear() {
	int i;
	for (i = 0; i < root->num_children(); ++i) {
		delete root->get_child(i);
	}
}

bool parse_n_floats(vector<string> &f, int &start, int n, float *x) {
	stringstream ss;
	if (start + n > f.size()) {
		start = f.size();
		return false;
	}
	for (int i = 0; i < n; ++start, ++i) {
		ss << f[start] << endl;
		if (!(ss >> x[i])) {  // conversion failure
			return false;
		}
	}
	return true;
}

bool parse_verts(vector<string> &f, int &start, ptlist &verts) {
	float x[3];
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
	float x[3];
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

int scene::parse_property(vector<string> &f) {
	node_map::iterator i;
	if (f.size() != 3) {
		return f.size();
	}
	stringstream ss(f[2]);
	float val;
	
	if (!(ss >> val)) {
		return 2;
	}
	if ((i = nodes.find(f[0])) == nodes.end()) {
		return 0;
	}
	i->second.props[f[1]] = val;
	return -1;
}

int scene::parse_dt(vector<string> &f) {
	if (f.size() != 1) {
		return f.size();
	}
	stringstream ss(f[0]);
	
	if (!(ss >> dt)) {
		return 1;
	}
	return -1;
}

void scene::parse_sgel(const string &s) {
	vector<string> lines, fields;
	vector<string>::iterator i;
	char cmd;
	int errfield;
	
	//cerr << "received sgel" << endl << "---------" << endl << s << endl << "---------" << endl;
	split(s, "\n", lines);
	for (i = lines.begin(); i != lines.end(); ++i) {
		split(*i, " \t", fields);
		
		if (fields.size() == 0)
			continue;
		if (fields[0].size() != 1) {
			cerr << "expecting a|d|c|p|t at beginning of line '" << *i << "'" << endl;
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
			case 'p':
				errfield = parse_property(fields);
				break;
			case 't':
				errfield = parse_dt(fields);
				break;
			default:
				cerr << "expecting a|d|c|p|t at beginning of line '" << *i << "'" << endl;
				exit(1);
		}
		
		if (errfield >= 0) {
			cerr << "error in field " << errfield + 1 << " of line '" << *i << "' " << endl;
			exit(1);
		}
	}
}

void scene::disp_update_node(sg_node *n) {
	stringstream ss;
	ptlist pts;
	ptlist::const_iterator i;
	if (!n->is_group() && disp) {
		ss << "updateobject\n" << name << '\n' << n->get_name() << '\n';
		n->get_world_points(pts);
		copy(pts.begin(), pts.end(), ostream_iterator<vec3>(ss, "\n"));
		disp->send(ss.str());
	}
}

void scene::disp_del_node(sg_node *n){
	stringstream ss;
	if (!n->is_group() && disp) {
		ss << "delobject\n" << name << '\n' << n->get_name();
		disp->send(ss.str());
	}
}

void scene::disp_new_scene() {
	if (disp) disp->send("newscene\n" + name);
}

void scene::disp_del_scene() {
	if (disp) disp->send("delscene\n" + name);
}

void scene::get_property_names(vector<string> &names) const {
	node_map::const_iterator i;
	property_map::const_iterator j;
	int k;
	stringstream ss;
	
	for (i = nodes.begin(); i != nodes.end(); ++i) {
		for (k = 0; k < NUM_NATIVE_PROPS; ++k) {
			ss.str("");
			ss << i->first << ":" << NATIVE_PROPS[k];
			names.push_back(ss.str());
		}
		for (j = i->second.props.begin(); j != i->second.props.end(); ++j) {
			ss.str("");
			ss << i->first << ":" << j->first;
			names.push_back(ss.str());
		}
	}
}

void scene::get_properties(floatvec &vals) const {
	node_map::const_iterator i;
	property_map::const_iterator j;
	int k1, k2, l = 0;
	const char *types = "prs";
	vec3 trans;
	
	vals.resize(get_dof());
	for (i = nodes.begin(); i != nodes.end(); ++i) {
		for (k1 = 0; k1 < 3; ++k1) {
			trans = i->second.node->get_trans(types[k1]);
			for (k2 = 0; k2 < 3; ++k2) {
				vals[l++] = trans[k2];
			}
		}
		for (j = i->second.props.begin(); j != i->second.props.end(); ++j) {
			vals[l++] = j->second;
		}
	}
}

bool scene::get_property(const string &obj, const string &prop, float &val) const {
	node_map::const_iterator i;
	property_map::const_iterator j;
	char type; int d;
	if ((i = nodes.find(obj)) == nodes.end()) {
		return false;
	}
	if (is_native_prop(prop, type, d)) {
		val = i->second.node->get_trans(type)[d];
	} else {
		if ((j = i->second.props.find(prop)) == i->second.props.end()) {
			return false;
		}
		val = j->second;
	}
	return true;
}

bool scene::add_property(const string &obj, const string &prop, float val) {
	node_map::iterator i;
	property_map::iterator j;
	char type; int d;
	if ((i = nodes.find(obj)) == nodes.end()) {
		return false;
	}
	if (is_native_prop(prop, type, d)) {
		return false;
	} else {
		if ((j = i->second.props.find(prop)) != i->second.props.end()) {
			return false;
		}
		i->second.props[prop] = val;
	}
	return true;
}

bool scene::set_property(const string &obj, const string &prop, float val) {
	node_map::iterator i;
	property_map::iterator j;
	char type; int d;
	if ((i = nodes.find(obj)) == nodes.end()) {
		return false;
	}
	if (is_native_prop(prop, type, d)) {
		vec3 trans = i->second.node->get_trans(type);
		trans[d] = val;
		i->second.node->set_trans(type, trans);
	} else {
		if ((j = i->second.props.find(prop)) == i->second.props.end()) {
			return false;
		}
		j->second = val;
	}
	return true;
}

bool scene::set_properties(const floatvec &vals) {
	node_map::iterator i;
	property_map::iterator j;
	int k1, k2, l = 0;
	const char *types = "prs";
	vec3 trans;
	
	for (i = nodes.begin(); i != nodes.end(); ++i) {
		for (k1 = 0; k1 < 3; ++k1) {
			for (k2 = 0; k2 < 3; ++k2) {
				trans[k2] = vals[l++];
				if (l >= vals.size()) {
					return false;
				}
			}
			i->second.node->set_trans(types[k1], trans);
		}
		for (j = i->second.props.begin(); j != i->second.props.end(); ++j) {
			j->second = vals[l++];
			if (l >= vals.size()) {
				return false;
			}
		}
	}
	return true;
}

bool scene::remove_property(const std::string &obj, const std::string &prop) {
	node_map::iterator i;
	property_map::iterator j;
	
	if ((i = nodes.find(obj)) == nodes.end()) {
		return false;
	}
	if ((j = i->second.props.find(prop)) == i->second.props.end()) {
		return false;
	}
	i->second.props.erase(j);
	return true;
}

int scene::num_nodes() const {
	return nodes.size();
}

float scene::get_dt() const {
	return dt;
}

int scene::get_dof() const {
	int dof = 0;
	node_map::const_iterator i;
	for (i = nodes.begin(); i != nodes.end(); ++i) {
		dof += NUM_NATIVE_PROPS + i->second.props.size();
	}
	return dof;
}

void scene::ipc_connect(ipcsocket *sock) {
	node_map::iterator i;
	disp_new_scene();
	for (i = nodes.begin(); i != nodes.end(); ++i) {
		disp_update_node(i->second.node);
	}
}

void scene::ipc_disconnect(ipcsocket *sock) {}
