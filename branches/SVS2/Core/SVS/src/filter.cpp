#include <iostream>
#include <string>
#include <list>
#include <iterator>
#include <sstream>
#include "linalg.h"
#include "sg_node.h"
#include "filter.h"

using namespace std;

typedef struct filter_cons_entry_struct {
	string name;
	filter* (*cons_func)(filter_params&);
} filter_cons_entry;

extern filter* make_bbox_filter(filter_params&);
extern filter* make_bbox_int_filter(filter_params&);

static filter_cons_entry filter_cons_tab[] = {
	{ "bbox",     make_bbox_filter },
	{ "bbox_int", make_bbox_int_filter }
};

filter::~filter() {
	std::list<filter*>::iterator i;
	for (i = childs.begin(); i != childs.end(); ++i) {
		delete *i;
	}
}

void filter::add_child(filter *c) {
	childs.push_back(c);
}

void filter::listen(filter_listener *l) {
	listeners.push_back(l);
}

void filter::unlisten(filter_listener *l) {
	listeners.remove(l);
}

void filter::notify() {
	std::list<filter_listener*>::iterator i;
	for (i = listeners.begin(); i != listeners.end(); ++i) {
		(**i).update(this);
	}
}

bool bool_filter::get_result_string(std::string &r) {
	bool b;
	if (!get_result(b)) {
		return false;
	}
	r = b ? "t" : "f";
	return true;
}

bool ptlist_filter::get_result_string(string &r) {
	ptlist pts;
	stringstream ss;
	
	if (!get_result(pts)) {
		return false;
	}
	copy(pts.begin(), pts.end(), ostream_iterator<vec3>(ss, ", "));
	r = ss.str();
	return true;
}

bool bbox_filter::get_result_string(string &r) {
	bbox b;
	stringstream ss;
	if (!get_result(b)) {
		return false;
	}
	ss << b;
	r = ss.str();
	return true;
}

sg_node_filter::sg_node_filter(sg_node *node)
: n(node)
{
	n->listen(this);
}

sg_node_filter::~sg_node_filter() {
	n->unlisten(this);
}

bool sg_node_filter::get_result(ptlist &r) {
	if (n) {
		n->get_world_points(r);
		return true;
	}
	return false;
}

string sg_node_filter::get_error() { 
	if (!n) {
		return "invalid node";
	}
	return "unknown error";
}

void sg_node_filter::update(sg_node *n, sg_node::change_type t) {
	if (t == sg_node::DEL || t == sg_node::DETACH) {
		n = NULL;
	}
	notify();
}

filter* make_filter(string name, filter_params &params) {
	int tabsize = sizeof(filter_cons_tab) / sizeof(filter_cons_entry);
	for(int i = 0; i < tabsize; ++i) {
		if (name == filter_cons_tab[i].name) {
			cout << "Making FILTER " << name << endl;
			return filter_cons_tab[i].cons_func(params);
		}
	}
	
	return NULL;
}
