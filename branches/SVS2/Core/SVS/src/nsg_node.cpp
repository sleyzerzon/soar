#include <list>
#include <vector>
#include <algorithm>
#include "nsg_node.h"

using namespace std;

typedef vector<nsg_node*>::iterator childiter;

nsg_node::nsg_node(std::string name) 
: name(name), parent(NULL), tdirty(false), pdirty(false), isgroup(true),
  pos(0.0, 0.0, 0.0), rot(0.0, 0.0, 0.0), scale(1.0, 1.0, 1.0)
{}

nsg_node::nsg_node(std::string name, const ptlist &points)
: name(name), parent(NULL), tdirty(false), pdirty(false), isgroup(false), points(points),
  pos(0.0, 0.0, 0.0), rot(0.0, 0.0, 0.0), scale(1.0, 1.0, 1.0)
{ }

nsg_node::~nsg_node() {
	childiter i;
	
	if (parent) {
		parent->detach_child(this);
	}
	for (i = children.begin(); i != children.end(); ++i) {
		(**i).parent = NULL;  // so it doesn't try to detach itself
		delete *i;
	}
	send_update(sg_node::DELETED);
}

sg_node* nsg_node::copy() {
	nsg_node *c;
	childiter i;
	
	if (isgroup) {
		c = new nsg_node(name);
	} else {
		c = new nsg_node(name, points);
	}
	c->set_pos(pos);
	c->set_rot(rot);
	c->set_scale(scale);
	
	for(i = children.begin(); i != children.end(); ++i) {
		c->attach_child((**i).copy());
	}
	return c;
}

std::string nsg_node::get_name() {
	return name;
}

void nsg_node::set_name(string nm) {
	name = nm;
}

bool nsg_node::is_group() {
	return isgroup;
}

sg_node* nsg_node::get_parent() {
	return parent;
}

int nsg_node::num_children() {
	return children.size();
}

sg_node* nsg_node::get_child(int i) {
	if (!isgroup) {
		return NULL;
	}
	if (0 <= i && i < children.size()) {
		return children.at(i);
	}
	return NULL;
}

bool nsg_node::attach_child(sg_node *c) {
	nsg_node* t;
	if (!isgroup) {
		return false;
	}
	t = dynamic_cast<nsg_node*>(c);
	if (!t) {
		return false;
	}
	children.push_back(t);
	t->parent = this;
	t->set_transform_dirty();
	set_points_dirty();
	send_update(sg_node::CHILD_ADDED);
	
	return true;
}

void nsg_node::set_trans(char type, vec3 trans) {
	switch (type) {
		case 'p':
			pos = trans;
			break;
		case 'r':
			rot = trans;
			break;
		case 's':
			scale = trans;
			break;
		default:
			assert(false);
	}
	set_transform_dirty();
}

vec3 nsg_node::get_trans(char type) {
	switch (type) {
		case 'p':
			return pos;
		case 'r':
			return rot;
		case 's':
			return scale;
		default:
			assert (false);
	}
}

vec3 nsg_node::get_pos() {
	return pos;
}

void nsg_node::set_rot(vec3 ypr) {
	rot = ypr;
	set_transform_dirty();
}

vec3 nsg_node::get_rot() {
	return rot;
}

void nsg_node::set_scale(vec3 xyz) {
	scale = xyz;
	set_transform_dirty();
}

vec3 nsg_node::get_scale() {
	return scale;
}

void nsg_node::get_local_points(ptlist &result) {
	update_points();
	result = points;
}

void nsg_node::get_world_points(ptlist &result) {
	update_points();
	update_transform();
	result.clear();
	result.reserve(points.size());
	transform(points.begin(), points.end(), back_inserter(result), wtransform);
}

void nsg_node::detach_child(nsg_node *c) {
	childiter i;
	for (i = children.begin(); i != children.end(); ++i) {
		if (*i == c) {
			children.erase(i);
			set_points_dirty();
			return;
		}
	}
}

void nsg_node::set_transform_dirty() {
	tdirty = true;
	if (parent) {
		parent->set_points_dirty();
	}
	for (childiter i = children.begin(); i != children.end(); ++i) {
		(**i).set_transform_dirty();
	}
	send_update(sg_node::POINTS_CHANGED);
}

void nsg_node::update_transform() {
	if (!tdirty) {
		return;
	}
	
	ltransform = transform3(transform_tags::TRANSLATION, pos) * 
	             transform3(transform_tags::ROTATION, rot) * 
	             transform3(transform_tags::SCALING, scale);
	if (parent) {
		parent->update_transform();
		wtransform = parent->wtransform * ltransform;
	} else {
		wtransform = ltransform;
	}
}

void nsg_node::set_points_dirty() {
	if (!isgroup) {  // may change if we allow modifying primitive geometries
		return;
	}
	pdirty = true;
	if (parent) {
		parent->set_points_dirty();
	}
	send_update(sg_node::POINTS_CHANGED);
}

void nsg_node::update_points() {
	back_insert_iterator<ptlist> pbi(points);
	
	if (!isgroup || !pdirty) {
		return;
	}
	
	points.clear();
	for (childiter i = children.begin(); i != children.end(); ++i) {
		(**i).update_points();
		(**i).update_transform();
		transform((**i).points.begin(), (**i).points.end(), pbi, (**i).ltransform);
	}
	
	pdirty = false;
}

/* if updates result in observers removing themselves, the iteration may
 * screw up, so make a copy of the std::list first */
void nsg_node::send_update(sg_node::change_type t) {
	std::list<sg_listener*>::iterator i;
	std::list<sg_listener*> c;
	std::copy(listeners.begin(), listeners.end(), back_inserter(c));
	for (i = c.begin(); i != c.end(); ++i) {
		(**i).update(this, t);
	}
}

void nsg_node::listen(sg_listener *o) {
	listeners.insert(o);
}

void nsg_node::unlisten(sg_listener *o) {
	listeners.erase(o);
}
