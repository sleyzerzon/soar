#include <stdlib.h>
#include <map>
#include <iterator>
#include <iostream>
#include <sstream>
#include <limits>
#include "scene.h"
#include "nsg_node.h"
#include "linalg.h"
#include "ipcsocket.h"
#include "common.h"

using namespace std;

ipcsocket *disp = NULL;

scene::scene(string name, string rootname) 
: name(name), rootname(rootname), iscopy(false), dt(1.0)
{
	if (!disp) {
		disp = new ipcsocket(getnamespace() + "disp", false);
	}
	add_node("", rootname);
	root = nodes[rootname];
	disp_new_scene(name);
}

scene::scene(scene &other)
: name(other.name), rootname(other.rootname), iscopy(true), properties(other.properties), dt(other.dt)
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
	if (!iscopy) {
		disp_del_scene(name);
	}
	delete root;
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

void scene::get_all_nodes(vector<sg_node*> &n) {
	node_map::const_iterator i;
	for (i = nodes.begin(); i != nodes.end(); ++i) {
		n.push_back(i->second);
	}
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
		disp_del_node(i->second);
	}
	delete i->second;
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
	if (f.size() != 3) {
		return f.size();
	}
	stringstream ss(f[2]);
	float val;
	
	if (!(ss >> val)) {
		return 1;
	}
	properties[make_pair(f[0], f[1])] = val;
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

void scene::disp_new_scene(string name) {
	if (disp) disp->send("newscene\n" + name);
}

void scene::disp_del_scene(string name) {
	if (disp) disp->send("delscene\n" + name);
}

float scene::get_property(const string &obj, const string &prop) const {
	property_map::const_iterator i;
	if ((i = properties.find(make_pair(obj, prop))) == properties.end()) {
		assert(false);
	}
	return i->second;
}

void scene::get_node_properties(const string &obj, map<string, float> &props) const {
	property_map::const_iterator i;
	for (i = properties.begin(); i != properties.end(); ++i) {
		if (i->first.first == obj) {
			props[i->first.second] = i->second;
		}
	}
}

void scene::set_property(const string &obj, const string &prop, float v) {
	pair<string, string> key = make_pair(obj, prop);
	property_map::iterator i;
	if ((i = properties.find(key)) == properties.end()) {
		properties[key] = v;
	} else {
		i->second = v;
 	}
}

int scene::num_nodes() const {
	return nodes.size();
}

float scene::get_dt() const {
	return dt;
}

int scene::get_dof() const {
	return nodes.size() * 9 + properties.size(); 
}

flat_scene::flat_scene() {}

flat_scene::flat_scene(const flat_scene &s) 
: vals(s.vals), nodes(s.nodes), properties(s.properties)
{}

flat_scene::flat_scene(scene *scn) : vals(scn->get_dof()) {
	vector<sg_node*> ns;
	vector<sg_node*>::iterator n;
	const char *types = "prs";
	vec3 trans;
	int i, j, k;
	
	scn->get_all_nodes(ns);
	
	i = 0;
	for (n = ns.begin(); n != ns.end(); ++n) {
		string name = (**n).get_name();
		node_info &info = nodes[name];
		info.begin = i;
		sg_node *parent = (**n).get_parent();
		if (parent) {
			info.parent = parent->get_name();
		} else {
			assert(name == "world");
			info.parent = "";
		}
		for (j = 0; j < 3; ++j) {
			trans = (**n).get_trans(types[j]);
			for (k = 0; k < 3; ++k) {
				vals[i++] = trans[k];
			}
		}
		
		map<string, float> props;
		map<string, float>::iterator pi;
		scn->get_node_properties(name, props);
		for (pi = props.begin(); pi != props.end(); ++pi) {
			properties[make_pair(name, pi->first)] = i;
			vals[i++] = pi->second;
		}
		info.length = i - info.begin;
	}
}

int flat_scene::get_trans_offset(const string &name, char type) const {
	map<string, node_info>::const_iterator i;
	if ((i = nodes.find(name)) == nodes.end()) {
		return -1;
	}
	switch (type) {
		case 'p':
			return i->second.begin;
		case 'r':
			return i->second.begin + 3;
			break;
		case 's':
			return i->second.begin + 6;
			break;
	}
	return -1;
}

bool flat_scene::get_node_trans(const string &name, char type, vec3 &t) const {
	int offset = get_trans_offset(name, type);
	if (offset < 0) return false;
	
	for (int i = 0; i < 3; ++i) {
		t[i] = vals[offset + i];
	}
	return true;
}

bool flat_scene::get_node_trans(const string &name, vec3 &p, vec3 &r, vec3 &s) const {
	map<string, node_info>::const_iterator i = nodes.find(name);
	if (i == nodes.end()) {
		return false;
	}
	int j, offset = i->second.begin;
	
	for (j = 0; j < 3; ++j) {
		p[j] = vals[offset + j];
	}
	for (j = 0; j < 3; ++j) {
		r[j] = vals[offset + 3 + j];
	}
	for (j = 0; j < 3; ++j) {
		s[j] = vals[offset + 6 + j];
	}
	
	return true;
}

bool flat_scene::set_node_trans(const string &name, char type, const vec3 &t) {
	int offset = get_trans_offset(name, type);
	if (offset < 0) return false;
	
	for (int i = 0; i < 3; ++i) {
		vals[offset + i] = t[i];
	}
	return true;
}

bool flat_scene::get_property(const string &name, const string &prop, float &val) const {
	prop_table::const_iterator i = properties.find(make_pair(name, prop));
	if (i == properties.end()) {
		return false;
	}
	val = vals[i->second];
	return true;
}

bool flat_scene::set_property(const string &name, const string &prop, float val) {
	prop_table::const_iterator i = properties.find(make_pair(name, prop));
	if (i == properties.end()) {
		return false;
	}
	vals[i->second] = val;
	return true;
}

void flat_scene::update_scene(scene *scn) const {
	node_table::const_iterator i;
	prop_table::const_iterator j;
	
	for (i = nodes.begin(); i != nodes.end(); ++i) {
		string nodename = i->first;
		int offset = i->second.begin;
		vec3 pos(vals[offset], vals[offset+1], vals[offset+2]);
		vec3 rot(vals[offset+3], vals[offset+4], vals[offset+5]);
		vec3 scale(vals[offset+6], vals[offset+7], vals[offset+8]);
		scn->set_node_trans(nodename, 'p', pos);
		scn->set_node_trans(nodename, 'r', rot);
		scn->set_node_trans(nodename, 's', scale);
	}
	for (j = properties.begin(); j != properties.end(); ++j) {
		scn->set_property(j->first.first, j->first.second, vals[j->second]);
	}
}

void flat_scene::get_column_names(vector<string> &names) const {
	node_table::const_iterator i;
	prop_table::const_iterator j;
	names.resize(vals.size());
	
	for(i = nodes.begin(); i != nodes.end(); ++i) {
		for(int k = 0; k < 9; ++k) {
			stringstream name;
			if (k < 3) {
				name << i->first << ":pos:" << k;
			} else if (k < 6) {
				name << i->first << ":rot:" << k - 3;
			} else {
				name << i->first << ":scl:" << k - 6;
			}
			names[i->second.begin + k] = name.str();
		}
	}
	
	for(j = properties.begin(); j != properties.end(); ++j) {
		names[j->second] = j->first.second;
	}
}

int flat_scene::get_dof() const {
	return vals.size();
}

bool flat_scene::congruent(const flat_scene &s) const {
	return nodes == s.nodes && properties == s.properties;
}

float flat_scene::distance(const flat_scene &s) const {
	if (vals.size() != s.vals.size()) {
		return numeric_limits<float>::infinity();
	}
	return vals.dist(s.vals);
}

void flat_scene::get_nodes(vector<string> &ns) const {
	node_table::const_iterator i;
	for (i = nodes.begin(); i != nodes.end(); ++i) {
		ns.push_back(i->first);
	}
}

floatvec flat_scene::get_node_vals(const string &name) const {
	node_table::const_iterator i;
	if ((i = nodes.find(name)) == nodes.end()) {
		return floatvec();
	}
	return vals.slice(i->second.begin, i->second.length);
}
