#include <list>
#include <vector>
#include "cgalsupport.h"
#include "nsg_node.h"

using namespace std;

typedef vector<nsg_node*>::iterator childiter;

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
	set_transform_dirty();
}

Vector3 nsg_node::get_pos() {
	return pos;
}

void nsg_node::set_rot(Vector3 &ypr) {
	rot = ypr;
	set_transform_dirty();
}

Vector3 nsg_node::get_rot() {
	return rot;
}

void nsg_node::set_scale(Vector3 &xyz) {
	scale = xyz;
	set_transform_dirty();
}

Vector3 nsg_node::get_scale() {
	return scale;
}

void nsg_node::clear_transforms() {
	pos   = CGAL::NULL_VECTOR;
	rot   = CGAL::NULL_VECTOR;
	scale = CGAL::NULL_VECTOR;
	
	set_transform_dirty();
}

ConvexPoly3 nsg_node::get_convex_hull() {
	update_convex_hull();
	return whull;
}

void nsg_node::set_transform_dirty() {
	tdirty = true;
	if (parent) {
		parent->set_convex_hull_dirty();
	}
	for (childiter i = childs.begin(); i != childs.end(); ++i) {
		(**i).set_transform_dirty();
	}
}

void nsg_node::update_transform() {
	if (!tdirty) {
		return;
	}
	ltransform = Transform3(CGAL::TRANSLATION, pos) * euler_ypr_transform(rot) * scaling_transform(scale);
	if (parent) {
		parent->update_transform();
		wtransform = parent->wtransform * ltransform;
	} else {
		wtransform = ltransform;
	}
}

void nsg_node::set_convex_hull_dirty() {
	if (!isgroup) {  // may change if we allow modifying primitive geometries
		return;
	}
	chdirty = true;
	if (parent) {
		parent->set_convex_hull_dirty();
	}
}

void nsg_node::update_convex_hull() {
	list<Point3> pts;
	back_insert_iterator<list<Point3> > pbi(pts);
	ConvexPoly3::Point_iterator cpi;
	
	if (!isgroup || !chdirty) {
		return;
	}
	
	update_transform();
	
	for (childiter i = childs.begin(); i != childs.end(); ++i) {
		(**i).update_convex_hull();
		transform_set(ltransform, (**i).lhull.points_begin(), (**i).lhull.points_end(), pbi);
	}
	convex_hull_3(pts.begin(), pts.end(), lhull);
	
	whull = lhull;
	for(cpi = whull.points_begin(); cpi != whull.points_end(); ++cpi) {
		*cpi = cpi->transform(wtransform);
	}
	
	chdirty = false;
}
