#include <list>
#include <vector>
#include "cgalsupport.h"
#include "nsg_node.h"

using std::vector;
using std::list;

typedef vector<nsg_node*>::iterator childiter;

nsg_node::nsg_node(std::string nm) 
	: name(nm),
	  parent(NULL),
	  tdirty(false),
	  chdirty(false),
	  pos(CGAL::NULL_VECTOR),
	  rot(CGAL::NULL_VECTOR),
	  scale(CGAL::NULL_VECTOR)
{
}

std::string nsg_node::get_name() {
	return name;
}

void nsg_node::set_name(std::string nm) {
	name = nm;
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

bool nsg_node::add_child(sg_node *c) {
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
	set_convex_hull_dirty();
	return true;
}

bool nsg_node::del_child(sg_node *c) {
	nsg_node *t = dynamic_cast<nsg_node*>(c);
	if (!t) { return false; }
	
	for (childiter i = childs.begin(); i != childs.end(); ++i) {
		if (t == *i) {
			childs.erase(i);
			t->parent = NULL;
			t->set_transform_dirty();
			set_convex_hull_dirty();
			return true;
		}
	}
	return false;
}

void nsg_node::nsg_node::set_pos(Vector3 &xyz) {
	pos = xyz;
	lpos = Transform3(CGAL::TRANSLATION, pos);
	set_transform_dirty();
}

Vector3 nsg_node::get_pos() {
	return pos;
}

void nsg_node::set_rot(Vector3 &ypr) {
	rot = ypr;
	lrot = euler_ypr_transform(rot);
	set_transform_dirty();
}

Vector3 nsg_node::get_rot() {
	return rot;
}

void nsg_node::set_scale(Vector3 &xyz) {
	scale = xyz;
	lscale = scaling_transform(scale);
	set_transform_dirty();
}

Vector3 nsg_node::get_scale() {
	return scale;
}

void nsg_node::clear_transforms() {
	wtransform = Transform3();
	lpos = wtransform;
	lrot = wtransform;
	lscale = wtransform;
	
	pos   = CGAL::NULL_VECTOR;
	rot   = CGAL::NULL_VECTOR;
	scale = CGAL::NULL_VECTOR;
	
	set_transform_dirty();
}

Transform3 nsg_node::get_world_transform() {
	if (tdirty) {
		wtransform = lpos * lrot * lscale;
		if (parent) {
			wtransform = parent->get_world_transform() * wtransform;
		}
	}
	return wtransform;
}

ConvexPoly3* nsg_node::get_convex_hull() {
	list<ConvexPoly3*> cpolys;
	
	if (!isgroup) {
		return convex_hull;
	}
	
	if (chdirty) {
		for (childiter ci = childs.begin(); ci != childs.end(); ++ci) {
			cpolys.push_back((**ci).get_convex_hull());
		}
		delete convex_hull;
		convex_hull = hull_of_hulls(cpolys.begin(), cpolys.end());
	}
}

void nsg_node::set_transform_dirty() {
	tdirty = true;
	for (childiter i = childs.begin(); i != childs.end(); ++i) {
		(**i).set_transform_dirty();
	}
}

void nsg_node::set_convex_hull_dirty() {
	chdirty = true;
	if (parent) {
		parent->set_convex_hull_dirty();
	}
}
