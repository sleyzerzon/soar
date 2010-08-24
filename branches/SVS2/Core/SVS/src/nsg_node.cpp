#include <list>
#include <vector>
#include <algorithm>
#include "nsg_node.h"

using namespace std;

typedef vector<nsg_node*>::iterator childiter;

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

int nsg_node::get_nchilds() {
	return childs.size();
}

sg_node* nsg_node::get_child(int i) {
	if (!isgroup) {
		return NULL;
	}
	if (0 <= i && i < childs.size()) {
		return childs.at(i);
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
	childs.push_back(t);
	t->parent = this;
	t->set_transform_dirty();
	set_points_dirty();
	
	send_update(sg_node::ADDCHILD);
	
	return true;
}

void nsg_node::detach() {
	if (parent) {
		parent->detach_child(this);
		parent = NULL;
	}
	send_update(sg_node::DETACH);
}

void nsg_node::nsg_node::set_pos(vec3 xyz) {
	pos = xyz;
	set_transform_dirty();
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

void nsg_node::get_local_points(std::list<vec3> &result) {
	update_points();
	result.clear();
	copy(pts.begin(), pts.end(), back_inserter(result));
}

void nsg_node::get_world_points(std::list<vec3> &result) {
	update_points();
	update_transform();
	result.clear();
	transform(pts.begin(), pts.end(), back_inserter(result), wtransform);
}

void nsg_node::detach_child(nsg_node *c) {
	childiter i;
	for (i = childs.begin(); i != childs.end(); ++i) {
		if (*i == c) {
			childs.erase(i);
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
	for (childiter i = childs.begin(); i != childs.end(); ++i) {
		(**i).set_transform_dirty();
	}
	send_update(sg_node::PTSCHANGE);
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
	send_update(sg_node::PTSCHANGE);
}

void nsg_node::update_points() {
	back_insert_iterator<std::list<vec3> > pbi(pts);
	
	if (!isgroup || !pdirty) {
		return;
	}
	
	pts.clear();
	for (childiter i = childs.begin(); i != childs.end(); ++i) {
		(**i).update_points();
		(**i).update_transform();
		transform((**i).pts.begin(), (**i).pts.end(), pbi, (**i).ltransform);
	}
	
	pdirty = false;
}

/* if updates result in observers removing themselves, the iteration may
 * screw up, so make a copy of the std::list first */
void nsg_node::send_update(sg_node::change_type t) {
	std::list<sg_observer*>::iterator i;
	std::list<sg_observer*> c;
	copy(observers.begin(), observers.end(), back_inserter(c));
	for (i = c.begin(); i != c.end(); ++i) {
		(**i).update(this, t);
	}
}

void nsg_node::observe(sg_observer *o) {
	observers.push_back(o);
}

void nsg_node::unobserve(sg_observer *o) {
	observers.remove(o);
}