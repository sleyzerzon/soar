#include <iostream>
#include "drawer.h"

using namespace std;

int drawer::POS = 1;
int drawer::ROT = 1 << 2;
int drawer::SCALE = 1 << 3;
int drawer::COLOR = 1 << 4;
int drawer::VERTS = 1 << 5;

ofstream drawer::fifo("/tmp/dispfifo");

drawer::drawer(const string &sname) 
: scene_name(sname), scl(1., 1., 1.), color(0., 1., 0.)
{}

drawer::~drawer() {
	// fifo.close();
}

void drawer::set_pos(const vec3 &p) {
	pos = p;
}

void drawer::set_rot(const vec3 &r) {
	rot = r;
}

void drawer::set_scale(const vec3 &s) {
	scl = s;
}

void drawer::set_transforms(sgnode *n) {
	set_pos(n->get_trans('p'));
	set_rot(n->get_trans('r'));
	set_scale(n->get_trans('s'));
}

void drawer::set_color(float r, float g, float b) {
	color[0] = r;
	color[1] = g;
	color[2] = b;
}

void drawer::set_vertices(const ptlist &v) {
	verts = v;
}

void drawer::set_vertices(sgnode *n) {
	ptlist pts;
	n->get_local_points(pts);
	verts = pts;
}

void drawer::reset_properties() {
	pos.zero();
	rot.zero();
	scl = vec3(1., 1., 1.);
	color = vec3(0., 1., 0.);
}

void drawer::add(const string &name) {
	fifo << scene_name << " n " << name << " p " << pos << " r " << rot << " s " << scl << " c " << color << " v ";
	copy(verts.begin(), verts.end(), ostream_iterator<vec3>(fifo, " "));
	fifo << endl;
	fifo << scene_name << " t " << name << "_label " << pos << " " << name << endl;
	fifo.flush();
}

void drawer::add(sgnode *n) {
	ptlist pts;
	n->get_local_points(pts);
	set_pos(n->get_trans('p'));
	set_rot(n->get_trans('r'));
	set_scale(n->get_trans('s'));
	set_vertices(pts);
	add(n->get_name());
}

void drawer::del(const string &name) {
	fifo << scene_name << " d " << name << endl;
	fifo << scene_name << " d " << name << "_label" << endl;
	fifo.flush();
}

void drawer::del(sgnode *n) {
	del(n->get_name());
}

void drawer::change(const string &name, int props) {
	fifo << scene_name << " n " << name;
	if (props & POS) {
		fifo << " p " << pos;
	}
	if (props & ROT) {
		fifo << " r " << rot;
	}
	if (props & SCALE) {
		fifo << " s " << scl;
	}
	if (props & COLOR) {
		fifo << " c " << color;
	}
	if (props & VERTS) {
		fifo << " ";
		copy(verts.begin(), verts.end(), ostream_iterator<vec3>(fifo, " "));
	}
	fifo << endl;
	if (props & POS) {
		fifo << scene_name << " t " << name << "_label " << pos << " " << name << endl;
	}
	fifo.flush();
}

