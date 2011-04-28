#include <stdlib.h>
#include <map>
#include <iterator>
#include <iostream>
#include <sstream>
#include "scene.h"
#include "nsg_node.h"
#include "linalg.h"
#include "ipcsocket.h"
#include "common.h"

using namespace std;

ipcsocket *disp = NULL;

scene::scene(string name, string rootname) 
: name(name), rootname(rootname), iscopy(false)
{
	if (!disp) {
		disp = new ipcsocket(getnamespace() + "disp", false);
	}
	add_node("", rootname);
	root = nodes[rootname];
	disp_new_scene(name);
}

scene::scene(scene &other)
: name(other.name), rootname(other.rootname), iscopy(true), properties(other.properties)
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

bool parse_n_floats(vector<string> &f, int &start, int n, double *x) {
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

int scene::parse_property(vector<string> &f) {
	if (f.size() != 2) {
		return f.size();
	}
	stringstream ss(f[1]);
	double val;
	
	if (!(ss >> val)) {
		return 1;
	}
	properties[f[0]] = val;
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
			case 'p':
				errfield = parse_property(fields);
				break;
			default:
				cerr << "expecting a|d|c at beginning of line '" << *i << "'" << endl;
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

double scene::get_property(const string &prop) const {
	property_map::const_iterator i;
	
	if ((i = properties.find(prop)) == properties.end()) {
		assert(false);
	}
	return i->second;
}

void scene::get_all_properties(vector<pair<string, double> > &props) const {
	property_map::const_iterator i;
	for (i = properties.begin(); i != properties.end(); ++i) {
		props.push_back(*i);
	}
}

void scene::set_property(const string &prop, double v) {
	property_map::iterator i;
	if ((i = properties.find(prop)) == properties.end()) {
		assert(false);
	}
	i->second = v;
}

flat_scene::flat_scene() {}

flat_scene::flat_scene(const flat_scene &s) 
: vals(s.vals), node_info(s.node_info), prop_info(s.prop_info)
{}

flat_scene::flat_scene(scene *scn) {
	vector<sg_node*> nodes;
	vector<sg_node*>::iterator i;
	vector<pair<string, double> > props;
	vector<pair<string, double> >::iterator j;
	const char *types = "prs";
	vec3 trans;
	
	scn->get_all_nodes(nodes);
	for (i = nodes.begin(); i != nodes.end(); ++i) {
		sg_node *n = *i;
		string parent = "";
		if (n->get_parent()) {
			parent = n->get_parent()->get_name();
		}
		node_info[n->get_name()] = make_pair(parent, vals.size());
		for (int k = 0; k < 3; ++k) {
			trans = n->get_trans(types[k]);
			for (int l = 0; l < 3; ++l) {
				vals.push_back(trans[l]);
			}
		}
	}
	
	scn->get_all_properties(props);
	for (j = props.begin(); j != props.end(); ++j) {
		prop_info[j->first] = vals.size();
		vals.push_back(j->second);
	}
}

int flat_scene::get_trans_offset(const string &name, char type) const {
	map<string, pair<string, int> >::const_iterator i;
	if ((i = node_info.find(name)) == node_info.end()) {
		return -1;
	}
	switch (type) {
		case 'p':
			return i->second.second;
		case 'r':
			return i->second.second + 3;
			break;
		case 's':
			return i->second.second + 6;
			break;
	}
	return -1;
}

bool flat_scene::get_node_trans(const string &name, char type, vec3 &t) const {
	int offset = get_trans_offset(name, type);
	if (offset < 0) return false;
	
	t.x = vals[offset];
	t.y = vals[offset+1];
	t.z = vals[offset+2];
	return true;
}

bool flat_scene::set_node_trans(const string &name, char type, const vec3 &t) {
	int offset = get_trans_offset(name, type);
	if (offset < 0) return false;
	vals[offset] = t.x;
	vals[offset+1] = t.y;
	vals[offset+2] = t.z;
	return true;
}

bool flat_scene::get_property(const string &prop, double &val) const {
	map<string, int>::const_iterator i = prop_info.find(prop);
	if (i == prop_info.end()) {
		return false;
	}
	val = vals[i->second];
	return true;
}

bool flat_scene::set_property(const string &prop, double val) {
	map<string, int>::const_iterator i = prop_info.find(prop);
	if (i == prop_info.end()) {
		return false;
	}
	vals[i->second] = val;
	return true;
}

void flat_scene::update_scene(scene *scn) const {
	map<string, pair<string, int> >::const_iterator i;
	map<string, int>::const_iterator j;
	
	for (i = node_info.begin(); i != node_info.end(); ++i) {
		string nodename = i->first;
		int offset = i->second.second;
		vec3 pos(vals[offset], vals[offset+1], vals[offset+2]);
		vec3 rot(vals[offset+3], vals[offset+4], vals[offset+5]);
		vec3 scale(vals[offset+6], vals[offset+7], vals[offset+8]);
		scn->set_node_trans(nodename, 'p', pos);
		scn->set_node_trans(nodename, 'r', rot);
		scn->set_node_trans(nodename, 's', scale);
	}
	for (j = prop_info.begin(); j != prop_info.end(); ++j) {
		scn->set_property(j->first, vals[j->second]);
	}
}

void flat_scene::get_signature(scene_sig &sig) const {
	map<string, pair<string, int> >::const_iterator i;
	map<string, int>::const_iterator j;
	string property = "PROPERTY";
	
	for(i = node_info.begin(); i != node_info.end(); ++i) {
		sig.insert(make_pair(i->first, i->second.first));
	}
	for(j = prop_info.begin(); j != prop_info.end(); ++j) {
		sig.insert(make_pair(j->first, property));
	}
}
