#include <assert.h>
#include <list>
#include <vector>
#include <algorithm>
#include "sgnode.h"

using namespace std;

typedef vector<sgnode*>::iterator childiter;
typedef vector<sgnode*>::const_iterator const_childiter;

sgnode::sgnode(std::string name) 
: name(name), parent(NULL), tdirty(false), pdirty(false), isgroup(true),
  pos(0.0, 0.0, 0.0), rot(0.0, 0.0, 0.0), scale(1.0, 1.0, 1.0)
{}

sgnode::sgnode(std::string name, const ptlist &points)
: name(name), parent(NULL), tdirty(false), pdirty(false), isgroup(false), points(points),
  pos(0.0, 0.0, 0.0), rot(0.0, 0.0, 0.0), scale(1.0, 1.0, 1.0)
{ }

sgnode::~sgnode() {
	childiter i;
	
	if (parent) {
		parent->detach_child(this);
	}
	for (i = children.begin(); i != children.end(); ++i) {
		(**i).parent = NULL;  // so it doesn't try to detach itself
		delete *i;
	}
	send_update(sgnode::DELETED);
}

sgnode* sgnode::copy() const {
	sgnode *c;
	const_childiter i;
	
	if (isgroup) {
		c = new sgnode(name);
	} else {
		c = new sgnode(name, points);
	}
	c->set_trans('p', pos);
	c->set_trans('r', rot);
	c->set_trans('s', scale);
	
	for(i = children.begin(); i != children.end(); ++i) {
		c->attach_child((**i).copy());
	}
	return c;
}

std::string sgnode::get_name() const {
	return name;
}

void sgnode::set_name(string nm) {
	name = nm;
}

bool sgnode::is_group() const {
	return isgroup;
}

sgnode* sgnode::get_parent() {
	return parent;
}

int sgnode::num_children() const {
	return children.size();
}

sgnode* sgnode::get_child(int i) {
	if (!isgroup) {
		return NULL;
	}
	if (0 <= i && i < children.size()) {
		return children.at(i);
	}
	return NULL;
}

void sgnode::walk(std::list<sgnode*> &result) {
	childiter i;
	result.push_back(this);
	for(i = children.begin(); i != children.end(); ++i) {
		(**i).walk(result);
	}
}

bool sgnode::attach_child(sgnode *c) {
	sgnode* t;
	if (!isgroup) {
		return false;
	}
	t = dynamic_cast<sgnode*>(c);
	if (!t) {
		return false;
	}
	children.push_back(t);
	t->parent = this;
	t->set_transform_dirty();
	set_points_dirty();
	send_update(sgnode::CHILD_ADDED, children.size() - 1);
	
	return true;
}

void sgnode::set_trans(char type, vec3 trans) {
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

vec3 sgnode::get_trans(char type) const {
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

void sgnode::get_local_points(ptlist &result) {
	update_points();
	result = points;
}

void sgnode::get_world_points(ptlist &result) {
	update_points();
	update_transform();
	result.clear();
	result.reserve(points.size());
	transform(points.begin(), points.end(), back_inserter(result), wtransform);
}

void sgnode::detach_child(sgnode *c) {
	childiter i;
	for (i = children.begin(); i != children.end(); ++i) {
		if (*i == c) {
			children.erase(i);
			set_points_dirty();
			return;
		}
	}
}

void sgnode::set_transform_dirty() {
	tdirty = true;
	if (parent) {
		parent->set_points_dirty();
	}
	for (childiter i = children.begin(); i != children.end(); ++i) {
		(**i).set_transform_dirty();
	}
	send_update(sgnode::POINTS_CHANGED);
}

void sgnode::update_transform() {
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

void sgnode::set_points_dirty() {
	if (!isgroup) {  // may change if we allow modifying primitive geometries
		return;
	}
	pdirty = true;
	if (parent) {
		parent->set_points_dirty();
	}
	send_update(sgnode::POINTS_CHANGED);
}

void sgnode::update_points() {
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
void sgnode::send_update(sgnode::change_type t, int added) {
	std::list<sgnode_listener*>::iterator i;
	std::list<sgnode_listener*> c;
	std::copy(listeners.begin(), listeners.end(), back_inserter(c));
	for (i = c.begin(); i != c.end(); ++i) {
		(**i).node_update(this, t, added);
	}
}

void sgnode::listen(sgnode_listener *o) {
	listeners.insert(o);
}

void sgnode::unlisten(sgnode_listener *o) {
	listeners.erase(o);
}
