#include <list>
#include <vector>
#include <algorithm>
#include "cgalsupport.h"
#include "nsg_node.h"

using namespace std;

typedef vector<nsg_node*>::iterator childiter;
typedef list<Point3>::iterator ptsiter;

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
	return true;
}

bool nsg_node::detach_child(sg_node *c) {
	nsg_node *t = dynamic_cast<nsg_node*>(c);
	if (!t) { return false; }
	
	for (childiter i = childs.begin(); i != childs.end(); ++i) {
		if (t == *i) {
			childs.erase(i);
			t->parent = NULL;
			t->set_transform_dirty();
			set_points_dirty();
			return true;
		}
	}
	return false;
}

void nsg_node::nsg_node::set_pos(Vector3 xyz) {
	pos = xyz;
	set_transform_dirty();
}

Vector3 nsg_node::get_pos() {
	return pos;
}

void nsg_node::set_rot(Vector3 ypr) {
	rot = ypr;
	set_transform_dirty();
}

Vector3 nsg_node::get_rot() {
	return rot;
}

void nsg_node::set_scale(Vector3 xyz) {
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

void nsg_node::get_local_points(list<Point3> &result) {
	cout << pts.size() << endl;
	update_points();
	cout << pts.size() << endl;
	copy(pts.begin(), pts.end(), back_inserter(result));
}

void nsg_node::get_world_points(list<Point3> &result) {
	update_points();
	update_transform();
	transform(pts.begin(), pts.end(), back_inserter(result), wtransform);
}

void nsg_node::set_transform_dirty() {
	tdirty = true;
	if (parent) {
		parent->set_points_dirty();
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
	cout << "transform:" << endl;
	print_transform(wtransform, cout);
	cout << "transform done" << endl;
}

void nsg_node::set_points_dirty() {
	if (!isgroup) {  // may change if we allow modifying primitive geometries
		return;
	}
	pdirty = true;
	if (parent) {
		parent->set_points_dirty();
	}
}

void nsg_node::update_points() {
	back_insert_iterator<list<Point3> > pbi(pts);
	
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
